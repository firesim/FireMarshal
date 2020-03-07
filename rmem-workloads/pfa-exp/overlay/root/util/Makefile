# CC := gcc
# CFLAGS := -O3

CC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static -DRISCV

DEPS := Makefile

mytime: mytime.o
	 ${CC} ${CFLAGS} -o $@ $^

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
