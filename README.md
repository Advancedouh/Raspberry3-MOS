# MOS移植树莓派3

## 系统环境

​	使用Qemu模拟树莓派3

​	交叉编译器使用aarch64-elf-gcc



## 内容

​	boot：启动的代码，包括设置页表和MMU

​	drivers：树莓派输入输出驱动

​	fs：文件系统内容，本内核为微内核结构，因此是用户态文件系统

​	include：头文件

​	kernel：内核态文件，包括进程管理和中断异常

​	lib：printf和EMMC的SD卡驱动

​	mm：页面管理

​	qemu：内核镜像和SD卡镜像

​	tools：链接脚本

​	user：用户态进程及链接脚本

​	

## 效果

​	目前实现了简易的Shell，可以进行交互

