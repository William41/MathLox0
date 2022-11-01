#include <stdlib.h>
#include"../include/chunk.h"
#include "../include/memory.h"
#include "../include/vm.h"

void initChunk(Chunk* chunk){
	chunk->count=0;
	chunk->capacity=0;
	chunk->code=NULL;
	chunk->lines=NULL;
	initValueArray(&chunk->literals);
}

void freeChunk(Chunk* chunk){
FREE_ARRAY(uint8_t,chunk->code,chunk->capacity);
FREE_ARRAY(int,chunk->lines,chunk->capacity);
freeValueArray(&chunk->literals);
initChunk(chunk);
}

void writeChunk(Chunk* chunk,uint8_t byte,int line){
	if(chunk->capacity<chunk->count+1){
		int oldCapacity=chunk->capacity;
		chunk->capacity=GROW_CAPACITY(oldCapacity);
		chunk->code=GROW_ARRAY(uint8_t,chunk->code,oldCapacity,chunk->capacity);
		chunk->lines=GROW_ARRAY(int,chunk->lines,oldCapacity,chunk->capacity);
	}
	chunk->code[chunk->count]=byte;
	chunk->lines[chunk->count]=line;
	chunk->count++;
}
int addLiteral(Chunk* chunk,Value value){
	push(value);
	writeValueArray(&chunk->literals,value);
	pop();
	return chunk->literals.count-1;
}
