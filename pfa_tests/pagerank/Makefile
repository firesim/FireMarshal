CC=riscv64-unknown-linux-gnu-gcc
CFLAGS=-O2 -Wall -std=gnu99 -static

pagerank: pagerank.c pagerank.h util.h pagerank.h barrier.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f pagerank
