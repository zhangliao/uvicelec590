
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <sys/types.h> 
#include <sys/stat.h> 


char abfilename1[1024][100]; //最多1024个文件，每个文件的绝对路径名字最多支持100个字符
char abfilename2[1024][100];
char abfilename[1024][100];
char Atruepathname[1024][100];//只在A中有的文件,此数组下的文件需要进行A到B的push
char Btruepathname[1024][100];//只在B中有的文件,此数组下的文件需要进行B到A的push
//char ABtruepathname[1024][50];//AB中都有但是不同的文件
void dir_scan( char *path, char *file); 
int comparename(char *line, int filecount, char truepathname[1024][100]);
long int fetchfilename(char *file);
long int filechangetime(char *file1,char *file2);
int count = 0;    int count1 = 0;  int count2 = 0;  
					int Afilecount=0;int Bfilecount=0;
int main(int argc, char *argv[]) 
{ 
                  struct stat s1,s2; 
                  if(argc != 3){ 
                                  printf("two direction requried\n"); 
                                  exit(1); 
                  } 
                  if((lstat(argv[1], &s1) < 0)|(lstat(argv[2], &s2) < 0)){ 
                                  printf("lstat error\n"); 
                                  exit(2); 
                  } 
                  if(!S_ISDIR(s1.st_mode)){ 
                                  printf("%s is not a direction name\n", argv[1]); 
                                  exit(3); 
                  } 
                 if(!S_ISDIR(s2.st_mode)){ 
                                  printf("%s is not a direction name\n", argv[2]); 
                                  exit(3); 
                  } 
				  int len1=strlen(argv[1]);
				 // printf("%d",len1);
				  int len2=strlen(argv[2]);
				  memset(abfilename1,0,sizeof(abfilename1));
				  memset(abfilename2,0,sizeof(abfilename2));
				  int i,j ;
                  dir_scan("", argv[1]);   	
				  memcpy(abfilename1,abfilename,sizeof(abfilename));
				  count1=count;
				  memset(abfilename,0,sizeof(abfilename));
				  count=0;
                  dir_scan("", argv[2]);   	
                  count2=count;				  
				  memcpy(abfilename2,abfilename,sizeof(abfilename));

                    char command[100];
                    sprintf(command,"diff -qr %s %s > result.txt",argv[1],argv[2]);
                    system(command);

					FILE *fp=NULL;
					fp=fopen("result.txt","r");
					if(NULL==fp)
                     {printf("open result.txt error\n");return-1;}
					//f1=fopen("result1.txt","w+");
					/*if(NULL==f1)
					{printf("open result1.txt error\n");return-1;}
					f2=fopen("result2.txt","w+");
					if(NULL==f2)
                    {printf("open result2.txt error\n");return-1;}*/
					char line[100];
                    memset(line,0,sizeof(line));

                    while((fgets(line,100,fp))!=NULL)
                   {
				        //printf("%s",line);
						char file1[]="Only in ";
						strcat(file1,argv[1]);
						if(!strncasecmp(line,file1,8+len1))//file only in argv[1]
						{
						 if(comparename(line, Afilecount,Atruepathname)==0)
						{ Afilecount++;}
						else continue;
						}
						char file2[]="Only in ";
						strcat(file2,argv[2]);
						if(!strncasecmp(line,file2,8+len2))//file only in argv[2]
						{//printf("%s",line);
						if(comparename(line, Bfilecount,Btruepathname)==0)
						{Bfilecount++;}
						else continue;
						//fwrite(line,sizeof(line),1,f2);
						}
						
						if(!strncasecmp(line,"files",5))//file both in argv[1] and argv[2] but different
						{//printf("%s",line);
						//fwrite(line,sizeof(line),1,f2);
						long int time=0;
						
						time=fetchfilename(line);
						
						}
						memset(line,0,sizeof(line));
				   
				   }
                    
				 close(fp);
				 int a=0;
				for(a=0;a<Afilecount;a++) 
				{
				 printf("%s\n",Atruepathname[a]);
				}
				int b=0;
				for(b=0;b<Bfilecount;b++) 
				{
				 printf("%s\n",Btruepathname[b]);
				}
				 
				 
				 
				 
                 exit(0); 
  } 
    
  void dir_scan(char *path,char *file) 
  {               
                  //memset(abfilename,0,sizeof(abfilename));
                  struct stat s; 
                  DIR *dir; 
                  struct dirent *dt; 
                  char dirname[100]; 
                  
                  memset(dirname, 0, 100*sizeof(char)); 
                  strcpy(dirname, path); 
    
                  if(lstat(file, &s) < 0){ 
                                  printf("lstat error\n"); 
                  } 
                  //whether it is dir  for the first it must be a dir 
                  if(S_ISDIR(s.st_mode)){ 
                                  strcpy(dirname+strlen(dirname), file); //save the current dir to 上一层目录
                                  strcpy(dirname+strlen(dirname), "/"); 
                                  if((dir = opendir(file)) == NULL){ 
                                                  printf("opendir %s/%s error\n",dirname,file); 
                                                  exit(4); 
                                  } 
                                  if(chdir(file) < 0) { 
                                                  printf("chdir error\n"); 
                                                  exit(5); 
                                  } 
                                  while((dt = readdir(dir)) != NULL){ //依次打开该目录下的所有文件
                                                  if(dt->d_name[0] == '.'){ 
                                                                  continue; 
                                                  } 
    
                                                  dir_scan(dirname, dt->d_name); 
                                  } 
                                  if(chdir("..") < 0){ 
                                                  printf("chdir error\n"); 
                                                  exit(6); 
                                  } 
                  }else{          
				                  char filename[100];
				                  memset(filename, 0, 100*sizeof(char)); 
                                  sprintf( filename,"%s%s", dirname, file); 
								  //printf("%s\n",filename);
								  //printf("%s\n",file);
								  struct stat sf;
								  memset(&sf,0,sizeof(sf));
								  if(lstat(filename, &sf) < 0){ 
                                  printf("lstat error\n"); }
								  strcpy(abfilename[count],filename);
								  //printf("%s\n",abfilename[count]);
								  count++; 
                 
                  } 
  }
  
  int comparename(char *line, int filecount,char truepathname[1024][100])
  {
                         //printf("%s",line);
						 //fwrite(line,sizeof(line),1,f1);
						 char *pointer;
						 char *pathpointer=line;
						 pathpointer=pathpointer+8;
						 char Apathname[1024][100];
						 sprintf(Apathname[filecount],"%s",pathpointer);
						 //printf(Apathname[filecount],"%s\n");//文件的全部路径
						 pointer=strchr(line,':');
						 pointer=pointer+2;
						 char Afilename[1024][100];
						 sprintf(Afilename[filecount],"%s",pointer);
						 //printf(Afilename[filecount],"%s\n");//文件名字
						 char AAfilename[100];//一维文件名字
						 memset(AAfilename,0,sizeof(AAfilename));
						 sprintf(AAfilename,"%s",Afilename[filecount]);
						 if(AAfilename[0]=='.'){return 2;}
						 int filelength=strlen(AAfilename);
						 AAfilename[filelength-1]='\0';
						 int k=0;
						 char AAtruepathname[100]={0};
						// memset(AAtruepathname,0,sizeof(AAtruepathname));
						 char AApathname[100];//一维
						 memset(AApathname,0,sizeof(AApathname));
						 sprintf(AApathname,"%s",Apathname[filecount]);
						 int pathlength=strlen(AApathname);
						  AApathname[pathlength-1]='\0';
						 //printf("%s\n",AApathname);
						
						 
						 for(k=0;k<(pathlength-filelength-2);k++)
						 {   
						    AAtruepathname[k]=AApathname[k];
						 }
						 //printf("%s\n",AAfilename);//文件的路径
						 strcat(AAtruepathname,"/");
						 strcat(AAtruepathname,AAfilename);
						 strcpy(truepathname[filecount],AAtruepathname);
						 //printf("%s\n",truepathname[filecount]);//文件的绝对路径
						 return 0;
}

