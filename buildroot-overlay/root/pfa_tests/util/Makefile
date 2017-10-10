
CFLAGS := -O3 -static

mytime: mytime.c
	riscv64-unknown-linux-gnu-gcc -DRISCV ${CFLAGS} -o mytime mytime.c

mytime_nat: mytime.c
	gcc ${CFLAGS} -o mytime_nat mytime.c
