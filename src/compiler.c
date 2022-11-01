#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include "../include/common.h"
#include "../include/compiler.h"
#include "../include/scanner.h"
#include "../include/value.h"
#include "../include/memory.h"

#ifdef DEBUG_PRINT_CODE
	#include "../include/debug.h"
#endif

extern const char* module_name;

typedef struct{
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
}Parser;

typedef enum{
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_POWER,
	PREC_PREFIX,
	PREC_CALL,
	PREC_SUBSCRIPT,
	PREC_PRIMARY
}Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
}ParseRule;

typedef struct{
	Token name;
	int depth;
	bool isCaptured;
}Local;

typedef struct{
	uint8_t index;
	bool isLocal;
}Upvalue;

typedef struct{
	Token name;
}Constant;

typedef enum{
	TYPE_FUNCTION,
	TYPE_METHOD,
	TYPE_INITIALIZER,
	TYPE_SCRIPT
}FunctionType;

typedef struct Compiler {
	struct Compiler* enclosing;
	ObjFunction* function;
	FunctionType type;
	
	Local locals[UINT8_COUNT];
	
	int localCount;
	Upvalue upvalues[UINT8_COUNT];
	int constCount;
	int scopeDepth;
}Compiler;

typedef struct ClassCompiler{
	struct ClassCompiler* enclosing;
	bool hasSuperClass;
}ClassCompiler;

Parser parser;
Compiler* current=NULL;
ClassCompiler* currentClass=NULL;
bool isPrefix=false;

static Chunk* currentChunk(){
	return &current->function->chunk;
}

static void errorAt(Token* token,const char* message){
	if(parser.panicMode) return;
	parser.panicMode=true;
	fprintf(stderr,"In module:%s ",module_name);
	fprintf(stderr,"[line %d] Error",token->line);
	
	if(token->type==TOKEN_EOF){
		fprintf(stderr," at end");
	}else if(token->type==TOKEN_ERROR){
		//Nothing
	}else{
		fprintf(stderr," at '%.*s'",token->length,token->start);
	}
	
	fprintf(stderr,": %s\n",message);
	parser.hadError=true;
}

static void error(const char* message){
	errorAt(&parser.previous,message);
}

static void errorAtCurrent(const char* message){
	errorAt(&parser.current,message);
}

static void advance(){
	parser.previous=parser.current;
	
	for(;;){
		parser.current=scanToken();
		if(parser.current.type!=TOKEN_ERROR) break;
		
		errorAtCurrent(parser.current.start);
	}
}

static void consume(TokenType type,const char* message){
	if(parser.current.type==type){
		advance();
		return;
	}
	
	errorAtCurrent(message);
}

static bool check(TokenType type){
	return parser.current.type==type;
}

static bool match(TokenType type){
	if(!check(type)) return false;
	advance();
	return true;
}

static void emitByte(uint8_t byte){
	writeChunk(currentChunk(),byte,parser.previous.line);
}

static void emitBytes(uint8_t byte1,uint8_t byte2){
	emitByte(byte1);
	emitByte(byte2);
}

static void emitLoop(int loopStart){
	emitByte(OP_LOOP);
	int offset=currentChunk()->count-loopStart+2;
	if(offset >UINT16_MAX) error("Loop body too large");
	
	emitByte((offset >>8) & 0xff);
	emitByte(offset & 0xff);
}

/*static void emitBreak(int breakPoint){
	emitByte(OP_JUMP);
	int offset=currentChunk()->count-breakPoint+2;
	if(offset >UINT16_MAX) error("Loop body too large");
	
	emitByte((offset >>8) & 0xff);
	emitByte(offset & 0xff);
}*/


static int emitJump(uint8_t instruction){
	emitByte(instruction);
	emitByte(0xff);
	emitByte(0xff);
	return currentChunk()->count-2;
}

static void emitReturn(){
	if(current->type==TYPE_INITIALIZER){
		emitBytes(OP_GET_LOCAL,0);
	}else{
		emitByte(OP_NIL);
	}
	emitByte(OP_RETURN);
}

static uint8_t makeLiteral(Value value){
	int constant=addLiteral(currentChunk(),value);
	if(constant>UINT8_MAX){
		error("Too many literals in one chunk.");
		return 0;
	}
	return (uint8_t)constant;
}


static void emitLiteral(Value value){
	emitBytes(OP_CONSTANT,makeLiteral(value));
}

static void patchJump(int offset){
	int jump=currentChunk()->count-offset-2;
	if(jump>UINT16_MAX){
		error("Too much code to jump over...");
	}
	
	currentChunk()->code[offset]=(jump>>8) & 0xff;
	currentChunk()->code[offset+1]=jump & 0xff;
	
}

static void initCompiler(Compiler* compiler,FunctionType type){
	compiler->enclosing=current;
	compiler->function=NULL;
	compiler->type=type;
	compiler->localCount=0;
	compiler->constCount=0;
	compiler->scopeDepth=0;
	compiler->function=newFunction();
	current=compiler;
	if(type!=TYPE_SCRIPT){
		current->function->name=copyString(parser.previous.start,parser.previous.length);
	}
	
		//claim stack slot 0 for the vm use... 
	Local* local=&current->locals[current->localCount++];
	local->depth=0;
	local->isCaptured=false;
	if(type!=TYPE_FUNCTION){
		local->name.start="this";
		local->name.length=4;
	}else{
		local->name.start="";
		local->name.length=0;
	}
}

