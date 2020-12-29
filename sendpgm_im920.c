#include <pigpiod_if2.h>
#include <stdio.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>

#include "switch.h"


void send_pgm(int gpfd, struct model *m)
{
  int pgm_bit = m->pgm_a == 0 ? 0 : 1<<(m->pgm_a-1);
  int pst_bit = m->pst_b == 0 ? 0 : 1<<(m->pst_b-1);
  
  char buf[32];
  int len = sprintf(buf, "TXDT 0%x00", pgm_bit | pst_bit );
  write( gpfd, buf, len ); 
}

int sendpgm_loop(int fd)
{
  if( fd > 0 )
    {
      struct termios tio;
      tcgetattr(fd, &tio);
      cfsetspeed(&tio, B19200);
      tio.c_iflag = IGNBRK | IGNPAR | IXON ;
      tio.c_oflag = 0;
      tio.c_cflag = CS8 | CREAD | CLOCAL ;
      tio.c_lflag = 0;
      
      tcflush(fd, TCIFLUSH);
      tcsetattr(fd, TCSANOW, &tio);
    }
  int timfd = timerfd_create(   CLOCK_MONOTONIC, 0 );
  

  struct itimerspec ts = {};
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 100*1000*1000;
  ts.it_value = ts.it_interval;
  //  ts.it_value.tv_sec = 10;
  timerfd_settime( timfd, 0, &ts, NULL );

  while(1){
    char buf[8];
    read( timfd, buf, sizeof(buf) );
    struct model mcopy = midi;
    send_pgm( fd, &mcopy );
  }
}

void send_232c_pgm( int ch ){

}
void send_232c_pst( int ch ){

}
void send_232c_ato(){

}


#ifdef SINGLE
int main()
{
  sendpgm_loop();
}
#endif
