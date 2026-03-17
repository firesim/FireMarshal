CC = riscv64-unknown-linux-gnu-gcc
DUMP = riscv64-unknown-linux-gnu-objdump
CFLAGS := -O3 -static -DRISCV

trace-submit: trace-submit.c tacit.h
	${CC} ${CFLAGS} -o trace-submit trace-submit.c

submit: submit.c
	${CC} ${CFLAGS} -o submit submit.c

dummy: dummy.c 
	${CC} ${CFLAGS} -o dummy dummy.c

dump: ${TARGET}
	${DUMP} -d ${TARGET} > ${TARGET}.dump

dump-symbols: ${TARGET}
	${DUMP} -t ${TARGET} > ${TARGET}.symbols

clean:
	rm -f dummy trace-submit submit
