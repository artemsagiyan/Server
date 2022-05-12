CC:=gcc
CFLAGS:=-Wall -Wextra -Werror -Wno-unused-parameter -MMD
LDFLAGS:=-lpthread -lc

all: client server

client server: %: %.o
	$(CC) -o $@ $^ $(LDFLAGS) 

clean:
	rm -f client server *.o *.d
	
include $(wildcard *.d)
