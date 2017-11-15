# CC := gcc
# CFLAGS := -O3 -static

CC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static -DRISCV

DEPS := util.h Makefile

all: fork1 fork2

fork1: fork1.o util.o
	$(CC) ${CFLAGS} -o $@ $^

fork2: fork2.o util.o
	$(CC) ${CFLAGS} -o $@ $^

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
