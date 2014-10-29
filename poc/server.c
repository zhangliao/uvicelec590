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
#include <openssl/ssl.h>
#include <openssl/err.h>
#define oops(msg) { perror(msg); exit(errno); }  
  
int main(){  
		
		SSL_CTX *ctx;

		SSL_library_init();/* SSL 库初始化 */
		
		OpenSSL_add_all_algorithms();/* 载入所有 SSL 算法 */
		
		SSL_load_error_strings();/* 载入所有 SSL 错误消息 */
		
		ctx = SSL_CTX_new(SSLv23_server_method());/* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
		
		if (ctx == NULL) {
		ERR_print_errors_fp(stdout);
		exit(1);
		}/* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
		
		
		if (SSL_CTX_use_certificate_file(ctx, "cacert.pem", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
		}/* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
		
		if (SSL_CTX_use_PrivateKey_file(ctx, "privkey.pem", SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stdout);
		exit(1);
		}/* 载入用户私钥 */
		
		if (!SSL_CTX_check_private_key(ctx)) {
		ERR_print_errors_fp(stdout);
		exit(1);
		}/* 检查用户私钥是否正确 */
		
		
		
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
				SSL *ssl;
                int client_fd = accept(sd, NULL, NULL);  
                if(client_fd == -1) oops("accept!!!");  
				
				/* 基于 ctx 产生一个新的 SSL */
				ssl = SSL_new(ctx);
				/* 将连接用户的 socket 加入到 SSL */
				SSL_set_fd(ssl, client_fd);
				/* 建立 SSL 连接 */
				if (SSL_accept(ssl) == -1) {
				perror("accept@@@");
				close(client_fd);
				break;
				}
				
				
                time_t current_time = time(NULL); 
				
                char fileName[100]="";  
				char filePath[100]="";
				SSL_read(ssl, fileName, sizeof(fileName));
				printf("file name:%s",fileName);
                sprintf(filePath, "%s", fileName);  
                int fd = open(filePath, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);  
                if(fd == -1) oops("open");  
  
                char buff[1024];  
                ssize_t length;  
                while((length = SSL_read(ssl, buff, sizeof(buff))) > 0){  
                        write(fd, buff, length);  
                }  
  
                close(fd);  
        }  
        return EXIT_SUCCESS;  
} 