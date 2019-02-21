rootfs.cpio: rootfs.img

.PHONY: rootfs.img

hpm_counters:=buildroot-overlay/usr/bin/hpm_counters
hpm_counters_dir:=riscv-hpmcounters

$(hpm_counters): $(hpm_counters_dir)/hpm_counters.cxx
	make -C $(hpm_counters_dir)
	cp $(hpm_counters_dir)/hpm_counters $@

rootfs.img: $(hpm_counters)
	cp buildroot-config buildroot/.config
	# Note: buildroot doesn't support make -jN, but it does parallelize anyway.
	make -C buildroot -j1
	cp buildroot/output/images/rootfs.ext2 rootfs.img
