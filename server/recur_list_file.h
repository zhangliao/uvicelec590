#include <ftw.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

typedef struct {
	char filename[200];
	char filetime[100];
	int filenum;
} file_info;
file_info *recur_list_file(char *input_dir);
int list(const char *name, const struct stat *status, int type);
int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
file_info file_item[1024];


int fileitem_index=0;
int len1,len2;
file_info *recur_list_file(char *input_dir) {
	bzero(file_item,sizeof(file_info)*1024);  
	ftw(input_dir, list, 1);
return file_item;
}

int list(const char *name, const struct stat *status, int type) {
if(type == FTW_NS)
	return 0;
if(type == FTW_F && strstr(name,"/.") == NULL && name) {
	// printf("in file name is :%s\n",name);
	strcpy(file_item[fileitem_index].filename,name);
	strcpy(file_item[fileitem_index].filetime,ctime(&status->st_mtime));
	fileitem_index++;}
	file_item[0].filenum=fileitem_index;
return 0;
}

int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr){
        int  StringLen;
        char caNewString[1000];
        char *FindPos = strstr(sSrc, sMatchStr);
        if( (!FindPos) || (!sMatchStr) )
                return -1;
        while( FindPos )
        {
                memset(caNewString, 0, sizeof(caNewString));
                StringLen = FindPos - sSrc;
                strncpy(caNewString, sSrc, StringLen);
                strcat(caNewString, sReplaceStr);
                strcat(caNewString, FindPos + strlen(sMatchStr));
                strcpy(sSrc, caNewString);
                FindPos = strstr(sSrc, sMatchStr);
        }
        return 0;
}