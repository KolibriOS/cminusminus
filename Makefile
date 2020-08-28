CC = gcc
CFLAGS = -Wall -fpermissive -Wno-write-strings
LDFLAGS =

OBJ = main.o misc.o toka.o tokb.o tokc.o toke.o tokr.o \
	  errors.o debug.o outobj.o outpe.o disasm.o switch.o \
	  outle.o pointer.o new_type.o class.o res.o optreg.o libobj.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

c--: $(OBJ)
	$(CC) -o $@ $^ $(CLFAGS)

.asm.obj:
	nasm -3pr $<

.PHONY: clean

clean:
	rm -rf *.o *~ core
