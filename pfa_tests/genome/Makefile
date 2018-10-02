# CC = gcc
# CXX = g++

CC = riscv64-unknown-linux-gnu-gcc
CXX = riscv64-unknown-linux-gnu-g++

C_INCLUDE_PATH = ../../mock-interface
CPLUS_INCLUDE_PATH = $(C_INCLUDE_PATH)

CPP_FLAGS = -O3 -std=gnu++14 -static

MOCK_PATH = ../../mock-interface
MOCK_SRC = $(MOCK_PATH)/remote_scratchpad.c
MOCK_OBJ = $(MOCK_SRC:.c=.o)

DEPS = read_kmers.hpp kmer_t.hpp

all: assemble

assemble: assemble.cpp $(MOCK_OBJ) $(DEPS)
	$(LINK.cpp) $< -o $@ -I$(CPLUS_INCLUDE_PATH) $(MOCK_OBJ) $(CPP_FLAGS)

$MOCK_OBJ: $(MOCK_SRC)
	$(CC) -c $<

clean:
	rm -f assemble
	rm -f $(MOCK_OBJ)
