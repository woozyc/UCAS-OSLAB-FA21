IMAGE_PATH=/home/stu/OSLab-RISC-V/oslab/UCAS_OS/Project4-VirtualMemory/image

if [ "$1" == "ci" ];
then
	/qemu-4.1.1/riscv64-softmmu/qemu-system-riscv64 -nographic -machine virt -m 256M -kernel /u-boot/u-boot -drive if=none,format=raw,id=image,file=${IMAGE_PATH} -device virtio-blk-device,drive=image -s -S
else
	/home/stu/OSLab-RISC-V/qemu-4.1.1/riscv64-softmmu/qemu-system-riscv64 -nographic -machine virt -m 256M -kernel /home/stu/OSLab-RISC-V/u-boot/u-boot -drive if=none,format=raw,id=image,file=${IMAGE_PATH} -device virtio-blk-device,drive=image -s -S
fi
