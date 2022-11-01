#ifndef clox_chunk_h
#define clox_chunk_h


#include "common.h"
#include "value.h"

typedef enum{
 OP_CONSTANT,
 OP_CLASS,
 OP_METHOD,
 OP_INVOKE,
 OP_SUPER_INVOKE,
 OP_INHERIT,
 OP_GET_SUPER,
 OP_NIL,
 OP_TRUE,
 OP_FALSE,
 OP_POP,
 OP_GET_LOCAL,
 OP_SET_LOCAL,
 OP_GET_GLOBAL,
 OP_DEFINE_GLOBAL,
 OP_SET_GLOBAL,
 OP_GET_UPVALUE,
 OP_SET_UPVALUE,
 OP_DEFINE_CONSTANT,
 OP_GET_CONSTANT,
 OP_INSTANCE_OF,
 OP_EQUAL,
 OP_GREATER,
 OP_LESS,
 OP_ADD,
 OP_SUBTRACT,
 OP_MODULUS,
 OP_MULTIPLY,
 OP_DIVIDE,
 OP_POWER,
 OP_NOT,
 OP_NEGATE,
 OP_PRINT,
 OP_JUMP,
 OP_JUMP_IF_FALSE,
 OP_LOOP,
 OP_CALL,
 OP_CLOSURE,
 OP_RETURN,
 OP_INCREMENT,
 OP_DECREMENT,
 OP_BUILD_LIST,
 OP_INDEX_SUBSCR,
 OP_INDEX_SUBSCR_2,
 OP_STORE_SUBSCR,
 OP_BUILD_MATRIX,
 OP_BUILD_SLICE,
 OP_BUILD_MAP,
 OP_CLOSE_UPVALUE,
 OP_GET_PROPERTY,
 OP_SET_PROPERTY,
 OP_GET_ROW,
 OP_GET_COL,
 OP_SET_ROW,
 OP_SET_COL,
 }OpCode; 

typedef struct{
int count;
int capacity;
uint8_t* code;
int* lines;
ValueArray literals;
}Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk,uint8_t byte,int line);
int addLiteral(Chunk* chunk,Value value);
int addConstant(Chunk* chunk,Value value);

#endif
