#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <termios.h>

#include "switch.h"

int can_read_fd(int fd, int ms_timeout)
{
  struct timeval tv = {ms_timeout/1000, (ms_timeout%1000)*1000};
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  int r = select(fd + 1, &rfds, NULL, NULL, &tv);
  if (r > 0)
    return 1;
  else
    return 0;
}
extern void output_csv(const char *msg, int byte);

void doOutputTclog(int fdlog)
{

  char tc[12];
  sprintf(tc, "%08d", current_tc());

  char buf[512];

  int len = sprintf(buf, "%c%c:%c%c:%c%c.%c%c QPL:%d,%d\n",
                    tc[0], tc[1], tc[2], tc[3], tc[4], tc[5], tc[6], tc[7],
                    midi.pgm_a - 1, midi.pst_b);
  if (fdlog > 0)
  {
    write(fdlog, buf, len);
  }

  output_csv(buf, len);
}


const char *ttyUSB(int i)
{
  static char port[64];
  sprintf(port, "/dev/ttyUSB%d", i);
  return port;
}

int open232c(const char *name)
{
  int fd = open(name, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd > 0)
  {
    struct termios tio;
    tcgetattr(fd, &tio);
    tio.c_iflag = IGNBRK | IGNPAR | IXON;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL;
    tio.c_lflag = 0;

    cfsetspeed(&tio, B9600);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tio);
  }
  return fd;
}
int openusb232c(int i)
{
  if (i < 0)
    return -1;
  return open232c(ttyUSB(i));
}
