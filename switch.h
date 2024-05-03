#pragma once
#include <time.h>

struct model {
  int pgm_a;
  int pst_b;
  int fader;
  int start_fader;
  long long tick;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern struct model midi;
extern int current_tc();

 int loop_switch232c(int fd, int fdlog);

 int ocloop(int fd, int fdlog);

 int midiloop(int fd, int fdlog);
 int tcserloop(int fd);

 int sendpgm_loop(int gpfd);


 const char *ttyUSB(int i);
 int open232c( const char *name );
 int openusb232c( int i );
 extern int can_read_fd(int fd, int ms_timeout);
void doOutputTclog(int fdlog);

#define strcmp_findimm( str, find ) memcmp( str, find, sizeof(find)-1) == 0

struct midiarg
{
  int fd_midi;
  int fd_log;
};

  void send_232c_pgm( int ch );
  void send_232c_pst( int ch );
  void send_232c_ato();

inline static long long nanosec_now(){
  struct timespec ts = {};
  clock_gettime(  CLOCK_REALTIME, &ts );
  return  (ts.tv_sec * 1000000000LL) + ts.tv_nsec;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */


