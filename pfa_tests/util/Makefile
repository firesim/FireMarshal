# CC := gcc
# CFLAGS := -O3

CC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static -DRISCV

DEPS := Makefile

install: pfa_launch mytime
	cp pfa_launch ../bin/
	cp mytime ../bin/
	cp run_cg.sh ../bin/

mytime: mytime.o
	 ${CC} ${CFLAGS} -o $@ $^

%.o: %.c $(DEPS)
	    $(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
