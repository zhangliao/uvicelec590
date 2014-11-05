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
#include <openssl/ssl.h>
#include <openssl/err.h>  
#define oops(msg) { perror(msg); exit(errno); }  
void ShowCerts(SSL * ssl)
{
	X509 *cert;
	char *line;
	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL) {
		printf("Digital Certification Information:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Certification: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	} 
	else
		printf("无证书信息！\n");
}
  
int main(int argc, char *argv[]){  
		
        if(argc != 2)oops("which file not specified?");  
  
        int sd = socket(PF_INET, SOCK_STREAM, 0);  
        if(sd == -1) oops("socket");  
  
        struct sockaddr_in address;  
        bzero(&address, sizeof(address));  
        address.sin_family = AF_INET;  
        address.sin_addr.s_addr = inet_addr("192.168.0.1");  
        address.sin_port =  8888;  
		
		SSL_CTX *ctx;
		SSL *ssl;
		
		/* SSL 库初始化，参看 ssl-server.c 代码 */
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		ctx = SSL_CTX_new(SSLv23_client_method());
		if (ctx == NULL) {
			ERR_print_errors_fp(stdout);
			exit(1);
		}
		
		
        int connect_ret = connect(sd, (struct sockaddr*)&address, sizeof(address));  
        if(connect_ret == -1) oops("connect");  
		
		/* 基于 ctx 产生一个新的 SSL */
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, sd);
		
		/* 建立 SSL 连接 */
		if (SSL_connect(ssl) == -1)
			ERR_print_errors_fp(stderr);
		else {
			printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
			ShowCerts(ssl);
		}
		
		SSL_write(ssl,argv[1],strlen(argv[1]));

  
        close(sd);  
        return EXIT_SUCCESS;  
}  

