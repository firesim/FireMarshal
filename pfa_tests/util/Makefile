
CC := gcc
RVCC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static

mytime: mytime.c
	 ${RVCC} -DRISCV ${CFLAGS} -o $@ $^

mytime_nat: mytime.c
	${CC} ${CFLAGS} -o mytime $^

install: pfa_launch mytime
	cp pfa_launch ../bin/
	cp mytime ../bin/
	cp run_cg.sh ../bin/
