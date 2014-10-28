#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<unistd.h>  
  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h>  
#include<arpa/inet.h>  
  
#include<sys/stat.h>  
#include<fcntl.h>  
#include<time.h>  
  
#include<errno.h>  
#define oops(msg) { perror(msg); exit(errno); }  
  
int main(){  
        int sd = socket(PF_INET, SOCK_STREAM, 0);  
        if(sd == -1) oops("socket");  
  
        struct sockaddr_in address;  
        bzero((void *)&address, sizeof(address));  
        address.sin_family = AF_INET;  
        address.sin_addr.s_addr = inet_addr("192.168.0.1");  
        address.sin_port =  8888;  
  
        int bind_return =  bind(sd, (struct sockaddr *)&address, sizeof(address));  
        if(bind_return == -1) oops("bind");  
  
        int listen_return = listen(sd, 1);  
        if(listen_return == -1) oops("listen");  
  
        while(1){  
                int client_fd = accept(sd, NULL, NULL);  
                if(client_fd == -1) oops("accept");  
  
                time_t current_time = time(NULL); 
				
                char fileName[100]="";  
				char filePath[100]="";
				recv(client_fd, fileName, sizeof(fileName), 0);
				printf("file name:%s",fileName);
                sprintf(filePath, "%s", fileName);  
                int fd = open(filePath, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);  
                if(fd == -1) oops("open");  
  
                char buff[1024];  
                ssize_t length;  
                while((length = recv(client_fd, buff, sizeof(buff), 0)) > 0){  
                        write(fd, buff, length);  
                }  
  
                close(fd);  
        }  
        return EXIT_SUCCESS;  
} 