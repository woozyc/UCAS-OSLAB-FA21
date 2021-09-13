CC = riscv64-unknown-linux-gnu-gcc
HOST_CC = gcc
# CFLAGS = -O2  -fno-pic -fno-builtin -nostdinc -N -o bootblock bootblock.s -nostdlib -e main -Wl,-m -T riscv.lds
CFLAGS = -O2  -fno-builtin -nostdlib -T riscv.lds -Iinclude -Wall -mcmodel=medany
DISK = /dev/sdb

BOOTLOADER_ENTRYPOINT = 0x5e000000
KERNEL_ENTRYPOINT = 0x50200000

all: createimage image

bootblock: bootblock.S riscv.lds
	${CC} ${CFLAGS} -o bootblock bootblock.S -ggdb -e main -Ttext=${BOOTLOADER_ENTRYPOINT}

kernel1: kernel1.c head.S riscv.lds
	${CC} ${CFLAGS} -o kernel1 kernel1.c head.S -ggdb -Ttext=${KERNEL_ENTRYPOINT}

kernel2: kernel1.c head.S riscv.lds
	${CC} ${CFLAGS} -o kernel2 kernel2.c head.S -ggdb -Ttext=${KERNEL_ENTRYPOINT}

kernel3: kernel1.c head.S riscv.lds
	${CC} ${CFLAGS} -o kernel3 kernel3.c head.S -ggdb -Ttext=${KERNEL_ENTRYPOINT}

createimage: createimage.c
	${HOST_CC} createimage.c -o createimage -Wall

image: bootblock kernel1 kernel2 kernel3 createimage
	./createimage --extended bootblock kernel1 kernel2 kernel3

clean:
	rm -rf bootblock image createimage kernel1 kernel2 kernel3 *.o

floppy:
	sudo fdisk -l ${DISK}
	sudo dd if=image of=${DISK}2 conv=notrunc
