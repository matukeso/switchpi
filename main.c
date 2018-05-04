#include <pigpiod_if2.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int midiloop(int fd, int fdlog);
extern int tcserloop(int fd);

int sendpgm_loop(int gpfd);

struct midiarg
{
  int fd_midi;
  int fd_log;
}  ;
void *run_midi(void *arg)
{
  struct midiarg *marg = (struct midiarg*)arg;
  midiloop( marg->fd_midi, marg->fd_log);
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
  struct midiarg marg = {0, 0};

  marg.fd_midi = open( "/dev/midi1", O_RDWR);
  printf("openmidi %d\n", marg.fd_midi);


  if( argc > 1 ){
    marg.fd_log = open( argv[1], O_RDWR | O_CREAT, 0600);
    printf("open(%s) %d\n", argv[1], marg.fd_log);
  }
  


  pthread_t thmidi;
  pthread_create( &thmidi, NULL, run_midi, (void*)&marg );

  pthread_t th_tcser;
  pthread_create( &th_tcser, NULL, run_tcser, (void*)gpfd );

  sendpgm_loop(gpfd);
}
