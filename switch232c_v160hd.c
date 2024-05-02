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


extern struct model midi;

static long long send_ack = 0;

static const char QueryAll[] = "\2RQH:002100,000004;";
//static const char QueryFader[] = "\2RQH:002102,000002;";
#define QueryFader QueryAll
static const long long timeout_ack = 1*1000000000LL;




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



extern void output_csv( const char *msg, int byte );


static void doOutputTclog(int fdlog)
{
#ifdef SINGLE
	return ;
#endif
    char tc[12];
    sprintf(tc, "%08d", current_tc() );
    
    

    char buf[512];
    
    int len  =    sprintf(buf, "%c%c:%c%c:%c%c.%c%c QPL:%d,%d\n",
			  tc[0],tc[1],tc[2],tc[3],tc[4],tc[5],tc[6],tc[7],
			  midi.pgm_a-1, midi.pst_b-1);
  if( fdlog > 0 ) {
    write(fdlog, buf, len );
  }
  output_csv( buf, len );
}

static char prod[128];
static char buf[256];
static int  bi = 0;

static void ParseCmd(const char *cmd, int fdlog ){
  int qv = midi.fader;
  int fad;
  int pgm;
  int pst;
  int btn[4];
  
  //   printf(">%s\n", cmd );
  int addr = 0;
  if( sscanf( cmd, "DTH:%x,%02x%02x%04x;", &addr, &pgm, &pst, &qv )
	       == 4 ){
//     printf(">%s\t", cmd );
    if( 0&&qv == 0 ){
      // transit-complete notification may wrong.
      // ex. <1,2,255> -> <1,2,128(dn)> -> <1,2,3> -> <2,1,0>
      // this should <1,2,0> or <2,1,255>
      qv = 255;
    }
    qv/=32; // 232c(0..255) to midi(0..127)
//#    printf("%s PGM:%d, PST:%d, Fout:%d\n",cmd, pgm, pst, qv);
    midi.pgm_a = pgm - 31;
    midi.pst_b = pst - 31;
    midi.fader = qv;
    midi.tick = nanosec_now();
    disp();
    doOutputTclog( fdlog );
    return ;
  }


  if( sscanf( cmd, "VER:%[^;];", prod) == 1 ){
	  printf("'%s'\n", prod);
    return ;
  }
}

static char cmd_queue[64];
static int cmd_len = 0;

void send_232c_pgm_v160( int ch ){
  cmd_len = sprintf(cmd_queue, "\2DTH:002100,%x;", ch+31);
}
void send_232c_pst_v160( int ch ){
  cmd_len = sprintf(cmd_queue, "\2DTH:002101,%x;", ch+31);
}
void send_232c_ato_v160(){
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
      send_ack = 0;
      continue;
    }
    if( cmd == ACK ){
    }

    buf[bi++] = cmd;

    if( cmd == ';'){
      buf[bi] = 0;
      ParseCmd(buf, fdlog);
      bi = 0;
    }
  }
    
}


extern int check_v160(int fd, int fdlog)
{
  if( fd > 0 ) {
    struct termios tio={};
    tcgetattr(fd, &tio);
    //cfsetspeed(&tio, B115200);
    tio.c_iflag = IGNBRK | IGNPAR | IXON ;
    tio.c_oflag = 0;
    tio.c_cflag = B115200 | CS8 | CREAD | CLOCAL ;
    tio.c_lflag = 0;
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &tio);
  }
  prod[0] = 0;
  cmd_len = sprintf(cmd_queue, "\2VER;");
  proc_command(fd, fdlog);
  return prod[0] != 0;
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

int loop_switch232c_v160(int fd, int fdlog)
{
if(	check_v160(fd, fdlog)== 0 ) return 0;
  init_232(fd);
  while(1){
    proc_command( fd, fdlog );    
  }
  return 1;
}


