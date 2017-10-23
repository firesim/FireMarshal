XCOMPILE_PREFIX=riscv64-unknown-linux-gnu-
CXX=$(XCOMPILE_PREFIX)g++
OBJDUMP=$(XCOMPILE_PREFIX)objdump

FLAGS= -std=c++0x -O2 -g -static

c_srcs=counter_check.cpp mem_range_check.cpp

targets=$(patsubst %.cpp,%,$(c_srcs))
dumps=$(patsubst %.cpp,%.dump,$(c_srcs))

all: $(targets) $(dumps)

install: $(targets)
	mkdir -p ../buildroot-overlay/usr/bin
	cp -f $^ ../buildroot-overlay/usr/bin/

clean:
	rm -f $(targets) $(dumps)

%: %.cpp
	$(CXX) $(FLAGS) $< -o $@

%.dump: %
	$(OBJDUMP) -D $< > $@


.PHONY: all install clean
