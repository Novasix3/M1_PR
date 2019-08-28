#! /bin/bash

# Fixer les variables avec les chemins de vos fichiers
HDA="-drive file=pnl-tp.img,format=raw"
HDB="-drive file=myHome.img,format=raw"
KERNEL=linux-4.19/arch/x86/boot/bzImage

# Linux kernel options
CMDLINE="root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1"

FLAGS="-s "

exec qemu-system-x86_64 ${FLAGS} \
     ${HDA} ${HDB} \
     -net user -net nic \
     -serial mon:stdio \
     -boot c -m 2G \
     -kernel "${KERNEL}" \
     -initrd linux-4.19/test/my_initramfs.cpio.gz \
     -append "${CMDLINE}"

