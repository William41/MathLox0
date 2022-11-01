#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../include/common.h"
#include "../include/scanner.h"

typedef struct{
	const char* start;
	const char* current;
	int line;
}Scanner;

Scanner scanner;

void initScanner(const char* source){
	scanner.start=source;
	scanner.current=source;
	scanner.line=1;
}
static bool isAlpha(char c){
	return( c>='a' && c<='z')||(c>='A' && c<='Z')||(c=='_');
}
static bool isValidHex(char c){
	return (c>='a' && c<='f')||(c>='A' && c<='F');
}
static bool isBinDigit(char c){
	return c=='0' || c=='1';
}
static bool isAtEnd()
{
	return *scanner.current=='\0';
}
static bool isDigit(char c){
	return c>='0' && c<='9'; 
}
static char advance(){
	scanner.current++;
	return scanner.current[-1];
}
static char peek(){
	return *scanner.current;
}
static char peekNext(){
	if(isAtEnd()) return '\0';
	return scanner.current[1];
}
static Token makeToken(TokenType type){
	Token token;
	token.type=type;
	token.start=scanner.start;
	token.length=(int)(scanner.current-scanner.start);
	token.line=scanner.line;
	return token;
}
static Token errorToken(const char* message){
	Token token;
	token.type=TOKEN_ERROR;
	token.start=message;
	token.length=(int)strlen(message);
	token.line=scanner.line;
	return token;
}
static bool match(char expected){
	if(isAtEnd()) return false;
	if(*scanner.current!=expected) return false;
	scanner.current++;
	return true;
}
static void skipWhiteSpace(){
	for(;;){
	char c=peek();
	switch(c){
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			scanner.line++;
			advance();
			break;
		case '/':
			if(peekNext()=='/')
			{
				while(peek()!='\n' && !isAtEnd()) advance();
			}
			else if(peekNext()=='*'){
				while(true){
					advance();
					if(peek()=='\n') scanner.line++;
					if(peek()=='*' && peekNext()=='/') break;
				}
				advance();//consume closing *
				advance();//consume closing /
				}
			else{
			return;
			}
			break;
		default:
			return;
	}
	}
}
static TokenType checkKeyword(int start,int length,const char* rest,TokenType type){
	if(scanner.current-scanner.start==start+length && memcmp(scanner.start+start,rest,length)==0){
		return type;
	}
	return TOKEN_IDENTIFIER;
}
static TokenType identifierType(){
	switch(scanner.start[0])
	{
		case 'a':return checkKeyword(1,2,"nd",TOKEN_AND);
		case 'b':return checkKeyword(1,4,"reak",TOKEN_BREAK);
		case 'c':
			if(scanner.current-scanner.start>1){
				switch(scanner.start[1]){
					case 'l':return checkKeyword(2,3,"ass",TOKEN_CLASS);
					case 'o':return checkKeyword(2,6,"ntinue",TOKEN_CONTINUE);
				}
			}
			break;
		case 'e':return checkKeyword(1, 3, "lse", TOKEN_ELSE);
		case 'f':
			if(scanner.current-scanner.start>1)
			{
				switch(scanner.start[1]){
					case 'a':return checkKeyword(2,3,"lse",TOKEN_FALSE);
					case 'i':return checkKeyword(2,3,"nal",TOKEN_CONST);
					case 'o':return checkKeyword(2,1,"r",TOKEN_FOR);
					case 'u':return checkKeyword(2,1,"n",TOKEN_FUN);
				}
			}
			break;
		case 'i':
			if(scanner.current-scanner.start>1)
			{
				switch(scanner.start[1]){
					case 'f':return TOKEN_IF;
					case 'n':return checkKeyword(1,9,"nstanceof",TOKEN_INST_OF);
				}
			}
			break;
		case 'm':return checkKeyword(1,2,"od",TOKEN_MOD);
		case 'n':return checkKeyword(1,2,"il",TOKEN_NIL);
		case 'o':return checkKeyword(1,1,"r",TOKEN_OR);
		case 'p':return checkKeyword(1,4,"rint",TOKEN_PRINT);
		case 'r':return checkKeyword(1,5,"eturn",TOKEN_RETURN);
		case 's':return checkKeyword(1,4,"uper",TOKEN_SUPER);
		case 't':
			if(scanner.current-scanner.start>1){
				switch(scanner.start[1]){
					case 'h':return checkKeyword(2,2,"is",TOKEN_THIS);
					case 'r':return checkKeyword(2,2,"ue",TOKEN_TRUE);
				}
			}
		case 'u':return checkKeyword(1,4,"sing",TOKEN_USING);
		case 'v':return checkKeyword(1,2,"ar",TOKEN_VAR);
		case 'w':return checkKeyword(1,4,"hile",TOKEN_WHILE);
	}
	return TOKEN_IDENTIFIER;
}
static Token identifier(){
	while(isAlpha(peek()) || isDigit(peek())) advance();
	return makeToken(identifierType());
}
static Token constIdentifier(){
	bool isConst=true;
	while((isAlpha(peek()) || isDigit(peek()))){
		if(isupper(peek()))
	 			advance();
	 	else{
	 		isConst=false;
	 		break;
	 	}
	 }
	 if(isConst)	return makeToken(TOKEN_CONST_IDENTIFIER);
	 else return identifier();
}
static Token string(){
	while(peek()!='"' && !isAtEnd()){
		if(peek()=='\n') scanner.line++;
		advance();
	}
	if(isAtEnd()) return errorToken("Unterminated string.");
	advance();
	return makeToken(TOKEN_STRING);
}
static Token character(){
	if(peekNext() =='\''){
		advance();
	}
	advance();
	return makeToken(TOKEN_CHAR);
}
static Token number(){
	while(isDigit(peek())) advance();
	if(peek()=='.' && isDigit(peekNext())){
		advance();
		while(isDigit(peek())) advance();
	}
	return makeToken(TOKEN_NUMBER);
}
static Token complxNumber(){
	while(isDigit(peek())) advance();
	if(peek()=='e' ||peek()=='E') {
			advance();
			if(peek()=='-') advance();
			while(isDigit(peek())) advance();
	}
	if(peek()=='.' && isDigit(peekNext())){
		advance();
		while(isDigit(peek())) advance();
	}
	if(peek()=='e' ||peek()=='E') {
		advance();
		if(peek()=='-') advance();
	}
	while(isDigit(peek())) advance();
	if(peek()=='.' && isDigit(peekNext())){
		advance();
		while(isDigit(peek())) advance();
	}
	//check if there is a trailing i after the number
	if(peek()=='i') {	
		advance();	//consume the i
		//if so it is an imaginary number
		return makeToken(TOKEN_IMAG_NUMBER);
	}
	else
		return makeToken(TOKEN_NUMBER);	//otherwise just a real number...
}
static Token hexToken(){
	while(isDigit(peek()) || isValidHex(peek())) advance();
	if(peek()=='.') return errorToken("Not a valid hexadecimal number");
	return makeToken(TOKEN_NUMBER_HEX);
}
static Token binToken(){
	while(isDigit(peek())) {
		advance();
		if(peek()>='2' && peek()<='9') return errorToken("Not a valid binary number");
	}
	if(peek()=='.') return complxNumber();
	return makeToken(TOKEN_NUMBER_BIN);
}
static Token octToken(){
	while(isDigit(peek())){ 
		advance();
		if(peek()>='8' && peek()<='9' ) return complxNumber();
	}
	if(peek()=='.'){
		return complxNumber();
		}
	return makeToken(TOKEN_NUMBER_OCT); 
}
static Token numToken(){
	char type=advance();
	switch(type){
		case 'b':
		case 'B':
			return binToken();
		case 'x':
		case 'X':
			return hexToken();
		default:
			break;
	}
}
Token scanToken()
{
	skipWhiteSpace();
	scanner.start=scanner.current;
	if(isAtEnd())
		return makeToken(TOKEN_EOF);
	char c=advance();
	if(isAlpha(c) && isupper(c)) return constIdentifier();
	if(isAlpha(c)) return identifier();
	if(c=='0' && (peek()=='x' || peek()=='X' || peek()=='b' || peek()=='B')) return numToken();
	if(c=='0') return octToken();
	if(isDigit(c)) return complxNumber();
	switch(c){
		case '(':
			return makeToken(TOKEN_LEFT_PAREN);
		case ')':
			return makeToken(TOKEN_RIGHT_PAREN);
		case '{':
			return makeToken(TOKEN_LEFT_BRACE);
		case '}':
			return makeToken(TOKEN_RIGHT_BRACE);
		case '[':
			return makeToken(TOKEN_LEFT_SQ_PAR);
		case ']':
			return makeToken(TOKEN_RIGHT_SQ_PAR);
		case ';':
			return makeToken(TOKEN_SEMICOLON);
		case ',':
			return makeToken(TOKEN_COMMA);
		case ':':
			return makeToken(TOKEN_COLON);
		case '.':
			return makeToken(TOKEN_DOT);
		case '+':
			return makeToken(match('+')?TOKEN_PLUS_PLUS:(match('=')?TOKEN_PLUS_EQUALS:TOKEN_PLUS));
		case '-':
			return makeToken(match('-')?TOKEN_MINUS_MINUS:(match('=')?TOKEN_MINUS_EQUALS:TOKEN_MINUS));
		case '/':
			return makeToken(match('=')?TOKEN_SLASH_EQUALS:TOKEN_SLASH);
		case '*':
			return makeToken(match('=')?TOKEN_STAR_EQUALS:TOKEN_STAR);
		case '^':
			return makeToken(match('=')?TOKEN_POWER_EQUALS:TOKEN_POWER);
		case '%':
			return makeToken(match('=')?TOKEN_PERCENT_EQUALS:TOKEN_PERCENT);
		case '!':
			return makeToken(match('=')?TOKEN_EXCL_EQUAL:TOKEN_EXCL);
		case '=':
			return makeToken(match('=')?TOKEN_EQUAL_EQUAL:TOKEN_EQUAL);
		case '<':
			return makeToken(match('=')?TOKEN_LESS_EQUAL:TOKEN_LESS);
		case '>':
			return makeToken(match('=')?TOKEN_GREATER_EQUAL:TOKEN_GREATER);
		case '\'':return character();
		case '"': return string();
	}
	return errorToken("Unexpected character.");
}

