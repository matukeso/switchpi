#include <pigpiod_if2.h>
#include <stdio.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#include <string.h>

#include "switch.h"

int  tcserloop(int gpfd)
;
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
