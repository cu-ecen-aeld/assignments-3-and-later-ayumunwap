#CC := $(CROSS_COMPILE)gcc

all: writer

writer: writer.o
	$(CC) $(CFLAGS) $(INCLUDES) writer.o -o writer $(LDFLAGS)
writer.o: writer.c
	$(CC) $(CFLAGS) $(INCLUDES) -c writer.c -o writer.o
clean:
	-rm -f writer.o writer
