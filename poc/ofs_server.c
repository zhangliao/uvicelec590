#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h> 
#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h> 
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <time.h>  
#include <signal.h>
#include <errno.h>  
#include <openssl/ssl.h>
#include <openssl/err.h>
#define oops(msg) { perror(msg); exit(errno); }  
typedef struct {
	char username[100];
	char password[100];
	char directory[200];
	char query_object[200];
	char commit_sha[100];
	int operation;
} reginfo;

typedef struct {
  char  commit_sha[50];
  char  author[50];
  char  date[50];
  char  message[50];
} version_info;

reginfo userinfo;  
version_info revsion[10];
git_repository *repo = NULL;
int create_user(char *username, char *password, char *directory );
int create_repository(char *username);
static void create_initial_commit(git_repository *repo,char *username);
int get_commit_time(char *username,char *filename);
static void checkout(char filename[], char commit_sha[]);
void set_fl(int fd, int flags);
void clr_fl(int fd, int flags);
int main(){  
		FILE *fp;
		char dirfile[100];
		char *localdir;
		SSL_CTX *ctx;
		int n;
		pid_t pid;
		signal(SIGCHLD, SIG_IGN);
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
				char message[100]; 
				char filename[100];  
				char filePath[100];	
				char buff[2048];  				
                int client_fd = accept(sd, NULL, NULL); 
				int error,status=0;			
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
				if((pid=fork())>0)
				{
					close(client_fd);
					continue;
				}
				else if(pid==0)
				{		
					while(1)
					{	
						n = SSL_read(ssl, &userinfo, sizeof(userinfo));
						if(n == -1)
						{
							perror("fail to read");
							exit(1);
							close(sd);
						}
						else if(n == 0)
						{
							printf("Connection Closed!\n");
							close(sd);
							exit(1);
						}
						printf("operation is:%d\n",userinfo.operation);
						switch( userinfo.operation ) 
						{
							case 1:
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
								break;
							case 2:
								printf("Username is:%s\n",userinfo.username);
								printf("Passowrd:%s\n",userinfo.password);
								printf("Directory:%s\n",userinfo.directory);
								create_user(userinfo.username,userinfo.password,userinfo.directory);
								create_repository(userinfo.username);
								create_initial_commit(repo,userinfo.username);
								break;
							case 3:
								printf("QUERY OBJECT IS:%s\n",userinfo.query_object);
								get_commit_time(userinfo.username,userinfo.query_object);
								SSL_write(ssl, (char*)revsion, sizeof(version_info)*10);
								break;
							case 4:
								checkout(userinfo.query_object,userinfo.commit_sha);
								break;
							case 5:
								n = SSL_read(ssl, filename, sizeof(filename));
								filename[n]='\0';
								printf("file name:%s\n",filename);
								sprintf(filePath, "/home/%s/%s", userinfo.username,filename); 
								printf("file path:%s\n",filePath);								
								int fd = open(filePath, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);  
								if(fd == -1) oops("open");  			
								ssize_t length; 
								set_fl(client_fd, O_NONBLOCK); 
								while(1)
								{  
									length = SSL_read(ssl, buff, sizeof(buff));
									//printf("length is:%d\n",(int)length);									
									if ( length == -1 )
									{	
										clr_fl(client_fd, O_NONBLOCK);
										break;
									}
									write(fd, buff, length); 
								}   
								close(fd);
								printf("File write finished!\n");
								break;								
						}
					}
					close(client_fd);
					exit(0);
				}
				else
				{
					printf("fork error.\n");
					exit(1);
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

int get_commit_time(char *username,char *filename){
    FILE *pf;
    char command[200];
    char data[5000];
	int i=0;
	memset( data, '\0', sizeof(data) );
    sprintf(command, "git -C /home/%s log %s",username,filename);
	//printf("%s\n",command);
    pf = popen(command,"r");
    if(!pf){
      fprintf(stderr, "Could not open pipe for output.\n");
      return;
    }
	fread( data, sizeof(char), sizeof(data), pf); 
	
	char * line;
	//printf ("Splitting data......\n");
	line = strtok (data,"\n");
	memset((char*)revsion, 0, sizeof(version_info)*10);
	while (line != NULL && i < 10)
	{	
		strncpy(revsion[i].commit_sha,line+7,strlen(line));
		line = strtok (NULL, "\n");
		strcpy(revsion[i].author,line);		
		line = strtok (NULL, "\n");
		strncpy(revsion[i].date,line+7,35);		
		line = strtok (NULL, "\n");
		strcpy(revsion[i].message,line);	
		line = strtok (NULL, "\n");
		i++;
		printf("\n");
	}
	if (pclose(pf) != 0)
    fprintf(stderr," Error: Failed to close command stream \n");
    return 0;
}

static void checkout(char filename[], char commit_sha[]){
	git_checkout_opts opts = GIT_CHECKOUT_OPTS_INIT;
	char *paths[] = {filename};
	opts.paths.strings = paths;
	opts.paths.count = 1;
	opts.checkout_strategy = GIT_CHECKOUT_FORCE ;
	git_commit *commit;
	git_oid oid;
	git_repository *repo = NULL;
	char repo_path[100];
	sprintf(repo_path,"/home/%s",userinfo.username);
	if (git_repository_open(&repo,repo_path) < 0 )
		printf("Unable to open the repository\n");
	// Get "Commit ID"
	if (git_oid_fromstr(&oid, commit_sha) < 0)
		printf("Could not find the commit object\n");
	if (git_commit_lookup(&commit, repo, &oid) < 0) 
		printf("Could not lookup the commit id\n");

	// Do the checkout
	if (git_checkout_tree(repo, (git_object*)commit, &opts) < 0)
		printf("Checkout faild\n");

	// Clean up
	git_commit_free(commit);
}

void set_fl(int fd, int flags) /* flags are file status flags to turn on */
{
        int     val;

        if ((val = fcntl(fd, F_GETFL, 0)) < 0)
                printf("fcntl F_GETFL error\n");

        val |= flags;       /* turn on flags */

        if (fcntl(fd, F_SETFL, val) < 0)
                printf("fcntl F_SETFL error\n");
}

void clr_fl(int fd, int flags) /* flags are file status flags to turn off */
{
        int     val;

        if ((val = fcntl(fd, F_GETFL, 0)) < 0)
                printf("fcntl F_GETFL error\n");

        val &= ~flags;      /* turn flags off */

        if (fcntl(fd, F_SETFL, val) < 0)
                printf("fcntl F_SETFL error\n");
}