static ObjFunction* endCompiler(){
	emitReturn();
	ObjFunction* function=current->function;
	#ifdef DEBUG_PRINT_CODE
		if(!parser.hadError){
			disassembleChunk(currentChunk(),function->name != NULL ?function->name->chars:"<script>");
		}
	#endif
	current=current->enclosing;
	return function;
}

static void beginScope(){
	current->scopeDepth++;
}

static void endScope(){
	current->scopeDepth--;
	while(current->localCount>0 && current->locals[current->localCount-1].depth>current->scopeDepth){
		if(current->locals[current->localCount-1].isCaptured){
			emitByte(OP_CLOSE_UPVALUE);
		}else{
			emitByte(OP_POP);
		}
		current->localCount--;
	}
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary(bool canAssign){
	TokenType operatorType=parser.previous.type;
	ParseRule* rule=getRule(operatorType);
	if(operatorType==TOKEN_POWER) parsePrecedence((Precedence)(rule->precedence));
	else 
		parsePrecedence((Precedence)(rule->precedence+1));
	switch(operatorType){
		case TOKEN_EXCL_EQUAL:		emitBytes(OP_EQUAL,OP_NOT);break;
		case TOKEN_EQUAL_EQUAL:		emitByte(OP_EQUAL);break;
		case TOKEN_GREATER:				emitByte(OP_GREATER);break;
		case TOKEN_GREATER_EQUAL:	emitBytes(OP_LESS,OP_NOT);break;
		case TOKEN_LESS:					emitByte(OP_LESS);break;
		case TOKEN_LESS_EQUAL:		emitBytes(OP_GREATER,OP_NOT);break;
		case TOKEN_MOD:						emitByte(OP_MODULUS);break;
		case TOKEN_PLUS:					emitByte(OP_ADD);break;
		case TOKEN_MINUS:					emitByte(OP_SUBTRACT);break;
		case TOKEN_STAR:					emitByte(OP_MULTIPLY);break;
		case TOKEN_SLASH:					emitByte(OP_DIVIDE);break;
		case TOKEN_POWER:					emitByte(OP_POWER);break;
		case TOKEN_PERCENT:				emitByte(OP_MODULUS);break;
		case TOKEN_INST_OF:				emitByte(OP_INSTANCE_OF);break;
		default:
			return;
	}
}

static uint8_t argumentList(){
	uint8_t argCount=0;
	if(!check(TOKEN_RIGHT_PAREN)){
		do{
			expression();
			if(argCount==255){
				error("Cannot have more than 255 arguments");
			}
			argCount++;
		}while(match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN,"Expected ')' after arguments");
	return argCount;
}

static void call(bool canAssign){
	uint8_t argCount=argumentList();
	emitBytes(OP_CALL,argCount);
}

static void literal(bool canAssign)
{
	switch(parser.previous.type){
		case TOKEN_FALSE:	emitByte(OP_FALSE);break;
		case TOKEN_TRUE:	emitByte(OP_TRUE);break;
		case TOKEN_NIL:		emitByte(OP_NIL);break;
		default:					return;
	}
}

static void grouping(bool canAssign){
	expression();
	consume(TOKEN_RIGHT_PAREN,"Expected ')' after expression.");
}

static void number(bool canAssign){
	double value=strtod(parser.previous.start,NULL);
	emitLiteral(NUMBER_VAL(value));
}

static void hexnumber(bool canAssign){
	long value=strtol(parser.previous.start,NULL,16);
	emitLiteral(NUMBER_VAL(value));
}

static void binnumber(bool canAssign){
	int value=strtol(parser.previous.start+2,NULL,2);
	emitLiteral(NUMBER_VAL(value));
}

static void octnumber(bool canAssign){
	long value=strtol(parser.previous.start,NULL,8);
	emitLiteral(NUMBER_VAL(value));
}

static void character(bool canAssign){
	char val=parser.previous.start[1];
	emitLiteral(CHAR_VAL(val));
}

static void or_ (bool canAssign)
{
	int elseJump=emitJump(OP_JUMP_IF_FALSE);
	int endJump=emitJump(OP_JUMP);
	
	patchJump(elseJump);
	emitByte(OP_POP);
	
	parsePrecedence(PREC_OR);
	patchJump(endJump);
}

static void and_ (bool canAssign){
	int endJump=emitJump(OP_JUMP_IF_FALSE);
	
	emitByte(OP_POP);
	parsePrecedence(PREC_AND);
	
	patchJump(endJump);
}

static void imaginary(bool canAssign){
	//function to handle parsing of imaginary numbers
	emitLiteral(OBJ_VAL(parseImaginary(parser.previous.start)));
}

static void string(bool canAssign){
	emitLiteral(OBJ_VAL(copyString(parser.previous.start+1,parser.previous.length-2)));
}

static uint8_t identifierLiteral(Token* name){
	return makeLiteral(OBJ_VAL(copyString(name->start,name->length)));
}

static bool identifiersEqual(Token* a,Token* b){
	if(a->length!=b->length) return false;
	return memcmp(a->start,b->start,a->length)==0;
}

static int resolveLocal(Compiler* compiler,Token* name){
	for(int i=compiler->localCount-1;i>=0;i--){
		Local* local=&compiler->locals[i];
		if(identifiersEqual(name,&local->name)){
			if(local->depth==-1){
				error("Cannot read local variable in its own initializer..");
			}
			return i;
		}
	}
	return -1;
}

static int addUpvalue(Compiler* compiler,uint8_t index,bool isLocal){
	int upvalueCount=compiler->function->upvalueCount;
	for(int i=0;i<upvalueCount;i++){
		Upvalue* upvalue=&compiler->upvalues[i];
		if(upvalue->index==index && upvalue->isLocal==isLocal){
			return i;
		}
	}
	if(upvalueCount==UINT8_COUNT){
		error("Too many closure variables in function.");
		return 0;
	}
	compiler->upvalues[upvalueCount].isLocal=isLocal;
	compiler->upvalues[upvalueCount].index=index;
	return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler,Token* name){
	if(compiler->enclosing==NULL) return -1;
	
	 int local=resolveLocal(compiler->enclosing,name);
	 
	 if(local!=-1){
	 	compiler->enclosing->locals[local].isCaptured=true;
	 	return addUpvalue(compiler,(uint8_t)local,true);
	 }
	 int upvalue=resolveUpvalue(compiler->enclosing,name);
	 if(upvalue!=-1){
	 	return addUpvalue(compiler,(uint8_t)upvalue,false);
	 }
	 return -1;
}

static void addLocal(Token name){
	if(current->localCount==UINT8_COUNT){
		error("Too many local variables in a function..");
		return;
	}
	Local* local=&current->locals[current->localCount++];
	local->name=name;
	local->depth=-1;
	local->isCaptured=false;
}

static void declareVariable(){
	if(current->scopeDepth==0)	return;
	
	Token* name=&parser.previous;
	for(int i=current->localCount-1;i>=0;i--){
		Local* local=&current->locals[i];
		if(local->depth != -1 && local->depth<current->scopeDepth){
			break;
		}
		
		if(identifiersEqual(name,&local->name)){
			error("Already a variable with this name in this scope...");
		}
	}
	addLocal(*name);
}

static void dot(bool canAssign){
	consume(TOKEN_IDENTIFIER,"Expected property name after '.'.");
	uint8_t name=identifierLiteral(&parser.previous);
	if(canAssign && match(TOKEN_EQUAL)){
		expression();
		emitBytes(OP_SET_PROPERTY,name);
	}else if(match(TOKEN_LEFT_PAREN)){
		uint8_t argCount=argumentList();
		emitBytes(OP_INVOKE,name);
		emitByte(argCount);
	}else{
		emitBytes(OP_GET_PROPERTY,name);
	}
}

static void namedVariable(Token name,bool canAssign){
//thanks to Caleb schoepp for the below modification:
	#define SHORT_HAND_ASSIGNER(op)\
		do{\
				emitBytes(getOp,(uint8_t)arg);\
				expression();\
				emitByte(op);\
				emitBytes(setOp,(uint8_t)arg);\
			}while(false)
	uint8_t getOp,setOp;
	int arg=resolveLocal(current,&name);
	if(arg!=-1){
		getOp=OP_GET_LOCAL;
		setOp=OP_SET_LOCAL;
	}else if((arg=resolveUpvalue(current,&name))!=-1){
		getOp=OP_GET_UPVALUE;
		setOp=OP_SET_UPVALUE;
	}else{
		arg=identifierLiteral(&name);
		getOp=OP_GET_GLOBAL;
		setOp=OP_SET_GLOBAL;
	}
	if(canAssign && match(TOKEN_EQUAL)){
			expression();
			emitBytes(setOp,(uint8_t)arg);
	}else if(canAssign && match(TOKEN_PLUS_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_ADD);
	}else if(canAssign && match(TOKEN_MINUS_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_SUBTRACT);
	}else if(canAssign && match(TOKEN_STAR_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_MULTIPLY);
	}else if(canAssign && match(TOKEN_SLASH_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_DIVIDE);
	}else if(canAssign && match(TOKEN_POWER_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_POWER);
	}else if(canAssign && match(TOKEN_PERCENT_EQUALS)){
		SHORT_HAND_ASSIGNER(OP_MODULUS);
	}else{
			emitBytes(getOp,(uint8_t)arg);
	}
	#undef SHORT_HAND_ASSIGNER
}

static void variable(bool canAssign){
	namedVariable(parser.previous,canAssign);
}

static Token syntheticToken(const char* text){
	Token token;
	token.start=text;
	token.length=(int)strlen(text);
	return token;
}

static void super_ (bool canAssign){
	if(currentClass==NULL){
		error("Cannot use 'super' outside of a class.");
	}else if(!currentClass->hasSuperClass){
		error("Cannot use 'super' in a class with no superclass.");
	}
	consume(TOKEN_DOT,"Expected . after 'super'.");
	consume(TOKEN_IDENTIFIER,"Expected superclass method name.");
	uint8_t name=identifierLiteral(&parser.previous);
	
	namedVariable(syntheticToken("this"),false);
	if(match(TOKEN_LEFT_PAREN)){
		uint8_t argCount=argumentList();
		namedVariable(syntheticToken("super"),false);
		emitBytes(OP_SUPER_INVOKE,name);
		emitByte(argCount);
	}else{
		namedVariable(syntheticToken("super"),false);
		emitBytes(OP_GET_SUPER,name);
	}
}

static void this_(bool canAssign){
	if(currentClass==NULL){
		error("Cannot use 'this' outside a class.");
		return;
	}
	variable(false);
}

static void constant(bool canAssign){
	namedVariable(parser.previous,false);
}

static void unary(bool canAssign){
	#define HANDLE_PREFIX(op)\
		do{\
			arg=resolveLocal(current,&name);\
			if(arg!=-1){setOp=OP_SET_LOCAL;}\
			else{	\
					arg=identifierLiteral(&name);\
					setOp=OP_SET_GLOBAL;\
				}\
				emitByte(op);\
				emitBytes(setOp,(uint8_t)arg);\
		}while(false)
	int arg;
	uint8_t setOp;
	TokenType operatorType=parser.previous.type;
	switch(operatorType){
		case TOKEN_EXCL:{
				parsePrecedence(PREC_UNARY);
				emitByte(OP_NOT);
				break;
			}
		case TOKEN_MINUS:{
				parsePrecedence(PREC_UNARY);
				emitByte(OP_NEGATE);
				break;
			}
		case TOKEN_PLUS:
			break;
		default:return;
	}
	#undef HANDLE_PREFIX
}

static void map(bool canAssign){
	int itemCount=1;
	consume(TOKEN_COLON,"Expected ':' between key and value pair");
	parsePrecedence(PREC_OR);
	match(TOKEN_COMMA);
	if(!check(TOKEN_RIGHT_BRACE)){
		do{
			if(check(TOKEN_RIGHT_BRACE)) break;
			parsePrecedence(PREC_OR);
			consume(TOKEN_COLON,"Expected ':' between key and value pair");
			parsePrecedence(PREC_OR);
			if(itemCount>256) error("Cannot have more than 256 values in map literal");
			itemCount++;
		}while(match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_BRACE, "Expect '}' after map elements.");
	emitByte(OP_BUILD_MAP);
	emitByte((uint8_t)itemCount);
	return;
}

static void list(bool canAssign){
	int itemCount=0;
	if(!check(TOKEN_RIGHT_BRACE)){
		do{
			if(check(TOKEN_RIGHT_BRACE))	break;
			parsePrecedence(PREC_OR);
			if(check(TOKEN_COLON)){
				map(canAssign);
				return;
			}else{
				if(itemCount==UINT8_COUNT){
					error("Cannot have more than 256 items in a list literal");
				}
				itemCount++;
			}	
		}while(match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_BRACE,"Expected '}' after list literal");
	emitByte(OP_BUILD_LIST);
	emitByte(itemCount);
	return;
}

static void matrix(bool canAssign){
	int rowCount=0;
	int colCount=0;
	if(!check(TOKEN_RIGHT_SQ_PAR)){
		do{
			colCount=0;
			if(check(TOKEN_RIGHT_SQ_PAR)) break;
			do{
				parsePrecedence(PREC_OR);
				if(colCount==(UINT8_COUNT)>>5){
				error("Cannot have more than 32 columns in a matrix literal");
				}
				colCount++;
			}while(match(TOKEN_COMMA));
			if(rowCount==(UINT8_COUNT)>>5){
				error("Cannot have more than 32  rows in a matrix literal");
				}
			rowCount++;
		}while(match(TOKEN_SEMICOLON));
	}
	consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after matrix literal.");
	emitByte(OP_BUILD_MATRIX);
	emitByte((uint8_t)rowCount);
	emitByte((uint8_t)colCount);
}

static void matrixSubscript(bool canAssign){
		if(match(TOKEN_STAR)){
			consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after index" );
			emitByte(OP_GET_ROW);
		return;
		}else{
			parsePrecedence(PREC_OR);
		}
		consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after index" );
		if(canAssign && match(TOKEN_EQUAL)){
			expression();
			emitByte(OP_STORE_SUBSCR);
		}else{
			emitByte(OP_INDEX_SUBSCR);
		}
		return;
}

static void sliceSubscript(bool canAssign){
	//function compiles of the form a[i:j:k]
	//i=start index j=end index k=step(optional) default 1
	parsePrecedence(PREC_OR);//for end index
	if(match(TOKEN_COLON)) parsePrecedence(PREC_OR);
	consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after slice literal");
		if(canAssign && match(TOKEN_EQUAL)){
			errorAt(&parser.current,"Cannot assign slice to value");
		}else{
			emitByte(OP_BUILD_SLICE);
	}
}

static void subscript(bool canAssign){
	#define ARR_SHORT_HAND_ASSIGNER(op)\
		do{\
			emitByte(OP_INDEX_SUBSCR_2);\
			expression();\
			emitByte(op);\
			emitByte(OP_STORE_SUBSCR);\
		}while(false)
	if(match(TOKEN_STAR)){
		consume(TOKEN_COMMA,"Expected ',' after '*'.");
		parsePrecedence(PREC_OR);
		consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after index" );
		emitByte(OP_GET_COL);
		return;
	}else{
		parsePrecedence(PREC_OR);
	}
	if(match(TOKEN_COMMA)){
		matrixSubscript(canAssign);
	}else if(match(TOKEN_COLON)){
		sliceSubscript(canAssign);
	}else{
		consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after index" );
		if(canAssign && match(TOKEN_EQUAL)){
			expression();
			emitByte(OP_STORE_SUBSCR);
		}else if(canAssign && match(TOKEN_PLUS_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_ADD);
		}else if(canAssign && match(TOKEN_MINUS_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_SUBTRACT);
		}else if(canAssign && match(TOKEN_STAR_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_MULTIPLY);
		}else if(canAssign && match(TOKEN_SLASH_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_DIVIDE);
		}else if(canAssign && match(TOKEN_POWER_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_POWER);
		}else if(canAssign && match(TOKEN_PERCENT_EQUALS)){
			ARR_SHORT_HAND_ASSIGNER(OP_MODULUS);
		}else{
			emitByte(OP_INDEX_SUBSCR);
		}
		return;
	}
	#undef ARR_SHORT_HAND_ASSIGNER
}

static void preIncDec(bool canAssign){
	#define GET_SET_OPCODES(arg)\
		do{\
			if(arg!=-1){\
				getOp=OP_GET_LOCAL;\
				setOp=OP_SET_LOCAL;\
			}else{\
				arg=identifierLiteral(&ident);\
				getOp=OP_GET_GLOBAL;\
				setOp=OP_SET_GLOBAL;\
			}\
		}while(false)
	TokenType operatorType=parser.previous.type;
	consume(TOKEN_IDENTIFIER,"Expected variable name after prefix.");
	Token ident=parser.previous;
	uint8_t getOp,setOp;
	int arg=resolveLocal(current,&ident);
		GET_SET_OPCODES(arg);
	if(match(TOKEN_LEFT_SQ_PAR)){
	emitBytes(getOp,(uint8_t)arg);
	parsePrecedence(PREC_OR);
		emitByte(OP_INDEX_SUBSCR_2);
		
		consume(TOKEN_RIGHT_SQ_PAR,"Expected ']' after array index");
		emitByte(OP_INCREMENT);
			emitByte(OP_STORE_SUBSCR);
	}else{
	switch(operatorType){
		case TOKEN_PLUS_PLUS:{
			emitBytes(getOp,(uint8_t)arg);
			emitByte(OP_INCREMENT);
			emitBytes(setOp,(uint8_t)arg);
			break;
		}
		case TOKEN_MINUS_MINUS:{
				emitBytes(getOp,(uint8_t)arg);
			emitByte(OP_DECREMENT);
			emitBytes(setOp,(uint8_t)arg);
			break;		
		}
	}
	}
	return;
	#undef GET_SET_OPCODES
}

ParseRule rules[]={
[TOKEN_LEFT_PAREN]=								{grouping,	call,				PREC_CALL},
[TOKEN_RIGHT_PAREN]=							{NULL,			NULL,				PREC_NONE},
[TOKEN_LEFT_BRACE]= 							{list,			NULL,				PREC_NONE},
[TOKEN_RIGHT_BRACE]=							{NULL,			NULL,				PREC_NONE},
[TOKEN_COMMA]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_DOT]=											{NULL,			dot,				PREC_CALL},
[TOKEN_MINUS]=										{unary,			binary,			PREC_TERM},
[TOKEN_PLUS]=											{unary,			binary,			PREC_TERM},
[TOKEN_SEMICOLON]=								{NULL,			NULL,				PREC_NONE},
[TOKEN_MOD]=											{NULL,			binary,			PREC_FACTOR},
[TOKEN_SLASH]=										{NULL,			binary,			PREC_FACTOR},
[TOKEN_STAR]=											{NULL,			binary,			PREC_FACTOR},
[TOKEN_PERCENT]=									{NULL,			binary,			PREC_FACTOR},
[TOKEN_POWER]=										{NULL,			binary,			PREC_POWER},
[TOKEN_EXCL]=											{unary,			NULL,				PREC_NONE},
[TOKEN_EXCL_EQUAL]=								{NULL,			binary,			PREC_EQUALITY},
[TOKEN_EQUAL]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_EQUAL_EQUAL]=							{NULL,			binary,			PREC_EQUALITY},
[TOKEN_GREATER]=									{NULL,			binary,			PREC_COMPARISON},
[TOKEN_GREATER_EQUAL]=						{NULL,			binary,			PREC_COMPARISON},
[TOKEN_LESS]=											{NULL,			binary,			PREC_COMPARISON},
[TOKEN_LESS_EQUAL]=								{NULL,			binary,			PREC_COMPARISON},
[TOKEN_INST_OF]=									{NULL,			binary,			PREC_COMPARISON},//refered to javascript's instance of precedence
[TOKEN_IDENTIFIER]=								{variable,	NULL,				PREC_NONE},
[TOKEN_CONST_IDENTIFIER]=					{constant,	NULL,				PREC_NONE},
[TOKEN_STRING]=										{string,		NULL,				PREC_NONE},
[TOKEN_CHAR]=											{character,	NULL,				PREC_NONE},
[TOKEN_NUMBER]=										{number,		NULL,				PREC_NONE},
[TOKEN_IMAG_NUMBER]=							{imaginary,	NULL,				PREC_NONE},
[TOKEN_AND]=											{NULL,			and_,				PREC_AND},
[TOKEN_CLASS]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_ELSE]=											{NULL,      NULL,				PREC_NONE},
[TOKEN_FALSE]=										{literal,   NULL,				PREC_NONE},
[TOKEN_FOR]=											{NULL,      NULL,				PREC_NONE},
[TOKEN_FUN]=											{NULL,      NULL,				PREC_NONE},
[TOKEN_IF]=												{NULL,      NULL,				PREC_NONE},
[TOKEN_NIL]=											{literal,		NULL,				PREC_NONE},
[TOKEN_OR]=												{NULL,			or_,				PREC_OR},
[TOKEN_PRINT]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_RETURN]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_SUPER]=										{super_,			NULL,				PREC_NONE},
[TOKEN_THIS]=											{this_,			NULL,				PREC_NONE},
[TOKEN_TRUE]=											{literal,		NULL,				PREC_NONE},
[TOKEN_VAR]=											{NULL,			NULL,				PREC_NONE},
[TOKEN_WHILE]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_ERROR]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_CONST]=										{NULL,			NULL,				PREC_NONE},
[TOKEN_PLUS_PLUS]=								{preIncDec,	NULL,				PREC_UNARY},
[TOKEN_MINUS_MINUS]=							{preIncDec,	NULL,				PREC_UNARY},
[TOKEN_LEFT_SQ_PAR]=							{matrix,		subscript,	PREC_SUBSCRIPT},
[TOKEN_RIGHT_SQ_PAR]=							{NULL,			NULL,				PREC_NONE},
[TOKEN_NUMBER_HEX]=								{hexnumber,	NULL,				PREC_NONE},
[TOKEN_NUMBER_BIN]=								{binnumber,	NULL,				PREC_NONE},
[TOKEN_NUMBER_OCT]=								{octnumber,	NULL,				PREC_NONE},
[TOKEN_EOF]=											{NULL,			NULL,				PREC_NONE},
};


