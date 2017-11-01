QSORT_DIR := qsort/
TEST_BINS += $(BIN_DIR)/qsort

$(BIN_DIR)/qsort: $(QSORT_DIR)/qsort_main.c $(QSORT_DIR)/util.h $(BIN_DIR)
	riscv64-unknown-linux-gnu-gcc -DRISCV ${CFLAGS} -o $@ $<

