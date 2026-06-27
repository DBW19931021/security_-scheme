# OSR eHSM Bootloader 工程

## 依赖

依赖系统上安装以下工具：

- 编译器：请联系 OSR 获取指定的编译器
- cmake：用于生成 Makefile
- make：用于执行 Makefile 生成目标文件
- Python3：用于执行一些构建脚本

## 构建

```shell
mkdir -p build
cd build
cmake .. -G "Unix Makefiles"
make
```

构建成功后，build 目录下的 elf 可用于 gdb 调试，bin 文件和 vhex 文件是不同的二进制文件，前者可直接查看二进制数据，后者可用于仿真和掩膜。
