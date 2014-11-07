#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<unistd.h> 
#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h> 
  
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
typedef struct {
	char username[100];
	char password[100];
	char directory[200];
	char operation[10];
} reginfo;

reginfo userinfo;  
git_repository *repo = NULL;
int create_user(char *username, char *password, char *directory );
int create_repository(char *username);
static void create_initial_commit(git_repository *repo,char *username);
int main(){  
		FILE *fp;
		char dirfile[100];
		char *localdir;
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
		int on=1;
	
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        int bind_return =  bind(sd, (struct sockaddr *)&address, sizeof(address));  
        if(bind_return == -1) oops("bind");  
  
        int listen_return = listen(sd, 1);  
        if(listen_return == -1) oops("listen");  
  
        while(1){
				SSL *ssl;
				int length;
                int client_fd = accept(sd, NULL, NULL);  
                if(client_fd == -1) oops("accept");  
				
				/* 基于 ctx 产生一个新的 SSL */
				ssl = SSL_new(ctx);
				/* 将连接用户的 socket 加入到 SSL */
				SSL_set_fd(ssl, client_fd);
				/* 建立 SSL 连接 */
				if (SSL_accept(ssl) == -1) {
					perror("accept");
					close(client_fd);
					break;
				}
							
                char message[100]; 
				while(1)
				{	
					SSL_read(ssl, &userinfo, sizeof(userinfo));
					if ( strcmp(userinfo.operation,"LOGIN") == 0 )
					{	
						memset(dirfile, 0, sizeof(dirfile) );
						sprintf(dirfile,"/home/%s/.dirfile",userinfo.username);
						printf("User %s try to login...\n",userinfo.username);
						fp = fopen(dirfile,"rb");
						if (fp)
						{
							fseek (fp, 0, SEEK_END);
							length = ftell (fp);
							fseek (fp, 0, SEEK_SET);
							localdir = malloc (length + 1);
              memset(localdir, 0, length+1);
							if (localdir)
							{
								fread (localdir, 1, length, fp);
							}
							fclose (fp);
						}
						printf("user local directory is:%s\n",localdir);
						SSL_write(ssl, localdir, strlen(localdir));
					}
					else if( strcmp(userinfo.operation,"REG") == 0 )
					{
						printf("message is:%s\n",userinfo.username);
						printf("Passowrd:%s\n",userinfo.password);
						printf("Directory:%s\n",userinfo.directory);
						create_user(userinfo.username,userinfo.password,userinfo.directory);
						create_repository(userinfo.username);
						create_initial_commit(repo,userinfo.username);
					}
				}
        }  
        return EXIT_SUCCESS;  
} 

int create_user(char *username, char *password ,char *directory ){
	char command[50];
	FILE *fp;
    char *hist;
    int  len, pos=0, hl=0, i;
    char c;
	char dirfile[100];
    fp = fopen( "/etc/passwd", "r" );
    len = (int)strlen( username );
    hist = malloc( len );
    memset( hist, 0, len );
    while ( !feof( fp )) {
        c = fgetc( fp );
        if ( username[pos++] == c ) {
            if ( pos == len ) {
                printf( "User Exist!\n" );
                return 1;
            }
        }
        else {
            if ( pos > 0 ) {
                pos = 0;
                for ( i = 0; i < len - 1; i++ )
                    if ( 0 == memcmp( hist+len-i-1, username, i + 1 )) {
                    pos = i;
                }
            }
        }
        memmove( hist, hist + 1, len - 1 );
        hist[len-1] = c;
    }
	fclose(fp);
	printf( "Creating user: %s\n",username);
	sprintf(command,"useradd %s -md /home/%s",username,username);
	system(command);
	sprintf(command,"echo %s:%s |chpasswd",username,password);
	system(command);
	sprintf(command,"rm -f /home/%s/examples.desktop",username);
	system(command);
	
	char *last;
	len=strlen(directory); // save length of string
	last=directory+len-1; // make pos point to last char of string
	if ( strcmp(last,"/") )
	{
		last++;
		*last = '/';
	}
	printf("Directory revised to:%s\n",directory);
	sprintf(dirfile,"/home/%s/.dirfile",username);
	fp = fopen(dirfile,"a");
	fprintf(fp, "%s", directory);
	fclose(fp);
	return 0;
}

int create_repository(char *username){	

	git_config *cfg = NULL;
	char home_path[200];
	sprintf(home_path,"/home/%s",username);
	if (git_repository_init(&repo, home_path, 0) < 0)
		printf("Repository initialize failed\n");
	
	git_repository_free(repo);
	
	if (git_repository_open(&repo,home_path) < 0 )
		printf("Unable to open the repository\n");
	
	if (git_repository_config(&cfg, repo) < 0)
		printf("Unable to open the config\n");
	
	git_config_set_string(cfg, "user.name", username);
	git_config_set_string(cfg, "user.email", username);
	git_config_free(cfg);
	
    printf("User repository created.\n");
	return 0;
}

static void create_initial_commit(git_repository *repo,char *username){
	git_signature *sig;
	git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;
	
	if (git_signature_now(&sig, username, username) < 0)
		printf("Unable to signature\n");
	if (git_repository_index(&index, repo) < 0)
		printf("Could not open repository index\n");
	if (git_index_write_tree(&tree_id, index) < 0)
		printf("Unable to write initial tree from index\n");
	
	git_index_free(index);
	
	if (git_tree_lookup(&tree, repo, &tree_id) < 0)
		printf("Could not look up initial tree\n");
	if (git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,NULL, "Initial commit", tree, 0) < 0)
		printf("Could not create the initial commit\n");
	
	git_tree_free(tree);
	git_signature_free(sig);
}
