CC=gcc
CFLAGS =-g -Wall -pedantic -Wextra
BINS=server
OBJS = server.o myqueue.o
all: $(BINS)

server: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.dSYM $(BINS)