static void parsePrecedence(Precedence precedence){
	advance();
	ParseFn prefixRule=getRule(parser.previous.type)->prefix;
	if(prefixRule==NULL){
		
		error("Expected expression.");
		return;
	}
	
	bool canAssign=precedence<=PREC_ASSIGNMENT;
	prefixRule(canAssign);
	
	while(precedence<=getRule(parser.current.type)->precedence){
		advance();
		ParseFn infixRule=getRule(parser.previous.type)->infix;
		infixRule(canAssign);
	}
	
	if(canAssign && match(TOKEN_EQUAL)){
		error("Invalid Assignment target.");
	}
}

static uint8_t parseVariable(const char* errorMessage){
	consume(TOKEN_IDENTIFIER,errorMessage);
	declareVariable();
	if(current->scopeDepth>0)	return 0;
	return identifierLiteral(&parser.previous);
}

static uint8_t parseConstant(const char* errorMessage){
	consume(TOKEN_CONST_IDENTIFIER,"Expected uppercase constant name.");
	declareVariable();
	if(current->scopeDepth>0)	return 0;
	return identifierLiteral(&parser.previous);
}

static void markInitialized(){
	if(current->scopeDepth==0) return;
	current->locals[current->localCount-1].depth=current->scopeDepth;
}

static void defineVariable(uint8_t global){
	if(current->scopeDepth>0){
		markInitialized();
		return;
	}
	emitBytes(OP_DEFINE_GLOBAL,global);
}

