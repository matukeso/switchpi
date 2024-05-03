#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <termios.h>

#include "switch.h"

// #define SINGLE

struct model midi;

enum
{
	MODE_V160 = 160,
	MODE_V1SD = 1,
	MODE_BLACKMAGIC = 111,
};

int controller_232c_mode = 0;
extern void set_bm_mode()
{
	controller_232c_mode = MODE_BLACKMAGIC;
}


extern void send_232c_pgm_v160(int ch);
extern void send_232c_pgm_v1(int ch);
extern void send_232c_pst_v160(int ch);
extern void send_232c_pst_v1(int ch);
extern void send_232c_ato_v160();
extern void send_232c_ato_v1();
extern int check_v160(int fd, int fdlog);
extern int check_v1sd1(int fd, int fdlog);
extern int loop_switch232c_v160(int fd, int fdlog);
extern int loop_switch232c_v1(int fd, int fdlog);

extern int init_bm_lan(const char *peer);
extern int bm_lan_loop(int fd, int fdlog);
extern void send_232c_pgm_bmlan(int ch);
extern void send_232c_pst_bmlan(int ch);
extern void send_232c_ato_bmlan();


void send_232c_pgm(int ch)
{
	if (controller_232c_mode == MODE_V160)
		send_232c_pgm_v160(ch);
	if (controller_232c_mode == MODE_V1SD)
		send_232c_pgm_v1(ch);
	if (controller_232c_mode == MODE_BLACKMAGIC)
		send_232c_pgm_bmlan(ch);
}
void send_232c_pst(int ch)
{
	if (controller_232c_mode == MODE_V160)
		send_232c_pst_v160(ch);
	if (controller_232c_mode == MODE_V1SD)
		send_232c_pst_v1(ch);
	if (controller_232c_mode == MODE_BLACKMAGIC)
		send_232c_pst_bmlan(ch);
}
void send_232c_ato()
{
	if (controller_232c_mode == MODE_V160)
		send_232c_ato_v160();
	if (controller_232c_mode == MODE_V1SD)
		send_232c_ato_v1();
	if (controller_232c_mode == MODE_BLACKMAGIC)
		send_232c_ato_bmlan();
}

int loop_switch232c(int fd, int fdlog)
{
	while (1)
	{
		struct timespec ts = {1, 0}, rem = {0, 0};
		if (check_v160(fd, fdlog))
		{
			controller_232c_mode = MODE_V160;
			loop_switch232c_v160(fd, fdlog);
		}
		if (check_v1sd1(fd, fdlog))
		{
			controller_232c_mode = MODE_V1SD;
			loop_switch232c_v1(fd, fdlog);
		}

		nanosleep(&ts, &rem);
	}
}
