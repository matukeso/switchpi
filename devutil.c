#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int get_usb232_by_serial(const char *serial)
{
	int slen = strlen(serial);
	int devno = -1;
	int i;
	struct stat s;
	for( i=0; i<10; i++){
		char cmd[256];
		sprintf(cmd, "/dev/ttyUSB%d", i);
		if( stat( cmd, &s ) < 0 )
		       continue;	

		sprintf(cmd, "udevadm info -q property /dev/ttyUSB%d", i);
		FILE *fp = popen( cmd, "r" );
		if( fp ){
			char line[256];
			while( fgets( line, 256, fp ) ){
				if( memcmp(line, "ID_SERIAL",9 ) != 0 ) continue;
				
				char *pos = strchr( line, '=' );
				if(pos){
//					printf("%d:'%.*s'\n", i, slen,  pos+1); 
					if( memcmp(pos+1, serial, slen ) == 0 ){
						devno = i;
					}	
				}

			}

			pclose(fp);
		}
	}

	return devno;
}
#if SINGLE
int main()
{
int r =	get_usb232_by_serial( "DN05J9A1");
printf("find. %d\n", r );

}
#endif

