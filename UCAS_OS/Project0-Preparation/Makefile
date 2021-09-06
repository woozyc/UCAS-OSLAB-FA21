CC = riscv64-unknown-linux-gnu-gcc
HOST_CC = gcc
# CFLAGS = -O2  -fno-pic -fno-builtin -nostdinc -N -o bootblock bootblock.s -nostdlib -e main -Wl,-m -T riscv.lds
CFLAGS = -O2  -fno-builtin -nostdlib -T riscv.lds -Iinclude -Wall -mcmodel=medany

ENTRYPOINT = 0x50000000

all: test.S riscv.lds
	${CC} ${CFLAGS} -o add add.s -ggdb -e main -Ttext=${ENTRYPOINT}

clean:
	rm -rf test *.o
