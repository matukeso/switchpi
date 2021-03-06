#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <termios.h>

#include "switch.h"

int open232c( const char *name ){
  int  fd = open( name, O_RDWR | O_NOCTTY | O_NDELAY);
  if( fd > 0 ) {
    struct termios tio;
    tcgetattr(fd, &tio);
    tio.c_iflag = IGNBRK | IGNPAR | IXON ;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL ;
    tio.c_lflag = 0;
    
    cfsetspeed(&tio, B9600);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tio);
  }
  return fd;  
}