long int fetchfilename(char *line)
{
    //char line[50]="Files /home/yuejiao and /user/yuejiao differ\n";
	line[strlen(line)-1]='\0';
	char *pointer=NULL;
	pointer=line+6;
	char *pointer1=NULL;
	pointer1=strchr(pointer,' ');
	int file1length=pointer1-pointer;//路径1的长度
	int i =0;
	char file1[100];
	memset(file1,0,sizeof(file1));
	for(i=0;i<file1length;i++)
	{file1[i]=line[6+i];}
	///printf("%s\n",file1);
	pointer1=pointer1+5;
	char *pointer2=strchr(pointer1,' ');
	int j=0;
	char file2[100];
	memset(file2,0,sizeof(file2));
	int file12ength=pointer2-pointer1;//路径2的长度
	for(j=0;j<(pointer2-pointer1);j++)
	{file2[j]=line[6+file1length+5+j];}
	//printf("%s\n",file2);
	filechangetime(file1,file2);
	//printf("%ld\n",filechangetime(file1,file2));//<0 file2 比较新 需要将B中文件更新给A 即将该文件放到B中
	if(filechangetime(file1,file2)<0)
	{
	 strcpy(Btruepathname[Bfilecount++],file2);
	}
	else
	{
	  strcpy(Atruepathname[Afilecount++],file1);
	}
	return filechangetime(file1,file2);
}

long int filechangetime(char *file1,char *file2)
{
    struct stat sf1,sf2;
	memset(&sf1,0,sizeof(sf1));
	memset(&sf2,0,sizeof(sf2));
	if(lstat(file1, &sf1) < 0)
	{ printf("lstat file1 error\n"); }
	//printf("%s\n",ctime(&sf1.st_mtime));
	//printf("%ld\n",sf1.st_mtime);
	if(lstat(file2, &sf2) < 0)
	{ printf("lstat file2 error\n"); }
	//printf("%s\n",ctime(&sf2.st_mtime));
	//printf("%ld\n",sf2.st_mtime);
	long int time=sf1.st_mtime-sf2.st_mtime;
	//if((sf1.st_mtime-sf2.st_mtime)<0)
	return time;
}