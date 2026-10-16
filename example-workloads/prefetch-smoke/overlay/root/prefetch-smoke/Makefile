CC = riscv64-unknown-linux-gnu-gcc
CFLAGS := -O2 -static -Wall

prefetch-smoke: prefetch-smoke.c
	${CC} ${CFLAGS} -o prefetch-smoke prefetch-smoke.c

clean:
	rm -f prefetch-smoke
