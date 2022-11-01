#include<stdlib.h>
#include<string.h>

#include "../include/memory.h"
#include "../include/object.h"
#include "../include/table.h"
#include "../include/value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table){
	table->count=0;
	table->capacity=0;
	table->entries=NULL;
}

void freeTable(Table* table){
	FREE_ARRAY(Entry,table->entries,table->capacity);
	initTable(table);
}

static uint32_t hashObject(Obj* obj){
	 switch (obj->type) {
    case OBJ_CLOSURE: {
        return (uint32_t)((uint64_t)obj & 0x0000ffff);
        break;
    }
    case OBJ_FUNCTION: 
    case OBJ_BOUND_METHOD:
    case OBJ_CLASS:
    case OBJ_INSTANCE:{
        return (uint32_t)((uint64_t)obj & 0x0000ffff);
    }
    case OBJ_LIST: {
        break;
    }
    case OBJ_NATIVE: {
        return (uint32_t)((uint64_t)obj & 0x0000ffff);
    }
    case OBJ_STRING: {
        break;
    }
    case OBJ_UPVALUE: {
        break;
    }
    default:
        break;
    }
    // Shouldn't reach here
    return 0;
}

static uint32_t hashValue(Value value){
	switch(value.type){
		case VAL_BOOL:{
			if(AS_BOOL(value)) return 1;
			else return 0;
		}
		case VAL_NUMBER:{
			return (uint32_t)AS_NUMBER(value);
		}
		case VAL_NIL:	return 2;
		case VAL_OBJ: return hashObject(AS_OBJ(value));
		default: break;
	}
	return 0;
}

static Entry* findEntry(Entry* entries,int capacity,Value key){
	uint32_t hash;//=key->hash;
	if(IS_STRING(key)){
		hash=AS_STRING(key)->hash;
	}else{
		hash=hashValue(key);
	}
	uint32_t index=hash & (capacity-1);
	Entry* tombstone=NULL;
	for(;;){
		Entry* entry=&entries[index];
		if(entry->empty){
				if(IS_NIL(entry->value)){
					//Empty entry
					return tombstone!=NULL?tombstone:entry;
				}else{
				//we found a tombstone
					if(tombstone==NULL) tombstone=entry;
			}
		}else if(valuesEqual(entry->key,key)){
			return entry;
		}
		index=(index+1) & (capacity-1);
	}
}

static void adjustCapacity(Table* table,int capacity){
	Entry* entries=ALLOCATE(Entry,capacity);
	for(int i=0;i<capacity;i++){
		entries[i].key=NIL_VAL;
		entries[i].value=NIL_VAL;
		entries[i].empty=true;
	}
	table->count=0;
	for(int i=0;i<table->capacity;i++){
		Entry* entry=&table->entries[i];
		if(entry->empty) continue;
		Entry* dest=findEntry(entries,capacity,entry->key);
		dest->key=entry->key;
		dest->value=entry->value;
		dest->empty=false;
		table->count++;
	}
	FREE_ARRAY(Entry,table->entries,table->capacity);
	table->entries=entries;
	table->capacity=capacity;
}

bool tableSet(Table* table,Value key,Value value){
	if(table->count+1>table->capacity*TABLE_MAX_LOAD){
		int capacity=GROW_CAPACITY(table->capacity);
		adjustCapacity(table,capacity);
	}
	Entry* entry=findEntry(table->entries,table->capacity,key);
	bool isNewKey=entry->empty;
	if(isNewKey && IS_NIL(entry->value)) table->count++; //linear probing
	
	entry->key=key;
	entry->value=value;
	entry->empty=false;
	return isNewKey;
}

bool tableDelete(Table* table,Value key){
	if(table->count==0) return false;
	//find the entry
	Entry* entry=findEntry(table->entries,table->capacity,key);
	if(entry->empty) return false;
	//place a tombstone in the entry
	entry->key=NIL_VAL;
	entry->value=BOOL_VAL(true);
	entry->empty=true;
	return true;
}

void tableAddAll(Table* from,Table* to){
	for(int i=0;i<from->capacity;i++){
		Entry* entry=&from->entries[i];
		if(!entry->empty){
			tableSet(to,entry->key,entry->value);
		}
	}
}

bool tableGet(Table* table,Value key,Value* value){
	if(table->count==0) return false;
	
	Entry* entry=findEntry(table->entries,table->capacity,key);
	if(entry->empty) return false;
	
	*value=entry->value;
	return true;
}

ObjString* tableFindString(Table* table,const char* chars,int length,uint32_t hash){
	if(table->count==0) return NULL;
	
	uint32_t index=hash & (table->capacity-1);
	for(;;){
		Entry* entry=&table->entries[index];
		if(entry->empty){
			if(IS_NIL(entry->value)) return NULL;
		}else if(IS_STRING(entry->key) && AS_STRING(entry->key)->length==length && AS_STRING(entry->key)->hash==hash && memcmp(AS_STRING(entry->key)->chars,chars,length)==0){
			return AS_STRING(entry->key);
		}
		index=(index+1)&(table->capacity-1);
	}

}

bool isHashable(Value value){
	if(IS_LIST(value) || IS_MAP(value)) return false;
	else return true;
}

void markTable(Table* table){
	for(int i=0;i<table->capacity;i++){
		Entry* entry=&table->entries[i];
		markObject((Obj*)AS_STRING(entry->key));
		markValue(entry->value);
	}
}

void tableRemoveWhite(Table* table){
	for(int i=0;i<table->capacity;i++){
		Entry* entry=&table->entries[i];
		if(!entry->empty && !AS_STRING(entry->key)->obj.isMarked){
			tableDelete(table,entry->key);
		}
	}
}
