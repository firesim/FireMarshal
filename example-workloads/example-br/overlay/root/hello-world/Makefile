RISCV_PFX=riscv64-unknown-elf-
RISCV_GCC=$(RISCV_PFX)g++

BINARY_OPT=-static -specs=htif_nano.specs

TARGET=hello-world

$(TARGET).riscv : $(TARGET).c
	$(RISCV_GCC) $(BINARY_OPT) -o $@ $^

.PHONY: clean
clean:
	rm -f *.riscv
