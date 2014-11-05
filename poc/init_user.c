#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   char command[50];
   FILE *fp;
    char *find, *hist;
    int  len, pos=0, hl=0, i;
    char c;
    fp = fopen( "/etc/passwd", "r" );
    find = argv[1];
    len = (int)strlen( find );
    hist = malloc( len );
    memset( hist, 0, len );
    while ( !feof( fp )) {
        c = fgetc( fp );
        if ( find[pos++] == c ) {
            if ( pos == len ) {
                printf( "User Exist!\n" );
                return 1;
            }
        }
        else {
            if ( pos > 0 ) {
                pos = 0;
                for ( i = 0; i < len - 1; i++ )
                    if ( 0 == memcmp( hist+len-i-1, find, i + 1 )) {
                    pos = i;
                }
            }
        }
        memmove( hist, hist + 1, len - 1 );
        hist[len-1] = c;
    }
    printf( "Creating user....\n" );
	sprintf(command,"useradd %s -md /home/%s",argv[1],argv[1]);
	system(command);
	sprintf(command,"echo %s:%s |chpasswd",argv[1],argv[2]);
	system(command);
	return(0);
} 