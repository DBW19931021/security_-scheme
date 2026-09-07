# Python Embedded Tests 发布包使用说明

## 概述
本发布包包含完整的嵌入式测试框架，用于EHSM设备的自动化测试。

## 包含内容
- **config/**: 测试配置文件
  - `test_system.cfg`: 系统配置 (UART端口、波特率等)
  - `test_config_4019.cfg`: 客户专用配置
- **platform_adapter/**: 平台适配层代码
- **resource/**: 测试资源文件 (镜像、ELF文件等)
- **server/**: 服务端代码
- **stp_pro/**: STP协议实现
- **utils/**: 工具类模块
- **tools/third_part/**: 第三方工具和库
- **tools/otp_data/4019/**: OTP数据配置
- **testcases/**: 测试用例 (ehsm_bl 和/或 ehsm_fw)
- **conftest.py**: pytest配置
- **pytest.ini**: pytest初始化配置
- **requirements.txt**: Python依赖包列表
- **run_tests.py**: 测试运行脚本

## 环境要求
- Python 3.8+
- Linux操作系统
- Git (用于版本管理)

## 快速开始

### 1. 创建虚拟环境
```bash
python3 -m venv venv
source venv/bin/activate
```

### 2. 安装依赖
```bash
pip install -r requirements.txt
```

### 3. Server 模块编译（可选）

**注意**: 此步骤仅在需要重新编译 server 模块时执行。发布包中通常已包含编译好的固件。

#### 3.1 编译环境要求

**CM3 平台** (ARM Cortex-M3):
- **ARM GCC 工具链**: `arm-none-eabi-gcc` (推荐版本 14.2+)
- **CMake**: 版本 3.10 或更高

**M130 平台** (RISC-V):
- **RISC-V GCC 工具链**: `riscv32-wing-elf-gcc`
- **CMake**: 版本 3.10 或更高

#### 3.2 快速编译

使用 `build.sh` 脚本快速编译：

```bash
cd server

# 编译 CM3 平台 (ARM Cortex-M3)
./build.sh port/cm3

# 编译 M130 平台 (RISC-V)
./build.sh port/m130

# 清理后重新编译
./build.sh -c port/m130
```

#### 3.3 手动编译步骤

如果需要手动配置：

```bash
cd server
mkdir -p build && cd build

# CM3 平台
cmake -DPORT=port/cm3 .. -G "Unix Makefiles"
make

# M130 平台
cmake -DPORT=port/m130 .. -G "Unix Makefiles"
make
```

#### 3.4 构建产物

编译成功后，在 `server/build/` 目录下会生成：
- `ehsm_host_demo.elf` - ELF 可执行文件
- `ehsm_host_demo.hex` - Intel HEX 格式（用于烧录）
- `ehsm_host_demo.bin` - 二进制格式
- `ehsm_host_demo.map` - 内存映射文件

**详细说明**: 参考 `server/README.md`

### 4. 硬件烧录（可选）

**注意**: 此步骤仅在需要烧录固件到硬件设备时执行。测试前需确保硬件已烧录正确的固件。

#### 4.1 Server 模块烧录

使用 OpenOCD flash loader 烧录 server 模块：

```bash
cd server/scripts

# 烧录到 CM3 平台 (通过 SWD)
python flash_loader.py -t cm3 -m swd -f ../build/ehsm_host_demo.elf -r

# 烧录到 M130 平台 (通过 JTAG)
python flash_loader.py -t m130 -m jtag -f ../build/ehsm_host_demo.elf -r
```

**参数说明**:
- `-t`: 目标平台 (cm3 或 m130)
- `-m`: 连接模式 (swd 或 jtag)
- `-f`: 要烧录的文件
- `-r`: 烧录后复位设备

**前置要求**:
- OpenOCD 0.12.0 或更高版本
- J-Link 调试器及驱动
- 配置 `server/scripts/flash_loader_config.ini` 中的 OpenOCD 路径

**详细说明**: 参考 `server/scripts/README_flash_loader.md`

#### 4.2 Bootloader 烧录

Bootloader (BL) 通常在出厂时已烧录，一般不需要重新烧录。

如需烧录，使用：
```bash
# 使用 flash_loader.py 烧录 Bootloader
python server/scripts/flash_loader.py -t m130 -m jtag -f resource/elf/ehsm_bl.elf -r
```

**烧录条件**:
- 首次部署设备
- Bootloader 固件更新
- 设备固件损坏需要恢复

#### 4.3 Firmware 烧录

使用 `ehsm_fw_loader.py` 烧录 EHSM 固件：

**纯固件模式**（推荐用于快速测试）:
```bash
cd server/scripts

# 直接加载固件到 0x10000000
python ehsm_fw_loader.py resource/image/ehsm_fw.bin
```

**FastBL 模式**（完整加载流程）:
```bash
cd server/scripts

# 使用 FastBL 完整加载流程
python ehsm_fw_loader.py resource/image/ehsm_fw.bin --fastbl fast_bl.bin
```

**参数说明**:
- `--fastbl`: 指定 FastBL 文件（可选）
- `--server`: 指定 OpenOCD 服务器地址（默认 127.0.0.1:3333）
- `--verbose`: 显示详细输出
- `--preview`: 预览生成的脚本而不执行

**详细说明**: 参考 `server/scripts/README_ehsm_fw_loader.md`

### 5. 配置硬件连接
编辑 `config/test_system.cfg` 文件，设置正确的UART端口：
```bash
# 修改UART端口为实际连接端口
CONFIG_TEST_UART_PORT = "/dev/ttyUSB0"  # 根据实际情况修改
CONFIG_TEST_UART_BAUDRATE = 115200
```

### 6. 运行测试

#### 运行单个测试用例
```bash
# 设置配置文件环境变量
export EHSM_CONFIG_FILES="test_system.cfg:test_config_4019.cfg"

# 运行单个测试
python -m pytest testcases/ehsm_bl/test_version.py::test_ehsm_1001 -v
```

#### 运行完整测试套件
```bash
# 运行所有EHSM BL测试
python -m pytest testcases/ehsm_bl/ -v

# 运行所有EHSM FW测试  
python -m pytest testcases/ehsm_fw/ -v

# 运行所有测试
python -m pytest testcases/ -v
```

#### 使用run_tests.py脚本
```bash
python run_tests.py
```

### 7. 查看测试报告
测试运行后会生成：
- `test.log`: 详细日志文件
- `reports/`: 测试报告目录

## 常见问题

### Q: 测试失败提示找不到配置
**A**: 确保设置了正确的环境变量：
```bash
export EHSM_CONFIG_FILES="test_system.cfg:test_config_4019.cfg"
```

### Q: UART连接失败
**A**: 检查以下设置：
1. 确认设备已正确连接
2. 确认UART端口路径正确 (通常是/dev/ttyUSB0或/dev/ttyUSB1)
3. 确认当前用户有串口访问权限
```bash
sudo usermod -a -G dialout $USER  # 添加串口权限
```

### Q: 依赖包安装失败
**A**: 确保系统已安装必要的开发工具：
```bash
sudo apt-get update
sudo apt-get install python3-dev build-essential
```

## 技术支持
如遇问题，请检查：
1. 硬件连接状态
2. 配置文件设置
3. 环境变量配置
4. 日志文件内容

---
*此发布包由打包脚本自动生成*