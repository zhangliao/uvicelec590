/*Create input filepath's whole parent directory*/
const char * create_full_directory(char *input_filepath);

const char * create_full_directory(char *input_filepath){
char filename[1000],*line,command[1000];
static char new_string[1000];
int i=1,j;
bzero(new_string,sizeof(new_string));
new_string[0]='/';
strcpy(filename,input_filepath);
line = strtok(filename,"/");
while ( line != NULL){
	i++;
	line = strtok(NULL,"/");
	}
strcpy(filename,input_filepath);
line = strtok(filename,"/");
for(j=1;j<i-1;j++){
	strcat(new_string,line);
	strcat(new_string,"/");
	line = strtok(NULL,"/");
	}
sprintf(command,"mkdir -p %s",new_string);
system(command);
return (char *)new_string;
}
