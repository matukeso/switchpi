#include "dialog.h"
#include <QApplication>

#include <pigpiod_if2.h>

#include "switch.h"

void *run_midi(void *arg)
{
  struct midiarg *marg = (struct midiarg*)arg;
  ocloop( marg->fd_midi, marg->fd_log);
  return NULL;
}

void *run_tcser( void *arg)
{
  tcserloop( (int)arg);
  return NULL;
}

void *run_sendpgm( void *arg)
{
  sendpgm_loop((int)arg);

  return NULL;
}
  
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int g_gpfd;
volatile int g_fd;
struct model midi;

int main(int argc, char *argv[])
{

  int gpfd = pigpio_start(NULL,NULL);
  if( gpfd < 0 )
    return 99;
  printf("gpfd = %d\n", gpfd );

  g_gpfd = gpfd;

  if( 1  ){
    struct midiarg marg = {0, 0};
    
    marg.fd_midi = gpfd;    
    
    if( argc > 1 ){
      marg.fd_log = open( "all.log", O_RDWR | O_APPEND, 0600);
      printf("open(%s) %d\n", argv[1], marg.fd_log);
    }
    
    pthread_t thmidi;
    pthread_create( &thmidi, NULL, run_midi, (void*)&marg );
  }
  

  pthread_t th_tcser;
  pthread_create( &th_tcser, NULL, run_tcser, (void*)gpfd );

  {
    int fd = open232c( "/dev/ttyUSB0");
    
    
    pthread_t th_sendpgm;
    pthread_create( &th_sendpgm, NULL, run_sendpgm, (void*)fd );
  }
  
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
