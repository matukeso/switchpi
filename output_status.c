#include <pigpiod_if2.h>
#include <stdio.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#include "switch.h"

static const int GPIO_VIDEOSEL = 4;

void send_pgm(int gpfd, struct model *m)
{
  gpioPulse_t pulse[8] = {
    {1<<GPIO_VIDEOSEL, 0, m->pgm_a*100},
    {0, 1<<GPIO_VIDEOSEL, m->pgm_a*100},
    {1<<GPIO_VIDEOSEL, 0, m->pgm_a*100},
    {0, 1<<GPIO_VIDEOSEL, m->pgm_a*100},
    {1<<GPIO_VIDEOSEL, 0, m->pst_b*100},
    {0, 1<<GPIO_VIDEOSEL, m->pst_b*100},
    {1<<GPIO_VIDEOSEL, 0, m->pst_b*100},
    {0, 1<<GPIO_VIDEOSEL, m->pst_b*100},
  };

  int st = 0;
  int count = 8;
  if( m->fader == 0  || m->fader == 127 ) {
    count = 4;
  }
  //  if( m->fader == 127 ){
  //   st = 4;
  //   count = 4;
  // }
  if( m->pgm_a == m->pst_b ) {
    count = 4;
  }

  
  wave_clear(gpfd);
  wave_add_generic( gpfd, count, &pulse[st] );
  int wave = wave_create(gpfd);
  wave_send_once( gpfd, wave );
  while( wave_tx_busy(gpfd ) ){
    struct timespec req_sleep = {0, 1000000};
    struct timespec rem_sleep = {};
    nanosleep( &req_sleep, &rem_sleep);
  }
  wave_delete( gpfd, wave );
}

int sendpgm_loop(int gpfd)
{
  set_mode( gpfd, GPIO_VIDEOSEL, PI_OUTPUT );
  gpio_write( gpfd, GPIO_VIDEOSEL, 0 );

  

  int timfd = timerfd_create(   CLOCK_MONOTONIC, 0 );

  struct itimerspec ts = {};
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 1e9/120;
  ts.it_value = ts.it_interval;
  //  ts.it_value.tv_sec = 10;
  timerfd_settime( timfd, 0, &ts, NULL );

  while(1){
    char buf[8];
    read( timfd, buf, sizeof(buf) );
    struct model mcopy = m;
    send_pgm( gpfd, &mcopy );
  }
  
  pigpio_stop(gpfd);
}


