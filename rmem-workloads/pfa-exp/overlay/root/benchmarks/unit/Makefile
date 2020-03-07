ifeq ($(CROSS), riscv)
CC = riscv64-unknown-linux-gnu-gcc
# CFLAGS := -O3 -static -DRISCV
CFLAGS := -O0 -g -static -DRISCV
else
CC = gcc
CFLAGS := -O3 -std=gnu99
endif

DEPS := util.h Makefile

all: unit

unit: unit.c util.o
	$(CC) ${CFLAGS} -o $@ $^

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o

cleanall: clean
	rm -f unit
