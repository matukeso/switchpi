#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include "switch.h"

int current_tc();

// static struct model midi;

static void disp()
{
#ifdef SINGLE
  printf("<%d, %d, %d>\n", midi.pgm_a, midi.pst_b, midi.fader);
#endif
}


static char cmd_queue[64];
static int cmd_len = 0;
static long long tick_ping;
static long long send_ack;
static const long long timeout_ack = 1 * 1000000000LL;
static const char cmd_fmt[] = "VIDEO OUTPUT ROUTING:\n%d %d\n\n";

void send_232c_pgm_bmlan(int ch)
{
  cmd_len = sprintf(cmd_queue, cmd_fmt, 2, ch + 31);
}
void send_232c_pst_bmlan(int ch)
{
  cmd_len = sprintf(cmd_queue, cmd_fmt, 3, ch + 31);
}
void send_232c_ato_bmlan()
{
}

static const char PING[] = "PING\n\n";

void proc_line(const char *line)
{
  static int mode;
//  printf("L:%s\n", line);
  if (strcmp_findimm(line, "ACK"))
  {
    midi.tick = nanosec_now();
    send_ack = 0;
  }
  if (line[0] == 0)
  {
    if (mode == 5)
      disp();
    mode = 0;
    return;
  }
  if (strcmp_findimm(line, "VIDEO OUTPUT ROUTING:"))
  {
    mode = 5;
    midi.fader = 64;
    midi.tick = nanosec_now();
    return;
  }

  if (mode == 5)
  {
    int from;
    int video;
    if (sscanf(line, "%d %d", &from, &video) == 2)
    {
      if (from == 2)
        midi.pgm_a = video;
      if (from == 3)
        midi.pst_b = video;
    }
  }
}

static void eachline(char *p)
{
  for (int line = 0;; line++)
  {
    char *el = strchr(p, '\n');
    if (el == NULL)
      break;
    *el = '\0';
    el++;
    proc_line(p);
    p = el;
  }
}

static int read_command(int fd)
{
  if (!can_read_fd(fd, 500))
    return 0;
  char buf[8192];
  int len = recv(fd, buf, 8192, 0);
  if (len < 0)
  {
    if (errno == EWOULDBLOCK)
      return 0;
    return -10;
  }
  if (len == 0)
  {
    fprintf(stderr, "disocnn");
    return -2;
  }

  eachline(buf);
  return 1;
}

int init_bm_lan(const char *peer)
{
  int ret = 1;
  printf("init bm-lan %s\n", peer);
  static const struct addrinfo ai_help = {AI_NUMERICSERV | AI_NUMERICHOST, PF_UNSPEC, SOCK_STREAM};
  struct addrinfo *ai_r = 0;
  getaddrinfo(peer, "9990", &ai_help, &ai_r);

  struct addrinfo *ai = ai_r;
  int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if (sock < 0)
  {
    fprintf(stderr, "cant create socket %d\n", errno);
    ret = -1;
  }

  if (ret > 0)
  {
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0)
    {
      fprintf(stderr, "cant connect %d\n", errno);
      ret = -2;
    }
  }
  if (ai_r)
  {
    freeaddrinfo(ai_r);
  }

  u_long Enabled = 1;
  ioctl(sock, FIONBIO, &Enabled);
  tick_ping = nanosec_now();
  read_command(sock);

  return sock;
}
static int proc_command(int fd, int fdlog)
{
  long long now = nanosec_now();

  if (send_ack == 0)
  {
    if (cmd_len == 0)
    {
      if (now - tick_ping > 1 * 1000000000LL)
      {
        tick_ping = now;
        write(fd, PING, sizeof(PING) - 1);
        send_ack = now;
      }
    }
    else
    {
      write(fd, cmd_queue, cmd_len);
      send_ack = now;
      cmd_len = 0;
    }
    //        printf("Q");
  }
  if (send_ack > 0 && now - send_ack > timeout_ack)
  {
    fprintf(stderr, "ack timeout : %lld\n", now - send_ack);
    send_ack = 0;
    return -2;
  }

  if (read_command(fd) < 0)
    return -1;
  return 0;
}

int bm_lan_loop(int fd, int fdlog)
{
  while (fd >= 0)
  {
    if (proc_command(fd, fdlog) < 0)
      break;
  }
  return 0;
}

#ifdef SINGLE
struct model midi;
int main()
{
  int s = init_bm_lan("192.168.11.59");
  if (s > 0)
  {
    bm_lan_loop(s, -1);
  }
}
#endif
