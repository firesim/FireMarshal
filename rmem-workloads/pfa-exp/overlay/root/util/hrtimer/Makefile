CC := gcc
CFLAGS := -O3 -static -DHRT_ARCH=2

# CC := riscv64-unknown-linux-gnu-gcc
# CFLAGS := -O3 -static -DHRT_ARCH=7

DEPS := hrtimer.h Makefile

getres: getres.c $(DEPS) 
	$(CC) ${CFLAGS} -o $@ $<

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