static void defineConstant(uint8_t constant){
	if(current->scopeDepth>0){
		markInitialized();
		return;
	}
	emitBytes(OP_DEFINE_GLOBAL,constant);
}

static ParseRule* getRule(TokenType type){
	return &rules[type];
}

static void expression(){
	parsePrecedence(PREC_ASSIGNMENT);
}

static void block(){
	while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
		declaration();
	}
	
	consume(TOKEN_RIGHT_BRACE,"Expected '}' after block.");
}

static void function(FunctionType type){
	Compiler compiler;
	initCompiler(&compiler,type);
	beginScope();
	
	consume(TOKEN_LEFT_PAREN,"Expect '(' after function name");
	if(!check(TOKEN_RIGHT_PAREN)){
		do{
			current->function->arity++;
			if(current->function->arity>255){
				errorAtCurrent("Cannot have more than 255 parameters");
			}
			uint8_t constant=parseVariable("Expected parameter name");
			defineVariable(constant);
		}while(match(TOKEN_COMMA));
	}
	
	
	consume(TOKEN_RIGHT_PAREN,"Expect ')' after parameters");
	consume(TOKEN_LEFT_BRACE,"Expect '{' before function body");
	
	block();
	
	ObjFunction* function=endCompiler();
	emitBytes(OP_CLOSURE,makeLiteral(OBJ_VAL(function)));
	for(int i=0;i<function->upvalueCount;i++){
		emitByte(compiler.upvalues[i].isLocal?1:0);
		emitByte(compiler.upvalues[i].index);
	}
}

