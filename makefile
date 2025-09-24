
CC = gcc

CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic \
		 -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

OBJ_CMD = $(CC) $(CFLAGS) -c $<

all: mmake

mmake: mmake.o program_handler.o file_handler.o builder.o parser.o
	$(CC) $(CFLAGS) $^ -o mmake 

mmake.o: mmake.c program_handler.h builder.h
	$(OBJ_CMD)

builder.o: builder.c file_handler.h program_handler.h
	$(OBJ_CMD)

program_handler.o: program_handler.c parser.h
	$(OBJ_CMD)

file_handler.o: file_handler.c
	$(OBJ_CMD)

parser.o: parser.c 
	$(OBJ_CMD)

clean:
	rm -f all *.o 
