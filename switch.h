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

#ifdef __cplusplus
}
#endif /* __cplusplus */


