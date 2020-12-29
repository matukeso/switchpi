#pragma once
struct model {
  int pgm_a;
  int pst_b;
  int fader;
  int start_fader;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern struct model midi;
extern volatile int g_fd;
extern int current_tc();

 int loop_switch232c(int fd, int fdlog);
 int open232c( const char *name );

  int ocloop(int fd, int fdlog);

 int midiloop(int fd, int fdlog);
 int tcserloop(int fd);

 int sendpgm_loop(int gpfd);

struct midiarg
{
  int fd_midi;
  int fd_log;
};

  void send_232c_pgm( int ch );
  void send_232c_pst( int ch );
  void send_232c_ato();

#ifdef __cplusplus
}
#endif /* __cplusplus */


