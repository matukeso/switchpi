#include <pigpiod_if2.h>
#include <stdio.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>

#include "switch.h"

static void send_pgm(int gpfd, struct model *m)
{
  int pgm_bit = m->pgm_a == 0 ? 0 : 1 << (m->pgm_a - 1);
  int pst_bit = m->pst_b == 0 ? 0 : 1 << (m->pst_b - 1);
  int pgm_bit = m->pgm_a == 0 ? 0 : 1<<(m->pgm_a-1);
  int pst_bit = m->pst_b == 0 ? 0 : 1<<(m->pst_b-1);
  if( m->fader == 127 || m->fader == 0) 
	  pst_bit = 0;

  char buf[32];
  int len = sprintf(buf, "TXDT %02x00\r\n", pgm_bit | pst_bit);
  write(gpfd, buf, len);

  //  printf("%d, %d => %s",m->pgm_a, m->pst_b, buf );

  char rbuf[4];
  read(gpfd, rbuf, 4);
}

int sendpgm_loop(int fd)
{
  printf("sendpgm im920(%d)\n", fd);
  if( fd > 0 )
    {
      struct termios tio;
      tcgetattr(fd, &tio);
      tio.c_iflag = IGNBRK | IGNPAR | IXON ;
      tio.c_oflag = 0;
      tio.c_cflag = CS8 | CREAD | CLOCAL ;
      tio.c_lflag = 0;
      
      cfsetspeed(&tio, B19200);
      tcflush(fd, TCIFLUSH);
      tcsetattr(fd, TCSANOW, &tio);
    }
  int timfd = timerfd_create(   CLOCK_MONOTONIC, 0 );

  struct itimerspec ts = {};
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 100 * 1000 * 1000;
  ts.it_value = ts.it_interval;
  //  ts.it_value.tv_sec = 10;
  timerfd_settime(timfd, 0, &ts, NULL);

  while (1)
  {
    char buf[8];
    read(timfd, buf, sizeof(buf));
    struct model mcopy = midi;
    send_pgm(fd, &mcopy);
  }
}


#ifdef SINGLE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct model midi;

int main()
{
  int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

  midi.pgm_a = 2;
  sendpgm_loop(fd);
}
#endif
