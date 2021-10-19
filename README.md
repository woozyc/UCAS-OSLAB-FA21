# UCAS Operating System Lab, Fall 2021
## 简介
本仓库是中国科学院大学本科生课程 操作系统-研讨课（B0911010Y）的作业代码。\
此操作系统可运行在基于RISC-V指令集的NutShell（果壳）处理器上。

此实验分为三个等级，S-Core, A-Core, C-Core，难度依次递增，高等级必须同时完成低等级的内容。\
为完成此课程，必须至少实现S-Core的全部功能。(以下项目说明中，若无特殊标注，则均为S-Core要求内容)
## 各项目说明
### Project0-Preration
熟悉实验环境，熟悉Makefile，qemu，gdb，git等工具的使用方法，熟悉RISC-V汇编语言。


 + Task1: 汇编语言编写1到100累加程序并编译
 + Task2: 使用objdump指令打印反汇编结果
 + Task3: gdb连接QEMU模拟器单步调试
 + Task4: 上传源码至Gitlab
### Project1-Bootloader
完成操作系统的引导和内核镜像的制作。

 + Task1: Bootloader阶段实现终端输出
 + Task2: 为内核准备运行环境，实现一个可以打印键盘输入的内核
	 - A-Core: 回收bootloader所占内存空间
 + Task3: 完成镜像制作文件createimage.c
 	 - C-Core: 支持双系统启动引导
### Project2-SimpleKernel
实现简易内核。

 + Task1: 进程的非抢占式调度
 + Task2: 互斥锁
 + Task3: 例外处理
 	- A-Core: 实现系统调用
 + Task4: 时钟中断与抢占式调度
 	- S-Core不作要求
 + Task5: sys_fork与优先级调度
 	- 仅C-Core需要完成
 <br>_To be continued..._
## 使用方法
### 编译
首先确保机器已安装RISCV交叉编译环境\
之后在`UCAS/ProjectX-xxxx`目录下终端输入：
```sh
$ make all
```
即可生成单个项目的image镜像文件。详见各项目目录下的Makefile。
### 调试
本实验使用QEMU模拟硬件环境，使用gdb连接调试。\
首先更改run_qemu.sh和debug.sh中的image路径为想要调试的项目中编译好的image，\
然后在仓库根目录下运行debug.sh：
```sh
$ sudo sh debug.sh
```
另开一个终端运行gdb：
```sh
$ riscv64-unknown-linux-gnu-gdb
```
在gdb中连接本地qemu：
```sh
$ target remote :1234
```
即可进行调试。
或者不调试直接运行：
```sh
$ sudo sh run_qemu.sh
```
当出现大写加粗“ **NUTSHELL** ”字样时，输入loadboot加载镜像。
### 上板运行
将分好区的SD卡接入电脑，在项目目录下输入：
```sh
$ make floppy
```
启动SD卡制作完成，插入PYNQ-Z2实验开发板，将开发板连接至电脑。
使用指令：
```sh
$ sudo minicom
```
连接开发板后加载镜像，即可在终端看到打印结果。
## 时间表
2021.08.30-09.13: Project0\
2021.09.06-09.18: Project1\
2021.09.18-10.04: Project2-part1 (Task1 & 2)\
2021.10.04-10.18: Project2-part2 (Task3 & 4 & 5)\
 <br>_To be continued..._


