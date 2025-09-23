
CC = gcc

CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic \
		 -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

OBJ_CMD = $(CC) $(CFLAGS) -c $<

all: mmake.o parser.o
	$(CC) $(CFLAGS) $^ -o mmake 

mmake.o: mmake.c parser.h
	$(OBJ_CMD)

parser.o: parser.c 
	$(OBJ_CMD)

clean:
	rm -f all *.o 
