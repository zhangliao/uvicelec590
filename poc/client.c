#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<unistd.h>  
  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<sys/sendfile.h>  
#include<netinet/in.h>  
#include<arpa/inet.h>  
  
#include<sys/stat.h>  
#include<fcntl.h>  
  
#include<errno.h>  
  
#define oops(msg) { perror(msg); exit(errno); }  
  
int main(int argc, char *argv[]){  
        if(argc != 2)oops("which file not specified?");  
  
        int sd = socket(PF_INET, SOCK_STREAM, 0);  
        if(sd == -1) oops("socket");  
  
        struct sockaddr_in address;  
        bzero(&address, sizeof(address));  
        address.sin_family = AF_INET;  
        address.sin_addr.s_addr = inet_addr("192.168.0.1");  
        address.sin_port =  8888;  
  
        int connect_ret = connect(sd, (struct sockaddr*)&address, sizeof(address));  
        if(connect_ret == -1) oops("connect");  
  
        int fd = open(argv[1], O_RDONLY);  
        off_t offset = 0;  
        struct stat fileStatus;  
        int fstat_ret = fstat(fd, &fileStatus);  
        if(fstat_ret == -1)oops("fstat");  
		//char fileName[50]; 
		printf("The filename is: %s\n",argv[1]);
		send(sd,argv[1],strlen(argv[1]),0);
        ssize_t length = sendfile(sd, fd, &offset, fileStatus.st_size);  
        if(length == -1)  
                {oops("sendfile");}  
        else  
                fprintf(stdout, "%ld characters have been send to server.\n", length);  
  
        close(fd);  
  
        close(sd);  
        return EXIT_SUCCESS;  
}  