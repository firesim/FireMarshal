CC = riscv64-unknown-elf-gcc
DUMP = riscv64-unknown-elf-objdump
CFLAGS = -fno-common -fno-builtin-printf -specs=htif_nano.specs -std=gnu99 -O2 -ffast-math -funroll-loops -I. -mcmodel=medany -g
LD_FLAGS = -ffast-math -static -specs=htif_nano.specs

DEPS := tacit.h rocketcore.h riscv.h mmio.h metal.h

trace-hello.elf: trace-hello.c $(DEPS)
	${CC} ${CFLAGS} -c trace-hello.c
	${CC} ${LD_FLAGS} trace-hello.o -o trace-hello.elf

dump: trace-hello.elf
	${DUMP} -d trace-hello.elf > trace-hello.dump

dump-symbols: trace-hello.elf
	${DUMP} -t trace-hello.elf > trace-hello.symbols

clean:
	rm -f trace-hello.elf trace-hello.o