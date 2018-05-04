#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "switch.h"


int current_tc();


struct model m;

int getbyte( int fd )
{
    unsigned char ch = 0;
    read( fd, &ch, 1 );
    return ch;  
}


int read_until_f7( int fd) {

  while(1){
    int ch = getbyte(fd);
    if( ch == 0xf7 )
      break;
    //    printf("%x ", ch);
  }
  //printf("\n");
  return 1;
}




void swap( int *a, int*b){
  int t = *a;
  *a = *b;
  *b = t;
}

void disp()
{
  //  printf("<%d, %d, %d>\n", m.pgm_a, m.pst_b, m.fader );
}



int can_read(int fd )
{
  struct timeval tv = {0,0};
  fd_set rfds;
  FD_ZERO( &rfds );
  FD_SET(fd, &rfds );
  int r = select( fd+1, &rfds, NULL, NULL, &tv );
  if (r > 0)
    return 1;
  else
    return 0;
}


  int a_or_b = 0;
  int fading = 0;

void proc_command( int fd )
{
    int cmd = getbyte(fd);
    switch(cmd){
    case 0xf0:
      read_until_f7(fd);
      break;
    case 0xb0:
      {
	int arg1 = getbyte(fd);
	int arg2 = getbyte(fd);

	if( arg1 == 0 ){
	  a_or_b = arg2;
	  break;
	}
	if( arg1 == 0x20 ){
	  break;
	}
	if( arg1 == 0x12 ){
	  fading = arg2;
	  //printf("fade : %d\n", arg2 );
	  if( can_read( fd ) ){
	    printf("c-c\n");
	    proc_command(fd);
	  }

	  int fade_to_0 = (m.fader > 0  && fading==0);
	  int fade_to_127 = (fading == 127 && m.fader < 127);
	  if( fading != m.start_fader&&
	      (fade_to_0 || fade_to_127) ){
	    
	    printf("swap!\n");
	    swap(&m.pgm_a ,&m.pst_b );

	    printf("%d QPL:%d\n", current_tc(), m.pgm_a);
	    m.start_fader = fading;
	  }
	  m.fader = fading;
	  disp();
	      
	  break;
	}
      	printf("b0 %x %x\n", arg1, arg2);
      }
      break;
      
    case 0xc0:
      {
	int ch = getbyte(fd);
	//	printf("CHG(%d) : %d\n",  a_or_b, ch+1 );
	if( a_or_b == 0 ){
	  printf("%d QPL:%d\n", current_tc(), ch+1);
	  
	}
	if( a_or_b == 0 ){
	  m.pgm_a = ch + 1;
	}
	  
	if( a_or_b == 1 ){
	  m.pst_b = ch + 1;
	}
	disp();
	
      }
      break;
    default:
      printf("CMD %x\n", cmd );

    }
}


  static const unsigned char  init_cmd[] =
    {
      0xb0, 00, 00, 0xb0, 0x20, 00, 0xc0, 0,
      0xb0, 00, 01, 0xb0, 0x20, 00, 0xc0, 1,
      0xb0, 0x12, 00 
    };

void init_midi(int fd)
{
  m.pgm_a = 1;
  m.pst_b = 2;
  m.fader = 0;
  m.start_fader = 0;

  int r = write( fd, init_cmd, sizeof(init_cmd) );
  printf("init(%d)",r );
}

int midiloop(int fd)
{
  init_midi(fd);
  while(1){
    proc_command( fd );    
  }
}
