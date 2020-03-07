# CC = gcc
# CXX = g++

CC = riscv64-unknown-linux-gnu-gcc
CXX = riscv64-unknown-linux-gnu-g++

CPP_FLAGS = -O3 -std=gnu++11 -static

DEPS = read_kmers.hpp kmer_t.hpp

all: assemble

assemble: assemble.cpp $(DEPS)
	$(CXX) $(CPP_FLAGS) -o $@ $<

clean:
	rm -f assemble
