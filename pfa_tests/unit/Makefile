# CC := gcc
# CFLAGS := -O3 -static
# CFLAGS := -O0 -g

CC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static

DEPS := util.h Makefile

all: unit

unit: unit.c util.o
	$(CC) ${CFLAGS} -o $@ $^

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
