#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <pigpiod_if2.h>

#include "switch.h"


int current_tc();


//static struct model midi;

static void swap( int *a, int*b){
  int t = *a;
  *a = *b;
  *b = t;
}

static void disp()
{
  //  printf("<%d, %d, %d>\n", midi.pgm_a, midi.pst_b, midi.fader );
}


static void doOutputTclog(int fdlog)
{

    char tc[12];
    sprintf(tc, "%08d", current_tc() );
    
    

    char buf[512];
    
    int len  =    sprintf(buf, "%c%c:%c%c:%c%c.%c%c QPL:%d,%d\n",
			  tc[0],tc[1],tc[2],tc[3],tc[4],tc[5],tc[6],tc[7],
			  midi.pgm_a-1, midi.pst_b);
  if( fdlog > 0 ) {
    write(fdlog, buf, len );
  }
  int fd = g_fd;
  if( fd > 0 ){
    write(fd, buf, len );
  }
}


static const int tar_oc_pins[]={13, 16,19,20,21,26};
static const int interval_millisec = 30;

struct oc_state {
  int pins[6];
};

static struct oc_state read_oc_state(int gpfd)
{
  struct oc_state oc;
  int i;
  for( i=0; i<6; i++){
    // オープンコレクタ＋プルアップなので、論理が逆になる。
    oc.pins[i] = !gpio_read( gpfd, tar_oc_pins[i] );
  }
  return oc;
}

static void extract_oc_state( const struct oc_state *s, int *p1, int *p2){
  int i;
  int mode = 0;
  *p1 = 0;
  *p2 = 0;
  
  for( i=0; i<6; i++){
    if( s->pins[i] ){
      if( mode ==0 ){
	*p1 = i+1;
	mode++;
      }
      else if( mode ==1 ){
	*p2 = i+1;
	mode++;
      }
    }
  }
}

static void proc_command( int fd, int fdlog )
{

  struct oc_state prev_oc_state = {0};

  while(1)
    {
      struct oc_state oc = read_oc_state(fd);

     extract_oc_state( &oc, &midi.pgm_a, &midi.pst_b );

      if( midi.pst_b == 0 ){
	midi.start_fader = midi.pgm_a;
	midi.fader = 0;
      }
      else{
	if( midi.pst_b == midi.start_fader ){
	  swap( &midi.pgm_a, &midi.pst_b );
	}
	midi.fader = 64;
      }


      if( 0 != memcmp( &prev_oc_state , &oc, sizeof(oc)) ){
	    doOutputTclog( fdlog );
      }
      prev_oc_state = oc;
      {
	static const struct timespec ms200 = { 0, interval_millisec*1000*1000 };
	struct timespec rem;
	nanosleep( &ms200,  &rem );
	
      }
    }
 
}
void init_oc(int fd)
{

  printf("init open-collector %d\n", fd);
  midi.pgm_a = 1;
  midi.pst_b = 2;
  midi.fader = 0;
  midi.start_fader = 1;


    int i;
    for( i=0; i<6; i++ ){
	set_mode( fd, tar_oc_pins[i], PI_INPUT );
	set_pull_up_down( fd, tar_oc_pins[i], PI_PUD_UP );
    }
}

int ocloop(int fd, int fdlog)
{
  init_oc(fd);
  while(fd >=0){
    proc_command( fd, fdlog );    
  }
  return 0;
}
