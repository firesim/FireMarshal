CC = riscv64-unknown-linux-gnu-gcc
CFLAGS := -O2 -static -DRISCV

trace-run: trace-run.c tacit.h
	${CC} ${CFLAGS} -o trace-run trace-run.c

clean:
	rm -f trace-run
