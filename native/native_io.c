#include<stdarg.h>
#include<stdlib.h>
#include<time.h>
#include<stdio.h>
#include<string.h>

#include "../include/memory.h"
#include "../include/native_io.h"
#include "../include/object.h"
#include "../include/value.h"
#include "../include/vm.h"
#include "../include/compiler.h"

#define BASE_PATH ""

static void Error(const char* format, ...){
	va_list args;
	va_start(args,format);
	vfprintf(stderr,format,args);
	va_end(args);
	fputs("\n",stderr);
	return;
}
extern const char* module_name;

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
		return NULL;
	}
	fseek(file,0L,SEEK_END);
	size_t fileSize=ftell(file);
	rewind(file);
	
	char* buffer=(char*)malloc(fileSize+1);
	if(buffer==NULL){
		fprintf(stderr,"Not enough memory to read \"%s\".\n",path);
		return NULL;
	}
	size_t bytesRead=fread(buffer,sizeof(char),fileSize,file);
	if(bytesRead<fileSize){
		fprintf(stderr,"Could not read file \"%s\".\n",path);
		return NULL;
	}
	buffer[bytesRead]='\0';
	
	fclose(file);
	return buffer;
}

Value clockNative(int argCount,Value* args){
	return NUMBER_VAL((double)clock()/CLOCKS_PER_SEC);	
}
Value scanNumberNative(int argCount,Value* args){
	if(argCount==1){
		printf("%s",AS_CSTRING(args[0]));
		double value=AS_NUMBER(args[1]);
		scanf("%lf",&value);
		return NUMBER_VAL(value);
	}else if(argCount==0){
		double value=AS_NUMBER(args[0]);
		scanf("%lf",&value);
		return NUMBER_VAL(value);
	}
	else
		Error("Wrong number of arguments to scanNumber()");
}
Value scanStringNative(int argCount,Value* args){
	if(argCount==1){
		printf("%s",AS_CSTRING(args[0]));
		char* str=ALLOCATE(char,1024);
		fgets(str,1024,stdin);
		int len=strlen(str);
		ObjString* res=takeString(str,len);
		return OBJ_VAL(res);
	}
}
Value printNative(int argCount,Value* args){
	printValue(args[0],stdout);
}
Value printNewNative(int argCount,Value* args){
	printValue(args[0],stdout);
	printf("\n");
}

Value lox_native_format(int argCount,Value* args){
	int format_prec=(int)AS_NUMBER(args[1]);
	double x=AS_NUMBER(args[0]);
	size_t len=0;
	len=snprintf(NULL,len,"%.*f",format_prec,x);
	len++;
	char* chars=ALLOCATE(char,len);
	snprintf(chars,len,"%.*f",format_prec,x);
	return OBJ_VAL(takeString(chars,len-1));
}

Value lox_native_import_module(int argCount,Value* args){
	const char* file_path=AS_CSTRING(args[0]);
	char* prog_buffer=readFile(file_path);
	if(prog_buffer==NULL) {
		fprintf(stderr,"Failed to import module:%s\n",module_name);
		return NIL_VAL;
	}
	Value value;
	/*if(tableGet(&vm.modules,OBJ_VAL(module_name),&value)){
		fprintf(stderr,"Module %s already imported\n",module_name);
	}else{*/
	ObjFunction* import_fun=compile(prog_buffer);
	if(import_fun==NULL){
		fprintf(stderr,"Failed to import module:%s\n",module_name);
	}else{
		//tableSet(&vm.modules,Amodule_name,NIL_VAL);
		//push(OBJ_VAL(import_fun));
		ObjClosure* closure=newClosure(import_fun);
		//pop();
		//push(OBJ_VAL(closure));
		//pop();
		 return (callValue(OBJ_VAL(closure),0));
		}
	//}
}
