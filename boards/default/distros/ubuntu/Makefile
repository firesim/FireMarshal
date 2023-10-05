RAWURL=https://cdimage.ubuntu.com/releases/22.04.2/release/ubuntu-22.04.3-preinstalled-server-riscv64+unmatched.img.xz
COMPIMG=rootfs.img.xz
NEWIMG=rootfs.img

# Extract root partition without partition table
# NOTE: Offset must be adjusted for different base image
$(NEWIMG): $(COMPIMG)
	xzcat -k $(COMPIMG) | dd of=$(NEWIMG) bs=512 skip=235520
$(COMPIMG):
	curl $(RAWURL) -o $(COMPIMG)

clean:
	rm -f $(COMPIMG) $(RAWIMG) $(NEWIMG)
