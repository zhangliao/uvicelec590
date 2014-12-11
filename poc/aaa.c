#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include "recur_list_file.h" 
#include <time.h>
struct tm tm;
time_t t1,t2;
file_info *recur_list_file(char *input_dir);
int main (int argc, char *argv[]){
		file_info *file_item,file_array1[1024],file_array2[1024];
		char A_to_B[1000][100];
		char B_to_A[1000][100];
        bzero(file_array1,sizeof(file_info));
		bzero(file_array2,sizeof(file_info));
		// bzero(A_to_B,sizeof(A_to_B));
		// bzero(B_to_A,sizeof(B_to_A));
		file_item=recur_list_file(argv[1]);
		int i=file_item[0].filenum;//printf("i=%d\n",i);
		int k;
		for(k=0;k<i;k++){
			stpcpy(file_array1[k].filename,file_item[k].filename);
			stpcpy(file_array1[k].filetime,file_item[k].filetime);
		}  		
		file_item=recur_list_file(argv[2]);
		int j=file_item[0].filenum;//printf("j=%d\n",j);
		for(k=i;k<j;k++){
			if(file_item[k].filename){
				stpcpy(file_array2[k-i].filename,file_item[k].filename);
				stpcpy(file_array2[k-i].filetime,file_item[k].filetime);
			}
		}
		int p=j-i,m,n,flag,A_to_B_index=0,B_to_A_index=0;
		char temp_a[100],temp_b[100];
		for(k=0;k<i;k++){
			printf("Dir A is %s  %s\n",file_array1[k].filename,file_array1[k].filetime);
		}
		for(k=0;k<p;k++){
			printf("Dir B is %s  %s\n",file_array2[k].filename,file_array2[k].filetime);
		}
		for(m=0;m<i;m++){
			for(n=0;n<p;n++){
				flag=0;
				strcpy(temp_a,file_array1[m].filename);
				ReplaceStr(temp_a,argv[1],"");
				// printf("temp_a:%s\n",temp_a);
				strcpy(temp_b,file_array2[n].filename);
				ReplaceStr(temp_b,argv[2],"");
				// printf("temp_b:%s\n",temp_b);
				if(!strcmp(temp_a,temp_b)){
					flag=1;
					strptime(file_array1[m].filetime, "%a %b %d %H:%M:%S %Y", &tm);
					time_t t1 = mktime(&tm);
					strptime(file_array2[n].filetime, "%a %b %d %H:%M:%S %Y", &tm);
					time_t t2 = mktime(&tm);					
					// printf("same file:%s   %s\n",temp_a,temp_b);
					if(difftime(t1,t2)>0){
						strcpy(A_to_B[A_to_B_index],file_array1[m].filename);
						A_to_B_index++;
						}
					break;
				}
				
			}
			if (flag == 0){
				// printf("ONLY IN A:%s\n",file_array1[m].filename);
				strcpy(A_to_B[A_to_B_index],file_array1[m].filename);
				A_to_B_index++;
			}
		}
		for(n=0;n<p;n++){
			for(m=0;m<i;m++){
				flag=0;
				strcpy(temp_a,file_array1[m].filename);
				ReplaceStr(temp_a,argv[1],"");
				// printf("temp_a:%s\n",temp_a);
				strcpy(temp_b,file_array2[n].filename);
				ReplaceStr(temp_b,argv[2],"");
				// printf("temp_b:%s\n",temp_b);
				if(!strcmp(temp_a,temp_b)){
					flag=1;
					strptime(file_array1[m].filetime, "%a %b %d %H:%M:%S %Y", &tm);
					time_t t1 = mktime(&tm);
					strptime(file_array2[n].filetime, "%a %b %d %H:%M:%S %Y", &tm);
					time_t t2 = mktime(&tm);					
					// printf("same file:%s   %s\n",temp_a,temp_b);
					if(difftime(t1,t2)<0){
					// printf("same file:%s   %s\n",temp_a,temp_b);
						strcpy(B_to_A[B_to_A_index],file_array2[n].filename);
						B_to_A_index++;
					}
					break;
				}
				
			}
			if (flag == 0){
				// printf("ONLY IN B:%s\n",file_array2[n].filename);
				strcpy(B_to_A[B_to_A_index],file_array2[n].filename);
				B_to_A_index++;
			}
		}
		for(k=0;k<A_to_B_index;k++){
			printf("A_to_B:%s\n",A_to_B[k]);
		}
		for(k=0;k<B_to_A_index;k++){
			printf("B_to_A:%s\n",B_to_A[k]);
		}
return 0;


}