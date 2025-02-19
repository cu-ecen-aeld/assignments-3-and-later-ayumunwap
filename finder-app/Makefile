ifeq ($(CROSS_COMPILE),aarch64-none-linux-gnu-)
	CC := aarch64-none-linux-gnu-
else
	CC := cc
endif
writer: writer.o
	$(CC) -o writer writer.o
writer.o: writer.c
	$(CC) -c writer.c
clean:
	-rm -f writer.o writer
