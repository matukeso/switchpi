#include <pigpiod_if2.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int midiloop(int fd);
extern int tcserloop(int fd);

int sendpgm_loop(int gpfd);

void *run_midi(void *arg)
{
  midiloop( (int)arg );
  return NULL;
}

void *run_tcser( void *arg)
{
  tcserloop( (int)arg);
  return NULL;
}

int main(int argc, char * argv[])
{
  int gpfd = pigpio_start(NULL,NULL);

  int fd = open( "/dev/midi1", O_RDWR);
  printf("open %d\n", fd);


  pthread_t thmidi;
  pthread_create( &thmidi, NULL, run_midi, (void*)fd );

  pthread_t th_tcser;
  pthread_create( &th_tcser, NULL, run_tcser, (void*)gpfd );

  sendpgm_loop(gpfd);
}