static void operator_function(FunctionType type){
	/*Compiler compiler;
	initCompiler(&compiler,type);
	beginScope();
	
	consume(TOKEN_LEFT_BRACE,"Expect '{' before operator body");
	
	block();
	
	ObjFunction* function=endCompiler();
	emitBytes(OP_OVL_ADD,makeLiteral(OBJ_VAL(function)));
	for(int i=0;i<function->upvalueCount;i++){
		emitByte(compiler.upvalues[i].isLocal?1:0);
		emitByte(compiler.upvalues[i].index);
	}*/
}

static void operatorOverload(){
	/*match(TOKEN_OPERATOR);
	if(check(TOKEN_PLUS)){
		consume(TOKEN_PLUS,"Expected operator after keyword.");
		uint8_t constant=identifierLiteral(&parser.previous);
		FunctionType type=TYPE_FUNCTION;
		operator_function(type);
		//emitBytes(OP_OVL_ADD,constant);
	}*/
}

static void method(){
	if(check(TOKEN_OPERATOR)){
		//operatorOverload();
	}else{
		consume(TOKEN_IDENTIFIER,"Expected method name.");
		uint8_t constant=identifierLiteral(&parser.previous);
	
		FunctionType type=TYPE_METHOD;
		if(parser.previous.length==4 && memcmp(parser.previous.start,"init",4)==0){
			type=TYPE_INITIALIZER;
		}
		function(type);
		emitBytes(OP_METHOD,constant);
	}
}

