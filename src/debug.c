#include<stdio.h>
#include "../include/debug.h"
#include "../include/value.h"
#include "../include/object.h"


void disassembleChunk(Chunk* chunk,const char* name,FILE* file_ptr){
	printf("== %s ==\n",name);
	for(int offset=0;offset<chunk->count;)
		offset=disassembleInstruction(chunk,offset,file_ptr);
}
static int constantInstruction(const char* name,Chunk* chunk,int offset,FILE* file_ptr){
	uint8_t constant=chunk->code[offset+1];
	fprintf(file_ptr,"%-16s %4d '",name,constant);
	printValue(chunk->literals.values[constant],file_ptr);
	fprintf(file_ptr,"\n");
	return offset+2;
}
static int invokeInstruction(const char* name,Chunk* chunk,int offset,FILE* file_ptr){
	uint8_t constant=chunk->code[offset+1];
	uint8_t argCount=chunk->code[offset+2];
	fprintf(file_ptr,"%-16s (%d args) %4d '",name,argCount,constant);
	printValue(chunk->literals.values[constant],file_ptr);
	fprintf(file_ptr,"'\n");
	return offset+3;
}
static int simpleInstruction(const char* name,int offset,FILE* file_ptr){
	fprintf(file_ptr,"%s\n",name);
	return offset+1;	
}
static int jumpInstruction(const char* name,int sign,Chunk* chunk,int offset,FILE* file_ptr){
	uint16_t jump=(uint16_t)(chunk->code[offset+1] <<8);
	jump |= chunk->code[offset+2];
	fprintf(file_ptr,"%-16s %4d-> %d\n",name,offset,offset+3+sign*offset);
	return offset+3;
}
static int byteInstruction(const char* name,Chunk* chunk,int offset,FILE* file_ptr){
	uint8_t slot=chunk->code[offset+1];
	fprintf(file_ptr,"%-16s  %4d\n",name,slot);
	return offset+2;
}
int disassembleInstruction(Chunk*chunk,int offset,FILE* file_ptr){
	fprintf(file_ptr,"%04d ",offset);
	if(offset>0 && chunk->lines[offset]==chunk->lines[offset-1]){
		fprintf(file_ptr,"    |");
	}
	else{
		fprintf(file_ptr,"%4d ",chunk->lines[offset]);
	}
	uint8_t instruction=chunk->code[offset];
	switch(instruction)
	{
		case OP_CONSTANT:
			return constantInstruction("OP_CONSTANT",chunk,offset,file_ptr);
		case OP_NIL:
			return simpleInstruction("OP_NIL",offset,file_ptr);
		case OP_TRUE:
			return simpleInstruction("OP_TRUE",offset,file_ptr);
		case OP_FALSE:
			return simpleInstruction("OP_FALSE",offset,file_ptr);
		case OP_RETURN:
			return simpleInstruction("OP_RETURN",offset,file_ptr);
		case OP_NEGATE:
			return simpleInstruction("OP_NEGATE",offset,file_ptr);
		case OP_ADD:
      return simpleInstruction("OP_ADD", offset,file_ptr);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset,file_ptr);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset,file_ptr);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset,file_ptr);
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset,file_ptr);
    case OP_POP:
    	return simpleInstruction("OP_POP", offset,file_ptr);
    case OP_NOT:
    	return simpleInstruction("OP_NOT",offset,file_ptr);
   	case OP_DEFINE_GLOBAL:
      return simpleInstruction("OP_DEFINE_GLOBAL", offset,file_ptr);
    case OP_GET_GLOBAL:
      return simpleInstruction("OP_GET_GLOBAL", offset,file_ptr);
   	case OP_SET_GLOBAL:
      return simpleInstruction("OP_SET_GLOBAL", offset,file_ptr);
    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL",chunk, offset,file_ptr);
   	case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL",chunk, offset,file_ptr);
   	case OP_LOOP:
   		return jumpInstruction("OP_LOOP",-1,chunk,offset,file_ptr);
    case OP_JUMP:
      return jumpInstruction("OP_JUMP",1,chunk, offset,file_ptr);
   	case OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE",1,chunk, offset,file_ptr);
    case OP_INCREMENT:
      return simpleInstruction("OP_INCREMENT", offset,file_ptr);
    case OP_DECREMENT:
      return simpleInstruction("OP_DECREMENT", offset,file_ptr);
    case OP_CALL:
      return byteInstruction("OP_CALL",chunk, offset,file_ptr);
    case OP_BUILD_LIST:
      return simpleInstruction("OP_BUILD_LIST", offset,file_ptr);
    case OP_INDEX_SUBSCR:
      return simpleInstruction("OP_INDEX_SUBSCR", offset,file_ptr);
    case OP_STORE_SUBSCR:
      return simpleInstruction("OP_STORE_SUBSCR", offset,file_ptr);
    case OP_BUILD_MATRIX:
      return simpleInstruction("OP_BUILD_MATRIX", offset,file_ptr);
    case OP_BUILD_MAP:
    case OP_CLOSURE:{
    	offset++;
    	uint8_t constant=chunk->code[offset++];
    	printf("%-16s %4d ","OP_CLOSURE",constant);
    	printValue(chunk->literals.values[constant],file_ptr);
    	printf("\n");
    	
    	ObjFunction* function=AS_FUNCTION(chunk->literals.values[constant]);
    	for(int j=0;j<function->upvalueCount;j++){
    		int isLocal=chunk->code[offset++];
    		int index=chunk->code[offset++];
    		fprintf(file_ptr,"%04d		|			%s	%d\n",offset-2,isLocal?"local":"upvalue",index);
    	}
    	return offset;
    }
    case OP_GET_UPVALUE:
      return constantInstruction("OP_GET_UPVALUE",chunk, offset,file_ptr);
   	case OP_SET_UPVALUE:
      return constantInstruction("OP_SET_UPVALUE",chunk, offset,file_ptr);
    case OP_CLOSE_UPVALUE:
      return simpleInstruction("OP_CLOSE_UPVALUE", offset,file_ptr);
    case OP_CLASS:
    	return constantInstruction("OP_CLASS",chunk,offset,file_ptr);
    case OP_GET_PROPERTY:
    	return constantInstruction("OP_GET_PROPERTY",chunk,offset,file_ptr);
    case OP_SET_PROPERTY:
    	return constantInstruction("OP_SET_PROPERTY",chunk,offset,file_ptr);
    case OP_INSTANCE_OF:
    	return constantInstruction("OP_SET_PROPERTY",chunk,offset,file_ptr);
    case OP_METHOD:
    	return simpleInstruction("OP_METHOD",offset,file_ptr);
    case OP_INVOKE:
    	return invokeInstruction("OP_INVOKE",chunk,offset,file_ptr);
    case OP_INHERIT:
    	return simpleInstruction("OP_INHERIT",offset,file_ptr);
    case OP_GET_SUPER:
    	return constantInstruction("OP_GET_SUPER",chunk,offset,file_ptr);
    case OP_SUPER_INVOKE:
    	return invokeInstruction("OP_SUPER_INVOKE",chunk,offset,file_ptr);
    case OP_GET_COL:
      return simpleInstruction("OP_GET_COL", offset,file_ptr);
    case OP_GET_ROW:
      return simpleInstruction("OP_GET_ROW", offset,file_ptr);
		default:
			printf("Unknown opcode %d\n",instruction);
		  return offset+1;
	}
}

