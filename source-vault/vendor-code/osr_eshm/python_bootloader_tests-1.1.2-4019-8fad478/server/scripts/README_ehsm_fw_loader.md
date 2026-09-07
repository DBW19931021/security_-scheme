# EHSM 固件加载器 (ehsm_fw_loader.py)

基于 OpenOCD 的 EHSM 固件快速加载工具，支持两种加载模式：**纯固件模式**和 **FastBL 模式**。

## 功能特性

- 🔄 **双模式支持**: 支持纯固件直接加载和 FastBL 完整加载
- 🚀 **快速加载**: 使用 OpenOCD 直接加载，避免 GDB 超时问题
- 🎯 **灵活选择**: 根据需要选择简单模式或完整模式
- 📝 **智能参数提取**: 自动从 fastbl_gdb 生成的脚本中提取参数文件
- 🔍 **内存验证**: 加载后自动验证内存内容确保加载成功
- 📊 **详细日志**: 提供详细的加载过程和错误信息
- 🎯 **预览模式**: 支持预览生成的 OpenOCD 脚本而不执行

## 加载模式说明

### 🔹 纯固件模式（推荐用于简单加载）
- **特点**: 直接加载 `ehsm_fw.bin` 到 `0x10000000`
- **优点**: 简单快捷，无需额外文件
- **适用场景**: 单纯的固件加载测试

### 🔹 FastBL 模式（完整加载流程）
- **特点**: 使用 `fastbl_gdb` + `fast_bl.bin` 完整加载
- **优点**: 完整的参数和 FastBL 加载流程
- **适用场景**: 生产环境或需要完整启动流程

## 安装要求

### 系统依赖
- Python 3.6+
- OpenOCD (支持 RISC-V 目标)
- fastbl_gdb 工具（仅 FastBL 模式需要）

### Python 依赖
```bash
pip install pyfastbl  # https://cryptoteam.osr-tech.com/dev_ip/pyfastbl/-/releases
pip install colorama  # Windows 彩色输出支持（可选）
```

## 使用方法

### 基本语法
```bash
# 🔹 纯固件模式（直接加载 ehsm_fw.bin 到 0x10000000）
python ehsm_fw_loader.py <firmware.bin> [选项]

# 🔹 FastBL 模式（通过 fastbl_gdb + fast_bl.bin 加载）
python ehsm_fw_loader.py <firmware.bin> --fastbl <fast_bl.bin> [选项]
```

### 常用示例

#### 1️⃣ 纯固件模式（简单快捷）
```bash
# 直接加载 ehsm_fw.bin 到 0x10000000
python ehsm_fw_loader.py build/ehsm_fw.bin
```
**特点**: 不执行 `fastbl_gdb`，直接加载固件

#### 2️⃣ FastBL 模式（完整流程）
```bash
# 通过 fastbl_gdb 命令和 fast_bl.bin 加载
python ehsm_fw_loader.py build/ehsm_fw.bin --fastbl fast_bl.bin
```
**特点**: 执行 `fastbl_gdb`，加载参数文件 + FastBL

#### 3. 指定远程服务器
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --server 192.168.1.100:3333
```

#### 4. 详细输出模式
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --verbose
```

#### 5. 预览脚本（不执行）
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --preview
```

#### 6. FastBL 模式指定远程服务器
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --fastbl fast_bl.bin --server 192.168.1.100:3333
```