static void classDeclaration(){
	consume(TOKEN_IDENTIFIER,"Expected class name after keyword 'class'");
	Token className=parser.previous;
	uint8_t nameConstant=identifierLiteral(&parser.previous);
	declareVariable();
	
	emitBytes(OP_CLASS,nameConstant);
	defineVariable(nameConstant);
	
	ClassCompiler classCompiler;
	classCompiler.hasSuperClass=false;
	classCompiler.enclosing=currentClass;
	currentClass=&classCompiler;
	
	if(match(TOKEN_LESS)){
		consume(TOKEN_IDENTIFIER,"Expected super class name after <.");
		variable(false);
		if(identifiersEqual(&className,&parser.previous)){
			error("A class cannot inherit form itself.");
		}
		
		beginScope();
		addLocal(syntheticToken("super"));
		defineVariable(0);	
				
		namedVariable(className,false);
		emitByte(OP_INHERIT);
		classCompiler.hasSuperClass=true;
	}
	
	namedVariable(className,false);
	consume(TOKEN_LEFT_BRACE,"Expected '{' before class body.");
	while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
			method();
	}
	consume(TOKEN_RIGHT_BRACE,"Expected '}' after class body.");
	emitByte(OP_POP);
	
	if(classCompiler.hasSuperClass){
		endScope();
	}
	
	currentClass=currentClass->enclosing;
}

