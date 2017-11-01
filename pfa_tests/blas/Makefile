DEFINES := -DSLOW
CFLAGS := -O3 -static --std=gnu++11

blas: blas.cpp
	riscv64-unknown-linux-gnu-g++ ${DEFINES} ${CFLAGS} -o blas blas.cpp

blas_nat: blas.cpp
	g++ ${DEFINES} ${CFLAGS} -o blas blas.cpp
