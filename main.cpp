#include "dialog.h"
#include <QApplication>

#include <pigpiod_if2.h>

#include "switch.h"

static int ttyUSB_for_send = -1;
extern "C" int get_usb232_by_serial(const char *serial);
extern "C" int loop_switch232c(int fd, int fdlog);
void *run_midi(void *arg)
{
  int ttyUSB_for_midi = 0;
  if (ttyUSB_for_send == 0)
    ttyUSB_for_midi = 1;

  int fd = openusb232c(ttyUSB_for_midi);
  struct midiarg *marg = (struct midiarg *)arg;
  if (fd >= 0)
  {
    printf("main midi port = ttyUSB%d. (%d)\n", ttyUSB_for_midi, fd);
    loop_switch232c(fd, marg->fd_log);
  }
  else
  {
    printf("main midi port = OC8(%d)\n", marg->fd_midi);
    ocloop(marg->fd_midi, marg->fd_log);
  }
  return NULL;
}

void *run_tcser(void *arg)
{
  tcserloop((int)arg);
  return NULL;
}

void *run_sendpgm(void *arg)
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

int main(int argc, char *argv[])
{

  int gpfd = pigpio_start(NULL, NULL);
  if (gpfd < 0)
    return 99;
  printf("gpfd = %d\n", gpfd);

  g_gpfd = gpfd;

  pthread_t th_tcser;
  pthread_create(&th_tcser, NULL, run_tcser, (void *)gpfd);

  ttyUSB_for_send = get_usb232_by_serial("DN05J9A1");
  if (ttyUSB_for_send >= 0)
  {
    int fd = openusb232c(ttyUSB_for_send);
    printf("920MHz wireless enabled. port = ttyUSB%d, (%d)\n", ttyUSB_for_send, fd);

    pthread_t th_sendpgm;
    pthread_create(&th_sendpgm, NULL, run_sendpgm, (void *)fd);
  }

  if (1)
  {
    struct midiarg marg = {0, 0};

    marg.fd_midi = gpfd;

    if (argc > 1)
    {
      marg.fd_log = open("all.log", O_RDWR | O_APPEND, 0600);
      printf("open(%s) %d\n", argv[1], marg.fd_log);
    }

    pthread_t thmidi;
    pthread_create(&thmidi, NULL, run_midi, (void *)&marg);
  }

  QApplication a(argc, argv);
  Dialog w;
  w.show();

  return a.exec();
}
