CC := $(CROSS_COMPILE)gcc

all: aesdsocket

default: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) $(INCLUDES) aesdsocket.o -o aesdsocket $(LDFLAGS)
aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) $(INCLUDES) -c aesdsocket.c -o aesdsocket.o
clean:
	-rm -f aesdsocket.o aesdsocket
