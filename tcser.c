#include <pigpiod_if2.h>
#include <stdio.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#include <string.h>

#include "switch.h"

int dateDfToFrame(int hhmmssff[]);
const int *DfFrameToDate(int f);


inline static long long nanosec_now(){
  struct timespec ts = {};
  clock_gettime(  CLOCK_REALTIME, &ts );
  return  (ts.tv_sec * 1000000000LL) + ts.tv_nsec;

}

volatile int bcd_tc;
volatile long long recv_nanosec;



  struct LTCFrame {
    unsigned int frame_units:4; ///< SMPTE framenumber BCD unit 0..9
    unsigned int user1:4;

    unsigned int frame_tens:2; ///< SMPTE framenumber BCD tens 0..3
    unsigned int dfbit:1; ///< indicated drop-frame timecode
    unsigned int col_frame:1; ///< colour-frame: timecode intentionally synchronized to a colour TV field sequence
    unsigned int user2:4;

    unsigned int secs_units:4; ///< SMPTE seconds BCD unit 0..9
    unsigned int user3:4;

    unsigned int secs_tens:3; ///< SMPTE seconds BCD tens 0..6
    unsigned int biphase_mark_phase_correction:1; ///< see note on Bit 27 in description and \ref ltc_frame_set_parity .
    unsigned int user4:4;

    unsigned int mins_units:4; ///< SMPTE minutes BCD unit 0..9
    unsigned int user5:4;

    unsigned int mins_tens:3; ///< SMPTE minutes BCD tens 0..6
    unsigned int binary_group_flag_bit0:1; ///< indicate user-data char encoding, see table above - bit 43
    unsigned int user6:4;

    unsigned int hours_units:4; ///< SMPTE hours BCD unit 0..9
    unsigned int user7:4;

    unsigned int hours_tens:2; ///< SMPTE hours BCD tens 0..2
    unsigned int binary_group_flag_bit1:1; ///< indicate timecode is local time wall-clock, see table above - bit 58
    unsigned int binary_group_flag_bit2:1; ///< indicate user-data char encoding (or parity with 25fps), see table above - bit 59
    unsigned int user8:4;

    unsigned int sync_word:16;
  };



int current_tc() {
  if( recv_nanosec == 0 ) return -1;
  return bcd_tc;
}
  



static const int SerialFromArduino = 19;

int  tcserloop(int gpfd)
{
  bb_serial_read_open( gpfd,  SerialFromArduino, 9600, 8 );

  unsigned char b[80];
  int di = 0;


  while(1)
    {
      
      int r = bb_serial_read( gpfd,  SerialFromArduino, &b[di], 80-di);
      if( r < 10 ){
	  usleep( 15 * 1000  );
      }

      if( r > 0 ){
	di += r;
	while( di >= 10 )
	{
	  if( b[8] == 0xfc && b[9] == 0xbf){
	    const struct LTCFrame *f = (const struct LTCFrame *)&b[0];
	    
	    int hhmmssff []=  { 
	      f->hours_tens*10 + f->hours_units,
	      f->mins_tens*10  + f->mins_units,
	      f->secs_tens*10  + f->secs_units,
	      f->frame_tens*10 + f->frame_units
	    };
	    bcd_tc =  hhmmssff[0] * 1000000
	      +   hhmmssff[1] * 10000
	      +   hhmmssff[2] * 100
	      +   hhmmssff[3] ;
	    
	    if(0){
	      fprintf(stderr,"baseltc=%d%d:%d%d:%d%d.%d%d, ",
		      f->hours_tens,
		      f->hours_units,
		      f->mins_tens,
		      f->mins_units,
		      f->secs_tens,
		      f->secs_units,
		      f->frame_tens,
		      f->frame_units);
	      
	    }
	    recv_nanosec = nanosec_now();

	    memmove( &b[0], &b[10], di-10);
	    di -= 10;	    

	  }
	  else{
	    memmove( &b[0], &b[1], di-1);
	    di--;
	  }
	}
      }
    }

  bb_serial_read_close( gpfd, SerialFromArduino);
}
/*

void *run_tcser( void *arg)
{
  tcserloop( (int)arg);
  return NULL;
}


int main()
{
  int gpfd = pigpio_start(NULL,NULL);

  pthread_t th_tcser;
  pthread_create( &th_tcser, NULL, run_tcser, (void*)gpfd );

  int tf =0 ;
  while(1){
    int f = current_tc();
    if( tf != f ){
      fprintf(stderr,"%d\n", f	      );

      tf = f;
    }
    usleep(10*1000);
  }
}
*/
