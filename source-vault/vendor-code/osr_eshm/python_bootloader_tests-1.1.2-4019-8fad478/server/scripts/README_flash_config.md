# Flash Configuration Generator 使用说明

## 概述
这个工具允许您通过编辑Excel表格来管理Jenkins Pipeline的烧录配置，无需修改Jenkinsfile代码。

## 文件结构
```
server/scripts/
├── generate_flash_config.py    # 主要脚本
├── flash_config.csv           # CSV模板文件
├── flash_config.json         # 生成的JSON配置文件（供Jenkinsfile使用）
├── README_flash_config.md     # 本说明文档
└── fast_bl.bin               # FastBL二进制文件
```

## 使用步骤

### 1. 创建Excel模板
```bash
# 进入虚拟环境
cd python_embedded_tests
. venv_claude/Scripts/activate

# 创建Excel模板
cd server/scripts
python generate_flash_config.py template -o flash_config_template.xlsx
```

### 2. 编辑Excel配置
打开生成的`flash_config_template.xlsx`文件，按以下格式编辑：

| customer_id | flash_type | command | args | description |
|-------------|------------|---------|------|-------------|
| b000 | bootloader | python3 ehsm_fw_loader.py | ${WORKSPACE}/${DIR_APP}/build/ehsm_bl_with_sig.bin | Load bootloader for b000 |
| b000 | firmware | python3 ehsm_fw_loader.py | ${WORKSPACE}/${DIR_APP}/build/ehsm_fw.bin | Load firmware for b000 |
| b000 | host | python3 flash_loader.py | -t cm3 -m swd | Flash host for b000 using CM3 SWD |

**字段说明：**
- `customer_id`: 客户标识符（如: b000, 0000, 4030）
- `flash_type`: 烧录类型（必须是: bootloader, firmware, host）
- `command`: 执行的命令
- `args`: 命令参数（自动分割为参数列表）
- `description`: 描述信息

### 3. 生成JSON配置
```bash
# 从Excel生成JSON配置
python generate_flash_config.py generate -i flash_config_template.xlsx

# 从CSV生成JSON配置
python generate_flash_config.py generate -i flash_config.csv
```

生成的`flash_config.json`文件会被Jenkinsfile自动读取。

## 添加新客户的步骤

### 方式1: 编辑Excel文件
1. 打开`flash_config_template.xlsx`
2. 添加新的customer_id行（每个customer需要3行：bootloader, firmware, host）
3. 配置对应的命令和参数
4. 重新生成JSON: `python generate_flash_config.py generate -i flash_config_template.xlsx`

### 方式2: 编辑CSV文件
1. 打开`flash_config.csv`
2. 按格式添加新行
3. 重新生成JSON: `python generate_flash_config.py generate -i flash_config.csv`

## 配置示例

### 添加新客户 "5050"
在Excel或CSV中添加以下行：
```
5050,bootloader,python3 ehsm_fw_loader.py,"${WORKSPACE}/${DIR_APP}/build/ehsm_bl_with_sig.bin",Load bootloader for 5050 customer
5050,firmware,python3 ehsm_fw_loader.py,"${WORKSPACE}/${DIR_APP}/build/ehsm_fw.bin --fastbl ${WORKSPACE}/${DIR_TEST}/server/scripts/fast_bl.bin",Load firmware for 5050 customer with FastBL
5050,host,python3 flash_loader.py,"-t m130 -m jtag",Flash host for 5050 customer using M130 JTAG
```

然后重新生成JSON配置文件。

## 验证配置
生成JSON后，脚本会显示：
- 支持的客户列表
- 配置类型列表
- 生成的文件位置

确保所有需要的customer_id都在支持列表中。

## 注意事项
1. **flash_type必须是**: bootloader, firmware, host
2. **每个customer_id必须包含所有3种flash_type**
3. **args字段**: 复杂参数用引号包围，空格分隔的参数会自动分割
4. **变量替换**: 支持Jenkins变量如`${WORKSPACE}`, `${DIR_SCRIPT}`等
5. **编码问题**: 如果遇到中文显示问题，属于正常现象，不影响功能

## 故障排除
- **缺少依赖**: `pip install pandas openpyxl`
- **文件不存在**: 确保Excel/CSV文件路径正确
- **格式错误**: 检查必需列是否存在，flash_type是否有效
- **权限问题**: 确保有文件写入权限

## 高级用法
```bash
# 指定输出文件
python generate_flash_config.py generate -i config.xlsx -o custom_config.json

# 使用CSV作为输入
python generate_flash_config.py generate -i flash_config.csv

# 创建自定义模板
python generate_flash_config.py template -o my_template.xlsx
```