static void funDeclaration(){
	uint8_t global=parseVariable("Expected function name..");
	markInitialized();
	function(TYPE_FUNCTION);
	defineVariable(global);
}

static void varDeclaration(){
	uint8_t global=parseVariable("Compile Error.Expected variable name");
	if(match(TOKEN_EQUAL)){
		expression();
	}else{
		emitByte(OP_NIL);
	}
	consume(TOKEN_SEMICOLON,"Expected ';' after variable declaration");
	defineVariable(global);
}

static void constDeclaration(){
	uint8_t constant=parseConstant("Expected upper case constant name");
	if(match(TOKEN_EQUAL)){
		expression();
		}else{
			emitByte(OP_NIL);
	}
	consume(TOKEN_SEMICOLON,"Expected ';' after constant declaration");
	defineConstant(constant);
}
static void expressionStatement(){
	expression();
	consume(TOKEN_SEMICOLON,"Expected ';' after expression");
	emitByte(OP_POP);
}

int loop=-1;
int jumpBreak=0;
int jumpContinue=0;
bool isInsideLoop=false;
bool isBreak=false;
int innermostLoopStart = -1;
int innermostLoopScopeDepth = 0;
static void forStatement(){
	beginScope();
	consume(TOKEN_LEFT_PAREN,"Expected '(' after 'for'");
	if(match(TOKEN_SEMICOLON)){
	//no initializer
	}else if(match(TOKEN_VAR)){
		varDeclaration();
	}else{
		expressionStatement();
	}
	//consume(TOKEN_SEMICOLON,"Expected ';'.");
	int loopStart=currentChunk()->count;
	int exitJump=-1;
	if(!match(TOKEN_SEMICOLON)){
		expression();
		consume(TOKEN_SEMICOLON,"Expected ';' after loop condition.");
		
		//Jump out of the loop if condition is false
		exitJump=emitJump(OP_JUMP_IF_FALSE);
		emitByte(OP_POP);
	}
	if(!match(TOKEN_RIGHT_PAREN)){
		int bodyJump=emitJump(OP_JUMP);
		int incrementStart=currentChunk()->count;
		expression();
		emitByte(OP_POP);
		consume(TOKEN_RIGHT_PAREN,"Expected ')' after for clauses.");
		
		emitLoop(loopStart);
		loopStart=incrementStart;
		patchJump(bodyJump);
	}
	loop++;
	
	statement();
	emitLoop(loopStart);
	
	if(exitJump!=-1){
		patchJump(exitJump);
		emitByte(OP_POP);
	}
	
	if(isBreak){
		patchJump(jumpBreak);
	}
	isBreak=false;
	endScope();
	loop--;
}

