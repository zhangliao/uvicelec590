#include <stdio.h>
#define COMMAND_LEN 80
#define DATA_SIZE 512000
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h>
typedef struct
{
  char  commit_sha[50];
  char  author[50];
  char  date[50];
  char  message[50];
} version_info;

version_info revsion[10];
int main(int argc,char *argv[]){
    FILE *pf;
    char command[COMMAND_LEN];
    char data[DATA_SIZE];
	int i=0;
	memset( data, '\0', sizeof(data) );
    sprintf(command, "git log %s",argv[1]);
    pf = popen(command,"r");
    if(!pf){
      fprintf(stderr, "Could not open pipe for output.\n");
      return;
    }
	fread( data, sizeof(char), sizeof(data), pf); 
	
	char * line;
	//printf ("Splitting data......\n");
	line = strtok (data,"\n");
	memset(&revsion, 0, sizeof(version_info));
	while (line != NULL && i < 10)
	{	
		strncpy(revsion[i].commit_sha,line+7,strlen(line));
		printf ("commit_sha:%s\n",revsion[i].commit_sha);

		line = strtok (NULL, "\n");
		strcpy(revsion[i].author,line);
		printf ("author:%s\n",revsion[i].author);
		
		line = strtok (NULL, "\n");
		strncpy(revsion[i].date,line+7,35);
		printf ("date:%s\n",revsion[i].date);
		
		line = strtok (NULL, "\n");
		strcpy(revsion[i].message,line);
		printf ("message:%s\n",revsion[i].message);
		
		line = strtok (NULL, "\n");
		i++;
		printf("\n");
	}
	/*
	for (i=0;i<11;i++){
	    if (revsion[i].commit_sha) printf ("commit_sha:%s\n",revsion[i].commit_sha);
	    if (revsion[i].author) printf ("author:%s\n",revsion[i].author);
	    if (revsion[i].date) printf ("date:%s\n",revsion[i].date);
	    if (revsion[i].message) printf ("message:%s\n",revsion[i].message);
	}
	*/
	if (pclose(pf) != 0)
    fprintf(stderr," Error: Failed to close command stream \n");
    return 0;
}