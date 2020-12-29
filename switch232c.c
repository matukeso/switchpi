#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <termios.h>

#include "switch.h"

// #define SINGLE

static const char STX = 0x02;
static const char ACK = 0x06;

extern int current_tc();


struct model midi;

static long long send_ack = 0;

static const char QueryFader[] = "\2QPL:7;";
static const char QueryAll  [] = "\2QPL:8;";
static const long long timeout_ack = 1*1000000000LL;

static long long nanosec_now(){
  struct timespec ts = {};
  clock_gettime(  CLOCK_REALTIME, &ts );
  return  (ts.tv_sec * 1000000000LL) + ts.tv_nsec;
}



static int getbyte( int fd )
{
    unsigned char ch = 0;
    if( read( fd, &ch, 1 ) != 1) return -1;
    return ch;  
}



static void disp()
{
#ifdef SINGLE
    printf("<%d, %d, %d>\n", midi.pgm_a, midi.pst_b, midi.fader );
#endif
}



static int can_read(int fd )
{
  struct timeval tv = {1,0};
  fd_set rfds;
  FD_ZERO( &rfds );
  FD_SET(fd, &rfds );
  int r = select( fd+1, &rfds, NULL, NULL, &tv );
  if (r > 0)
    return 1;
  else
    return 0;
}


static int a_or_b = 0;
static int fading = 0;
volatile int g_fd;

static void doOutputTclog(int fdlog)
{

    char tc[12];
    sprintf(tc, "%08d", current_tc() );
    
    

    char buf[512];
    
    int len  =    sprintf(buf, "%c%c:%c%c:%c%c.%c%c QPL:%d,%d\n",
			  tc[0],tc[1],tc[2],tc[3],tc[4],tc[5],tc[6],tc[7],
			  midi.pgm_a-1, midi.pst_b-1);
  if( fdlog > 0 ) {
    write(fdlog, buf, len );
  }
  int fd = g_fd;
  if( fd > 0 ){
    write(fd, buf, len );
  }
}


static char buf[256];
static int  bi = 0;

static void ParseCmd(const char *cmd, int fdlog ){
  int qv = midi.fader;
  int fad;
  int pgm;
  int pst;
  int btn[4];
  
  //   printf(">%s\n", cmd );
  if( sscanf( cmd, "QPL:%d,%d,%d,%d,%d,%d,%d,%d;",
	      &pgm, &pst, &btn[0], &btn[1], &btn[2], &btn[3],
	      &fad, &qv) == 8 ){
    //   printf(">%s\t", cmd );
    if( qv == 0 ){
      // transit-complete notification may wrong.
      // ex. <1,2,255> -> <1,2,128(dn)> -> <1,2,3> -> <2,1,0>
      // this should <1,2,0> or <2,1,255>
      qv = 255;
    }
    qv/=2; // 232c(0..255) to midi(0..127)
    //  printf("PGM:%d, PST:%d, Fout:%d\n", pgm, pst, qv);
    midi.pgm_a = pgm + 1;
    midi.pst_b = pst + 1;
    midi.fader = qv;
    disp();
    doOutputTclog( fdlog );
    return ;
  }
  if( sscanf( cmd, "QPL:%d;", &qv ) == 1 ){
    qv/=2; // 232c(0..255) to midi(0..127)
    if( qv != midi.fader ){
#ifdef SINGLE
      printf("f:%d\n", qv );
#endif
      midi.fader = qv;
    }
    return ;
  }
}

static char cmd_queue[64];
static int cmd_len = 0;

void send_232c_pgm( int ch ){
  cmd_len = sprintf(cmd_queue, "\2PGM:%d;", ch-1);
}
void send_232c_pst( int ch ){
  cmd_len = sprintf(cmd_queue, "\2PST:%d;", ch-1);
}
void send_232c_ato(){
  cmd_len = sprintf(cmd_queue, "\2ATO;");
}



static void proc_command( int fd, int fdlog )
{
  long long now = nanosec_now();
  
  if( send_ack == 0 ){
    if( cmd_len == 0 ){
        write(fd, QueryFader, sizeof(QueryFader) - 1);
	send_ack = now;
    }else{
        write(fd, cmd_queue, cmd_len);
	send_ack = now;
	cmd_len = 0;
    }
	//        printf("Q");
     return ;
  }

  if( send_ack > 0 && now - send_ack > timeout_ack ){
    fprintf(stderr,"ack timeout : %lld\n", now-send_ack);
    send_ack = 0;
    return ;
  }

  if( !can_read(fd) )
    return ;
  while( 1 ){ 
    int cmd = getbyte(fd);
    if( cmd < 0)
      return ;

    // STX. clear buffer.
    if( cmd == STX ){
      bi = 0;
      continue;
    }
    if( cmd == ACK ){
      send_ack = 0;
    }

    buf[bi++] = cmd;

    if( cmd == ';'){
      buf[bi] = 0;
      ParseCmd(buf, fdlog);
      bi = 0;
    }
  }
    
}


static void init_232(int fd)
{
  midi.pgm_a = 1;
  midi.pst_b = 2;
  midi.fader = 0;
  midi.start_fader = 0;

  write(fd, QueryAll, sizeof(QueryAll) - 1);
  send_ack = nanosec_now();
  //  printf("INIT\n");
}

int loop_switch232c(int fd, int fdlog)
{
  init_232(fd);
  while(1){
    proc_command( fd, fdlog );    
  }
}

int open232c( const char *name ){
  int  fd = open( name, O_RDWR | O_NOCTTY | O_NDELAY);
  if( fd > 0 ) {
    struct termios tio;
    tcgetattr(fd, &tio);
    cfsetspeed(&tio, B9600);
    tio.c_iflag = IGNBRK | IGNPAR | IXON ;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL ;
    tio.c_lflag = 0;
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tio);
  }
  return fd;  
}



#ifdef SINGLE
int main(int argc, char *argv[] )
{
  int fd_midi = 0;
  int fd_log = 0;
  
  fd_midi = open232c( "/dev/ttyUSB0");
  printf("openmidi %d\n", fd_midi);


  if( argc > 1 ){
    fd_log = open( argv[1], O_RDWR | O_CREAT, 0600);
    printf("open(%s) %d\n", argv[1], fd_log);
  }

  loop_switch232c(fd_midi, 2 );

  
}


int current_tc(){
  return 0;
}
#endif
