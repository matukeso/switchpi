#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <termios.h>

#include "switch.h"

void output_csv(const char *msg, int byte) {}

int main(int argc, char *argv[])
{
  int fd_midi = 0;
  int fd_log = 0;

  fd_midi = open232c("/dev/ttyUSB0");
  printf("openmidi %d\n", fd_midi);

  if (argc > 1)
  {
    fd_log = open(argv[1], O_RDWR | O_CREAT, 0600);
    printf("open(%s) %d\n", argv[1], fd_log);
  }

  loop_switch232c(fd_midi, 2);
}

int current_tc()
{
  return 0;
}