static void ifStatement(){
	consume(TOKEN_LEFT_PAREN,"Expected '(' after if.");
	expression();
	consume(TOKEN_RIGHT_PAREN,"Expected ')' after condition");
	
	int thenJump=emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);
	statement();
	
	int elseJump=emitJump(OP_JUMP);	//jump over the else block
	patchJump(thenJump);
	emitByte(OP_POP);
	
	if(match(TOKEN_ELSE)) statement();
	patchJump(elseJump);
	
}

static void breakStatement(){
	if(loop==-1)
		error("Break cannot be used outside a loop body");
	consume(TOKEN_SEMICOLON,"Expected ';' at end of statement");
	isBreak=true;
	
	jumpBreak=emitJump(OP_JUMP);
}


static void printStatement(){
	expression();
	consume(TOKEN_SEMICOLON,"Expected ';' at end of print statement");
	emitByte(OP_PRINT);
}

static void returnStatement(){
	if(current->type==TYPE_SCRIPT){
		error("Cannot return from top level code");
	}
	if(match(TOKEN_SEMICOLON)){
		emitReturn();
	}else{
		if(current->type==TYPE_INITIALIZER){
			error("Cannot return a valur from an initializer.");
		}
		expression();
		consume(TOKEN_SEMICOLON,"Expected ';' after return value");
		emitByte(OP_RETURN);
	}
}

static void whileStatement(){
	int loopStart=currentChunk()->count;
	consume(TOKEN_LEFT_PAREN,"Expected '(' after while.");
	expression();
	consume(TOKEN_RIGHT_PAREN,"Expected ')' after condition");
	loop++;
	int exitJump=emitJump(OP_JUMP_IF_FALSE);
	emitByte(OP_POP);

	statement();
	emitLoop(loopStart);
	
	patchJump(exitJump);
	emitByte(OP_POP);
	if(isBreak){
		patchJump(jumpBreak);
	}
	isBreak=false;
	isInsideLoop=false;
	loop--;
}

static void synchronize(){
	parser.panicMode=false;
	
	while(parser.current.type!=TOKEN_EOF){
		if(parser.previous.type==TOKEN_SEMICOLON) return;
		switch(parser.current.type){
			case TOKEN_CLASS:
			case TOKEN_FUN:
			case TOKEN_VAR:
			case TOKEN_FOR:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
				return;
			default:
				;//do nothing
		}
		
		advance();
	}
}

static void declaration(){
	if(match(TOKEN_CLASS)){
		classDeclaration();
	}else if(match(TOKEN_FUN)){
		funDeclaration();
	}else if(match(TOKEN_CONST)){
		constDeclaration();
	}else if(match(TOKEN_VAR)){
		varDeclaration();
	}else{
		statement();
	}
	if(parser.panicMode) synchronize();
}

static void statement()
{
	if(match(TOKEN_PRINT)){
		printStatement();
	}else if(match(TOKEN_RETURN)){
		returnStatement();
	}else if(match(TOKEN_FOR)){
		forStatement();
	}else if (match(TOKEN_WHILE)){
		whileStatement();
	}else if(match(TOKEN_BREAK)){
		breakStatement();
	}else if(match(TOKEN_CONTINUE)){
		//continueStatement();
	}else if(match(TOKEN_IF)){
		ifStatement();
	}else if(match(TOKEN_LEFT_BRACE)){
		beginScope();
		block();
		endScope();
	}else{
		expressionStatement();
	}
}

ObjFunction* compile(const char* source){
	initScanner(source);
	Compiler compiler;
	initCompiler(&compiler,TYPE_SCRIPT);
	parser.hadError=false;
	parser.panicMode=false;
	advance();
	while(!match(TOKEN_EOF)){
		declaration();
	}
	ObjFunction* function=endCompiler();
	return parser.hadError?NULL:function;
}

void markCompilerRoots(){
	Compiler* compiler=current;
	while(compiler!=NULL){
		markObject((Obj*)compiler->function);
		compiler=compiler->enclosing;
	}
}