#### 7. 使用自定义配置文件
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --config my_openocd.cfg
```

## 命令行参数

### 必需参数
| 参数 | 描述 |
|------|------|
| `firmware` | 目标固件文件路径（.bin 文件） |

### 可选参数
| 参数 | 默认值 | 描述 |
|------|--------|------|
| `--fastbl FASTBL` | - | FastBL 文件路径（可选，如果未指定则使用纯固件模式） |
| `--server, -s` | `127.0.0.1:3333` | GDB/OpenOCD 服务器地址 |
| `--output, -o` | `load_fw.gdb` | 生成的 GDB 脚本文件名 |
| `--openocd-path` | 自动检测 | OpenOCD 可执行文件路径 |
| `--openocd-script` | `load_fw_openocd.cfg` | 生成的 OpenOCD 脚本文件名 |
| `--config` | `openocd_ftdi.cfg` | OpenOCD 配置文件路径 |
| `--no-hex` | - | 不使用 --hex 选项调用 fastbl_gdb |
| `--verbose, -v` | - | 显示详细输出信息 |
| `--preview, -p` | - | 仅预览生成的脚本，不执行加载 |

## 工作流程

### 🔹 纯固件模式流程
1. **参数验证**: 检查固件文件存在性
2. **创建 OpenOCD 脚本**: 生成直接加载命令
3. **执行加载**: 直接加载 ehsm_fw.bin 到 0x10000000
4. **验证结果**: 读取内存内容验证加载成功

### 🔹 FastBL 模式流程
1. **参数验证**: 检查固件和 FastBL 文件存在性
2. **生成参数文件**: 使用 fastbl_gdb 生成参数文件
3. **创建 OpenOCD 脚本**: 生成参数 + FastBL 加载命令
4. **执行加载**: 加载参数文件到 0xe0041000，加载 FastBL 到 0x10000000
5. **验证结果**: 验证两个地址的内存内容

### 内存映射

| 组件 | 加载地址 | 纯固件模式 | FastBL 模式 | 描述 |
|------|----------|---------|---------|------|
| 内存映射寄存器 | `0x30003800` | ✓ | ✓ | 内存映射备份寄存器 |
| 参数文件 | `0xe0041000` | ✗ | ✓ | 固件参数数据（fastbl_gdb 生成） |
| 固件/FastBL | `0x10000000` | ehsm_fw.bin | fast_bl.bin | 直接固件或快速启动加载器 |

## 配置文件

### OpenOCD 配置
脚本默认使用 `openocd_ftdi.cfg` 作为 OpenOCD 配置文件，该文件应包含：
- FTDI JTAG 适配器配置
- RISC-V 目标配置
- 调试接口设置

### 示例配置文件结构
```
项目目录/
├── ehsm_fw_loader.py          # 主脚本
├── openocd_ftdi.cfg           # OpenOCD 配置
├── build/ehsm_fw.bin          # 目标固件
├── fast_bl.bin                # FastBL 文件
└── params_fw_in_soc_*.bin     # 参数文件（自动生成）
```

## 错误处理

### 常见错误

#### 1. 文件不存在
```
ERROR - FastBL 文件不存在: fast_bl.bin
```
**解决方案**: 检查文件路径是否正确

#### 2. OpenOCD 连接失败
```
ERROR - OpenOCD 执行失败
```
**解决方案**:
- 检查硬件连接
- 确认 OpenOCD 配置文件正确
- 验证目标设备状态

#### 3. fastbl_gdb 命令失败
```
WARNING - fastbl_gdb 失败，将尝试使用默认参数文件路径
```
**解决方案**:
- 确保 fastbl_gdb 在系统 PATH 中
- 检查固件文件格式是否正确

## 输出解释

### 成功加载示例
```
==================================================
    EHSM 固件加载器 (基于 FastBL + OpenOCD)
==================================================
平台: Windows AMD64
固件文件: D:\project\build\ehsm_fw.bin
文件大小: 141.27 KB
服务器地址: 127.0.0.1:3333
OpenOCD 配置: D:\project\openocd_ftdi.cfg
OpenOCD 路径: openocd.exe
FastBL 文件: fast_bl.bin
==================================================

INFO - 步骤 1/2: 生成参数文件信息...
INFO - 步骤 2/2: 使用 OpenOCD 加载固件...
OpenOCD 标准输出:
... (OpenOCD 输出内容)
Params verification - first 4 words: 0x55aaff00 0x60041100 0x10800400 0x23518
Fastbl verification - first 4 words: 0x10001197 0x94018193 0x10008117 0xff810113
All loads completed and verified!

INFO - 检测到加载成功指示: 3 个成功标记
INFO - 内存验证成功: 检测到有效的加载数据
INFO - 固件加载成功!
[OK] 固件加载成功！
```

### 验证指标
- **成功标记**: 查找预期的输出消息
- **内存验证**: 检查加载地址的内存内容
- **非零值检测**: 确认加载的数据不全为零

## 高级用法

### 自定义 OpenOCD 路径
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --fastbl fast_bl.bin --openocd-path /usr/local/bin/openocd
```

### 调试模式
```bash
python ehsm_fw_loader.py build/ehsm_fw.bin --fastbl fast_bl.bin --verbose --preview
```

### 批处理脚本示例
```bash
#!/bin/bash
FIRMWARE="build/ehsm_fw.bin"
FASTBL="fastbl/fast_bl.bin"

echo "开始加载 EHSM 固件..."
python ehsm_fw_loader.py "$FIRMWARE" --fastbl "$FASTBL" --verbose

if [ $? -eq 0 ]; then
    echo "固件加载成功完成"
else
    echo "固件加载失败"
    exit 1
fi
```

## 故障排除

### 1. 编码问题
如果在 Windows 上看到中文显示异常，可以：
- 安装 `colorama`: `pip install colorama`
- 设置控制台编码: `chcp 65001`

### 2. 权限问题
在 Linux/macOS 上可能需要：
```bash
sudo python ehsm_fw_loader.py ...
```

### 3. 路径问题
使用绝对路径避免相对路径问题：
```bash
python ehsm_fw_loader.py /full/path/to/firmware.bin --fastbl /full/path/to/fast_bl.bin
```

## 版本信息

- **当前版本**: 2.0 (优化版)
- **Python 要求**: 3.6+
- **测试平台**: Windows 10/11, Ubuntu 20.04+
- **支持架构**: RISC-V

## 许可证

本工具作为 EHSM 项目的一部分，遵循项目许可证条款。

## 更新日志

### v2.0 (2024-09-09)
- 重构代码结构，改进可维护性
- 添加日志系统支持
- 优化错误处理机制
- 改进配置管理和路径处理
- 简化输出和日志显示
- 优化性能和资源使用

### v1.0
- 初始版本
- 基本的固件加载功能
- 支持自定义 FastBL 文件