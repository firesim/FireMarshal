
CC := gcc
RVCC := riscv64-unknown-linux-gnu-gcc
CFLAGS := -O3 -static

pfa_launch: pfa_launch.c
	${RVCC} ${CFLAGS} -o $@ $^

pfa_launch_nat: pfa_launch.c
	${CC} ${CFLAGS} -o $@ $^


mytime: mytime.c
	 ${RVCC} -DRISCV ${CFLAGS} -o $@ $^

mytime_nat: mytime.c
	${CC} ${CFLAGS} -o $@ $^
