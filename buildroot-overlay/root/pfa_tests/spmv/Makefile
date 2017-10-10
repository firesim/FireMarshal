CFLAGS := -O0 -static

spmv: spmv_main.c dataset1.h util.h
	riscv64-unknown-linux-gnu-gcc ${CFLAGS} -o spmv spmv_main.c

spmv-nat: spmv_main.c dataset1.h util.h
	gcc ${CFLAGS} -o spmv spmv_main.c

