#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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


struct model {
  int pgm_a;
  int pst_b;
  int fader;
}m;


void swap( int *a, int*b){
  int t = *a;
  *a = *b;
  *b = t;
}

void disp()
{
  printf("<%d, %d, %d>\n", m.pgm_a, m.pst_b, m.fader );
}

int main()
{
  int fd = open( "/dev/dmmidi1", O_RDONLY);
  printf("open %d\n", fd);

  int a_or_b = 0;
  int fading = 0;
  while(1){
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

	  if( m.fader<64 && fading >= 64 ||
	      fading < 64 && m.fader >= 64 ){
	    swap(&m.pgm_a ,&m.pst_b );
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
	//	printf("CHG(%d) : %d\n", a_or_b, ch );

	if( a_or_b == 0 ){
	  m.pgm_a = ch + 1;
	}
	  
	if( a_or_b == 1 ){
	  m.pst_b = ch + 1;
	}
	disp();
	
      }
      break;
    deafult:
      printf("CMD %x\n", cmd );

    }
    
  }
}
