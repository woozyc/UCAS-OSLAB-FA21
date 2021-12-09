#!/bin/bash

sudo kill `sudo lsof | grep tun | awk '{print $2}'`

if [ "$1" == "ci" ];
then
	/qemu-4.1.1/riscv64-softmmu/qemu-system-riscv64 -nographic -machine virt -m 256M -kernel /u-boot/u-boot -drive if=none,format=raw,id=image,file=${IMG_PATH} -device virtio-blk-device,drive=image -s -netdev tap,id=mytap,ifname=tap0,script=${QEMU_PATH}/etc/qemu-ifup,downscript=/qemu-4.1.1/etc/qemu-ifdown -device e1000,netdev=mytap
else
	/home/stu/OSLab-RISC-V/qemu-4.1.1/riscv64-softmmu/qemu-system-riscv64 -nographic -machine virt -m 256M -kernel /home/stu/OSLab-RISC-V/u-boot/u-boot -drive if=none,format=raw,id=image,file=${IMG_PATH} -device virtio-blk-device,drive=image -s  -netdev tap,id=mytap,ifname=tap0,script=/home/stu/OSLab-RISC-V/qemu-4.1.1/etc/qemu-ifup,downscript=${QEMU_PATH}/etc/qemu-ifdown -device e1000,netdev=mytap 
fi
