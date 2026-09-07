# eHSM Host Server 模块

## 概述

eHSM Host Server模块是一个基于ARM Cortex-M3的嵌入式安全模块主机端实现，提供完整的加密运算和密钥管理功能。

## 目录结构

```
server/
├── CMakeLists.txt          # 主构建配置文件
├── README.md              # 本文档
├── build/                 # 构建输出目录
├── src/                   # 核心源代码
│   ├── api.c             # eHSM API实现
│   ├── mailbox.c         # 邮箱通信实现
│   ├── bl_mb.h           # Bootloader邮箱头文件
│   ├── mb.h              # 邮箱定义头文件
│   └── types_internal.h  # 内部类型定义
├── include/              # 公共头文件
│   └── ehsmdrv/
│       └── basic/
│           ├── api.h     # API接口定义
│           ├── bl_api.h  # Bootloader API
│           ├── mailbox.h # 邮箱接口
│           ├── test_api.h # 测试API
│           ├── types.h   # 基础类型定义
│           └── version.h # 版本信息
├── cm3/                  # ARM Cortex-M3平台支持
│   ├── Keil_Project/     # Keil工程文件（已废弃）
│   ├── System/           # 系统支持文件
│   │   ├── cmsis/        # ARM CMSIS库
│   │   ├── linker/       # 链接脚本
│   │   └── retarget/     # 重定向实现
│   ├── Uart/             # UART驱动
│   ├── arm-none-eabi-gcc.cmake # GCC工具链配置
│   └── port.cmake       # 端口配置
├── riscv/                # RISC-V平台支持（待完善）
├── cmd_table.c           # 命令表定义
├── basic_commands.c      # 基础命令实现
├── hostapi_commands.c    # Host API命令实现
├── server_main.c         # 主程序入口
├── test_types.h          # 测试类型定义
└── version.h             # 版本定义
```

## 编译环境要求

### 工具链
- **ARM GCC工具链**: arm-none-eabi-gcc (推荐版本14.2+)
- **CMake**: 版本3.10或更高
- **Make**: Unix Makefiles支持

### 验证工具链
```bash
# 检查ARM GCC工具链
arm-none-eabi-gcc --version

# 检查CMake版本
cmake --version
```

## 编译步骤

### 快速编译
```bash
cd server
mkdir -p build && cd build
cmake -DPORT=cm3 ..
make
```

### 详细步骤

1. **进入server目录**
   ```bash
   cd server
   ```

2. **创建构建目录**
   ```bash
   mkdir -p build && cd build
   ```

3. **配置构建系统**
   ```bash
   cmake -DPORT=cm3 ..
   ```
   
   参数说明：
   - `-DPORT=cm3`: 指定目标平台为ARM Cortex-M3

4. **执行编译**
   ```bash
   make
   ```

5. **清理构建**
   ```bash
   make clean
   ```

## 构建产物

编译成功后，在`build/`目录下会生成：

- **ehsm_host_demo**: 可执行ELF文件
- **ehsm_host_demo.hex**: Intel HEX格式固件文件（用于烧录）
- **libehsm_host.a**: 静态库文件
- **ehsm_host.map**: 内存映射文件

## 支持的平台

### ARM Cortex-M3 (cm3)
- **状态**: ✅ 完全支持
- **工具链**: arm-none-eabi-gcc
- **启动文件**: startup_CM3DS_gcc.s
- **链接脚本**: ehsm_host.ld

### RISC-V (riscv)
- **状态**: 🚧 开发中
- **工具链**: riscv-none-elf-gcc

## 功能特性

- ✅ 对称加密算法 (AES, SM4, ChaCha20)
- ✅ 非对称加密算法 (RSA, SM2, SM9)
- ✅ 哈希算法 (SHA-1/256/384/512, SM3)
- ✅ 消息认证码 (HMAC, GMAC)
- ✅ 数字签名 (RSA-PSS, ECDSA, SM2)
- ✅ 密钥管理 (生成、导入、导出、删除)
- ✅ 安全启动和固件认证
- ✅ 真随机数生成器 (TRNG)

## 编译选项说明

### 关键编译标志
- `-mcpu=cortex-m3`: 指定目标CPU
- `-mthumb`: 使用Thumb指令集
- `-nostdlib`: 不链接标准库
- `-Wconversion`: 启用类型转换警告
- `-O2`: 优化级别2

### 安全编译选项
- `-ffunction-sections`: 函数段分离
- `-fdata-sections`: 数据段分离
- `-Wl,--gc-sections`: 删除未使用段

## 调试和验证

### 查看构建详情
```bash
make VERBOSE=1
```

### 检查生成的hex文件
```bash
ls -la build/ehsm_host_demo.hex
head -5 build/ehsm_host_demo.hex
```

### 验证ELF文件信息
```bash
arm-none-eabi-objdump -h build/ehsm_host_demo
arm-none-eabi-size build/ehsm_host_demo
```

## 常见问题

### 1. 工具链找不到
```
CMake Error: Could not find toolchain file
```
**解决方案**: 确保arm-none-eabi-gcc在系统PATH中

### 2. 链接错误
```
undefined reference to 'function_name'
```
**解决方案**: 检查源文件是否正确添加到CMakeLists.txt

### 3. 内存不足
```
region 'ram' overflowed
```
**解决方案**: 检查链接脚本内存配置

## 开发指南

### 添加新功能
1. 在`src/`目录添加源文件
2. 更新`CMakeLists.txt`中的源文件列表
3. 在`include/`目录添加对应头文件
4. 重新编译测试

### 移植到新平台
1. 在根目录创建新的平台目录
2. 复制并修改`port.cmake`配置文件
3. 提供平台特定的启动文件和链接脚本
4. 更新主CMakeLists.txt添加新平台支持

---

**注意**: 本模块包含安全敏感代码，请确保在安全的开发环境中使用，并遵循相关的安全开发规范。