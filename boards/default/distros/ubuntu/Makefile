RAWURL=https://cdimage.ubuntu.com/releases/23.10.1/release/ubuntu-23.10-preinstalled-server-riscv64+unmatched.img.xz
COMPIMG=rootfs.img.xz
NEWIMG=rootfs.img

# Extract root partition without partition table
# NOTE: Offset must be adjusted for different base image
# How to obtain the "skip" value.
# curl $(RAWURL) -o $(COMPIMG)
# xzcat -k $(COMPIMG) > $(UNCOMPIMG)
# fdisk -l $(UNCOMPIMG)
$(NEWIMG): $(COMPIMG)
	xzcat -k $(COMPIMG) | dd of=$(NEWIMG) bs=512 skip=235520

$(COMPIMG):
	curl $(RAWURL) -o $(COMPIMG)

clean:
	rm -f $(COMPIMG) $(RAWIMG) $(NEWIMG)
