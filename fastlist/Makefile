CC = gcc
CFLAGS = -Wall
LDFLAGS =

OBJ = fastlist.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

fastlist: $(OBJ)
	$(CC) -o $@ $^ $(CLFAGS)

.asm.obj:
	nasm -3pr $<

.PHONY: clean

clean:
	rm -rf *.o *~ core
