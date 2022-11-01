#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<termios.h>

#include "../include/common.h"
#include "../include/chunk.h"
#include "../include/debug.h"
#include "../include/vm.h"

#define MAX_HISTORY 1024

const char* module_name;
char* comm_list[MAX_HISTORY];
int hist_ptr=0;

static const char* getFileName(const char* path){
	const char* filename=strrchr(path,'/');
	if(filename==NULL)	filename=path;
	else filename++;
	return filename;
}

static char* readFile(const char* path){
	module_name=getFileName(path);
	FILE* file=fopen(path,"rb");
	if(file==NULL){
		fprintf(stderr,"Could not open file \"%s\".\n",path);
		exit(74);
	}
	fseek(file,0L,SEEK_END);
	size_t fileSize=ftell(file);
	rewind(file);
	
	char* buffer=(char*)malloc(fileSize+1);
	if(buffer==NULL){
		fprintf(stderr,"Not enough memory to read \"%s\".\n",path);
		exit(74);
	}
	size_t bytesRead=fread(buffer,sizeof(char),fileSize,file);
	if(bytesRead<fileSize){
		fprintf(stderr,"Could not read file \"%s\".\n",path);
		exit(74);
	}
	buffer[bytesRead]='\0';
	
	fclose(file);
	return buffer;
}
static void repl()
{
	char line[1024];
	char key;
	
	for(;;){
		printf(COLOR_BOLD ANSI_COLOR_BRIGHT_YELLOW"lox:> "ANSI_COLOR_RESET COLOR_OFF);
		module_name="STD";
		if(!fgets(line,sizeof(line),stdin)){
			printf("\n");
			break;
		}else{
			interpret(line);
		}
	}
}
static void runFile(const char* path){
	char* source=readFile(path);
	InterpretResult result=interpret(source);
	free(source);
	
	if(result==INTERPRET_COMPILE_ERROR) exit(65);
	if(result==INTERPRET_RUNTIME_ERROR) exit(70);
}
int main(int argc,char* argv[])
{
	initVM();
	if(argc==1){
		printf("-----------------------------------------------------------\n");
		printf("\tMathLox 0.0.0.1\t");
		#ifdef __clang_major__	//these are compiler defined macros
			fprintf(stdout,"[using clang version %d.%d]\n",__clang_major__,__clang_minor__);
		#endif
		#ifdef __GNUC__
			fprintf(stdout,"[using GCC version %d.%d]\n",__GNUC__,__GNUC_MINOR__);
		#endif
		printf("-----------------------------------------------------------\n");
		module_name="STD ";
		repl();
	}else if(argc==2){
		runFile(argv[1]);
	}
	else{
		fprintf(stderr,"Usage: clox [path]\n");
		exit(64);
	}
	freeVM();
	return 0;
}

