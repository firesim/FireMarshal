DEFINES := -DSLOW
CFLAGS := -O3 -static

blas: blas.cpp
	riscv64-unknown-linux-gnu-g++ ${DEFINES} ${CFLAGS} -o blas blas.cpp
