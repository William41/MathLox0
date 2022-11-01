SHELL=/bin/sh
CC=gcc
CFLAGS=-O3 -Wall
PROF_FLAG=-pg
DBG_FLAG=-g
BLASFLAG=-lblas
INCLUDE_PATH=/home/william/Desktop/CLOX/include/
SRC_PATH=/home/william/Desktop/CLOX/src/
BUILD_PATH=/home/william/Desktop/CLOX/build/
DATATYPES_PATH=/home/william/Desktop/CLOX/datatypes/
NATIVE_PATH=/home/william/Desktop/CLOX/native/
NUMERIC_PATH=/home/william/Desktop/CLOX/numeric
OBJ_FILES=main.o memory.o chunk.o debug.o value.o compiler.o scanner.o object.o table.o vm.o complex.o native_io.o native_math.o native_coll.o  init_obj.o  rmatrix.o

#clox: $(OBJ_FILES)
	#$(CC) $(addprefix $(BUILD_PATH),main.o memory.o chunk.o debug.o vm.o value.o compiler.o scanner.o object.o table.o complex.o native_io.o native_math.o) -o clox -lm

clox: $(OBJ_FILES)
	 $(CC)  -g $(CFLAGS)  $(addprefix $(BUILD_PATH),*.o) -o clox -lm $(BLASFLAG)
	

chunk.o:$(addprefix $(INCLUDE_PATH),chunk.h)
	$(CC)  -g $(CFLAGS) -c $(addprefix $(SRC_PATH),chunk.c) -o $(addprefix $(BUILD_PATH),chunk.o) 
memory.o:$(addprefix $(INCLUDE_PATH), memory.h)
	$(CC) -g $(CFLAGS)   -c $(addprefix $(SRC_PATH),memory.c) -o $(addprefix $(BUILD_PATH),memory.o) 
debug.o:$(addprefix $(INCLUDE_PATH),debug.h)
	$(CC) -g $(CFLAGS)  -c $(addprefix $(SRC_PATH),debug.c) -o $(addprefix $(BUILD_PATH),debug.o) -pg
value.o:$(addprefix $(INCLUDE_PATH),value.h)
	$(CC) -g $(CFLAGS)   -c $(addprefix $(SRC_PATH),value.c) -o $(addprefix $(BUILD_PATH),value.o) -lm 
scanner.o:$(addprefix $(INCLUDE_PATH),scanner.h)
	$(CC) -g $(CFLAGS) -c $(addprefix $(SRC_PATH),scanner.c) -o $(addprefix $(BUILD_PATH),scanner.o)
compiler.o:$(addprefix $(INCLUDE_PATH), compiler.h)
	$(CC)  -g $(CFLAGS) -c $(addprefix $(SRC_PATH),compiler.c) -o $(addprefix $(BUILD_PATH),compiler.o)
object.o:$(addprefix $(INCLUDE_PATH),object.h)
	$(CC)  -g $(CFLAGS) -c $(addprefix $(SRC_PATH),object.c) -o $(addprefix $(BUILD_PATH),object.o)
table.o:$(addprefix $(INCLUDE_PATH),table.h)
	$(CC) -g $(CFLAGS)  -c $(addprefix $(SRC_PATH),table.c) -o $(addprefix $(BUILD_PATH),table.o)
vm.o:$(addprefix $(INCLUDE_PATH), vm.h)
	$(CC)  -g $(CFLAGS) -c $(addprefix $(SRC_PATH),vm.c) -o $(addprefix $(BUILD_PATH),vm.o) -lm
	
complex.o:$(addprefix $(INCLUDE_PATH), complex.h)
	$(CC)  $(CFLAGS) -c $(addprefix $(DATATYPES_PATH),complex.c) -o $(addprefix $(BUILD_PATH),complex.o)	-lm
rmatrix.o:$(addprefix $(INCLUDE_PATH), rmatrix.h)
	$(CC) $(CFLAGS) -c $(addprefix $(DATATYPES_PATH),rmatrix.c) -o $(addprefix $(BUILD_PATH),rmatrix.o)	-lm $(BLASFLAG)
	

native_io.o:$(addprefix $(INCLUDE_PATH), native_io.h)
	$(CC)  -g $(CFLAGS) -c $(addprefix $(NATIVE_PATH),native_io.c) -o $(addprefix $(BUILD_PATH),native_io.o)
native_math.o:$(addprefix $(INCLUDE_PATH), native_math.h)
	$(CC)  $(CFLAGS) -c $(addprefix $(NATIVE_PATH),clox_math.c) -o $(addprefix $(BUILD_PATH),native_math.o) -lm
native_coll.o:$(addprefix $(INCLUDE_PATH), collect_ops.h)
	$(CC) $(CFLAGS) -c $(addprefix $(NATIVE_PATH),native_coll.c) -o $(addprefix $(BUILD_PATH),native_coll.o) 
init_obj.o:$(addprefix $(INCLUDE_PATH), init_obj.h)
	$(CC) -g $(CFLAGS) -c $(addprefix $(NATIVE_PATH),native_obj.c) -o $(addprefix $(BUILD_PATH),init_obj.o) 
	
main.o:$(addprefix $(SRC_PATH),main.c)
	$(CC) -g $(CFLAGS)  -c $(addprefix $(SRC_PATH),main.c) -o $(addprefix $(BUILD_PATH),main.o) 
clean:
	rm $(addprefix $(BUILD_PATH),*.o)

