#!/bin/bash
IMG_PATH=test
/home/stu/OSLab-RISC-V/qemu-4.1.1/riscv64-softmmu/qemu-system-riscv64 -nographic -machine virt -m 256M -kernel /home/stu/OSLab-RISC-V/oslab/UCAS_OS/Project0-Preparation/add -s -S
