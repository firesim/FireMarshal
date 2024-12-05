default: bin.trace good.trace bin.trace.int good.trace.int

vmlinux.dump: boards/default/linux/vmlinux
	riscv64-unknown-elf-objdump -D $< > $@

busybox.dump: wlutil/busybox/busybox_unstripped
	riscv64-unknown-linux-gnu-objdump -D -S $< > $@

OFILE=images/firechip/encrypt-measure-checkpoint/encrypt-measure-checkpoint-bin
bin.dump: $(OFILE)
	riscv64-unknown-elf-objdump -D -S $< > $@

# from RTL
OUTFILE=../../sims/vcs/output/chipyard.harness.TestHarness.DTMHyperscaleTotal1Config/encrypt-measure-checkpoint-bin.0x80000000.0x18013.0.customdts.loadarch.out
bin.trace: vmlinux.dump busybox.dump bin.dump $(OUTFILE)
	./create-func-trace.py $^
	mv $(OUTFILE).functrace $@

# from Spike
GFILE=../../encrypt-measure-checkpoint-bin.0x80000000.0x18013.0.customdts.loadarch/good-trace.log
good.trace: vmlinux.dump busybox.dump bin.dump $(GFILE)
	./create-func-trace.py $^
	mv $(GFILE).functrace $@

bin.trace.int: vmlinux.dump busybox.dump bin.dump $(OUTFILE)
	./create-func-trace-int.py $^
	mv $(OUTFILE).functrace $@

good.trace.int: vmlinux.dump busybox.dump bin.dump $(GFILE)
	./create-func-trace-int.py $^
	mv $(GFILE).functrace $@

.PHONY: dumps
dumps: busybox.dump bin.dump vmlinux.dump

.PHONY: clean
clean:
	rm -rf *.trace
