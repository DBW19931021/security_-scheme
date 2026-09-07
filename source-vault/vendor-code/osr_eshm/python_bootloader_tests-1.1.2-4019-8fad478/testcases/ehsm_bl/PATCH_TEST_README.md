# eHSM Bootloader Patch 测试说明文档

## 概述

本测试模块(`test_patch.py`)实现了对 eHSM Bootloader 的 Patch（补丁）功能的全面测试，包括 Naked 类型和 Cipher 类型镜像的加载、验证和执行测试。

## 测试原理

### Patch 验证机制

为了验证 patch 函数是否真正被执行，测试采用了**寄存器写入验证法**：

1. **验证寄存器**：使用空闲寄存器 `soc_dbg_en_128b0`
   - HSM侧（eHSM内部）地址：`0x30001104` (可写)
   - HOST侧（主机侧）地址：`0x40010088` (只读)
   - 寄存器大小：4字节

2. **Magic Number 机制**：
   - 每个被 patch 的函数在执行时，会向验证寄存器写入唯一的 magic number
   - HOST侧测试程序读取该寄存器值，验证是否为预期的 magic number
   - 如果读到预期值，则证明 patch 函数被成功执行

3. **Magic Number 定义**：
   ```python
   PATCH_OTP_WRITE_MAGIC     = 0x11111111  # otp_write patch 魔数
   PATCH_DBGAUTH_MAGIC       = 0x22222222  # dbgauth_srv_handler patch 魔数
   PATCH_UART_BAUDRATE_MAGIC = 0x33333333  # uart_set_baudrate patch 魔数
   ```

4. **寄存器特性约束**：
   - HOST侧只读，无法清除该寄存器
   - 为避免测试用例间混淆，确保相邻测试用例调用不同的函数
   - 每个函数的 patch handler 写入不同的 magic number

### Patch Demo 工程

测试使用的 patch 镜像来自 `ehsm_patch_demo` 代码仓库。

该 demo 工程中的 patch 函数实现（`src/patch_funcs.c`）：

```c
// Patch for otp_write - 写入 0x11111111
uint32_t patch_otp_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    write_verify_magic(PATCH_FUNC1_MAGIC);  // 0x11111111
    return 0;
}

// Patch for dbgauth_srv_handler - 写入 0x22222222
uint32_t patch_dbgauth_srv_handler(const cmd_req_data_st *req_data, cmd_rsp_data_st *rsp_data)
{
    write_verify_magic(PATCH_FUNC2_MAGIC);  // 0x22222222
    return 0;
}

// Patch for uart_set_baudrate - 写入 0x33333333
void patch_uart_set_baudrate(uint32_t baudrate_div)
{
    write_verify_magic(PATCH_FUNC3_MAGIC);  // 0x33333333
}
```

## 测试用例清单

### 正常测试 - Naked 类型镜像（P001-P005）

| 测试用例ID | 函数名 | 优先级 | 测试目标 |
|-----------|--------|-------|---------|
| EHSM_P001 | test_ehsm_p001 | CRITICAL | 验证Naked类型PATCH镜像加载和执行（完整验证3个patch函数） |
| EHSM_P002 | test_ehsm_p002 | CRITICAL | 验证Naked镜像在MANU模式下加载失败 |
| EHSM_P003 | test_ehsm_p003 | CRITICAL | 验证Naked镜像在USER模式下加载失败 |
| EHSM_P004 | test_ehsm_p004 | CRITICAL | 验证Naked镜像在DEBUG模式下加载失败 |
| EHSM_P005 | test_ehsm_p005 | NORMAL | 验证Naked类型PATCH前后函数行为对比 |

### 正常测试 - Cipher 类型镜像（P006-P007）

| 测试用例ID | 函数名 | 优先级 | 测试目标 |
|-----------|--------|-------|---------|
| EHSM_P006 | test_ehsm_p006 | CRITICAL | 验证Cipher类型PATCH镜像加载和执行（SM2签名+SM4加密） |
| EHSM_P007 | test_ehsm_p007 | NORMAL | 验证Cipher类型PATCH前后函数行为对比 |

### 异常测试（P008-P009）

| 测试用例ID | 函数名 | 优先级 | 测试目标 |
|-----------|--------|-------|---------|
| EHSM_P008 | test_ehsm_p008 | CRITICAL | 验证PATCH功能禁用时不加载patch镜像 |
| EHSM_P009 | test_ehsm_p009 | CRITICAL | 验证Cipher镜像签名错误时加载失败 |

## 测试前准备

### 前置步骤：拷贝 eHSM Bootloader 编译产物

在构建 patch 镜像之前，需要将当前被测版本的 eHSM Bootloader 编译产物拷贝到 patch demo 工程目录，供构建脚本解析符号地址：

```
external/ehsm_patch_demo/ehsm_bl/
├── ehsm_bl.bin
├── ehsm_bl.dis
├── ehsm_bl.elf
└── ehsm_bl.map
```

> 这些文件不包含在仓库中，每次更换被测固件版本时需要重新拷贝。

### 方法一：自动化构建（推荐）⭐

使用自动化构建脚本，一键构建所需的 patch 镜像：

```bash
# 自动构建当前配置的客户镜像（从 pytest.ini 读取）
python tools/patch_update_current.py

# 或显式指定客户 ID（支持缩写）
python tools/patch_update_current.py -cid 4022

# 构建所有客户的镜像
python tools/patch_build_image.py --all

# 只构建 naked 镜像
python tools/patch_build_image.py -cid 4022 --type naked

# 列出已构建的镜像
python tools/patch_list_images.py
```

**智能缓存机制**：
- 自动检测配置文件和源代码变化
- 仅在必要时重新构建（节省时间）
- 构建信息保存在 `resource/patch/<客户ID>/.build_info.txt`

自动化脚本会：
1. 从配置文件读取 IRAM size 和 Patch 地址
2. 调用 patch demo 工程的 Makefile 构建镜像
3. 自动拷贝镜像到 `resource/patch/<客户ID>/` 目录
4. 生成构建信息和时间戳

### 方法二：手动构建（可选）

如果需要手动构建，可以直接操作 `ehsm_patch_demo` 子仓：

```bash
cd external/ehsm_patch_demo
make clean
make all
```

构建完成后，会在 `dist/` 目录下生成：
- `patch_naked_image.bin` - Naked 类型镜像（无签名加密，仅在TEST和DEV模式可用）
- `patch_encrypted_image.bin` - Cipher 类型镜像（SM2签名+SM4加密，所有模式可用）

然后**手动拷贝**镜像文件到测试项目的 `resource/patch/<客户ID>/` 目录下。

### 镜像目录结构

无论使用哪种方法，最终的镜像目录结构应为：

```
python_embedded_tests/
├── resource/
│   └── patch/
│       ├── 4022/
│       │   ├── patch_naked_image.bin
│       │   └── patch_encrypted_image.bin
│       └── ... (其他客户ID文件夹)
└── testcases/
    └── ehsm_bl/
        └── test_patch.py
```

**客户 ID 文件夹说明**：
- 客户 ID 从配置文件中的 `TEST_CUSTOM_ID` 读取（如 `0x4022`）
- 文件夹名称为 4 位十六进制小写字符串（如 `4022`）

`test_patch.py` 中的路径配置（自动根据客户ID动态生成）：

```python
# PATCH镜像文件路径（根据客户ID从项目resource目录获取）
# 客户ID格式转换：0x4022 -> "4022", 0xa001 -> "a001"
_custom_id_str = f"{cfg_data.TEST_CUSTOM_ID:04x}" if hasattr(cfg_data, 'TEST_CUSTOM_ID') else "0000"
PATCH_NAKED_IMAGE_PATH     = f"resource/patch/{_custom_id_str}/patch_naked_image.bin"
PATCH_ENCRYPTED_IMAGE_PATH = f"resource/patch/{_custom_id_str}/patch_encrypted_image.bin"
```

**使用示例**：
- 配置文件 `test_config_4022.cfg` 中定义 `TEST_CUSTOM_ID = 0x4022`
- 将镜像放入 `resource/patch/4022/` 目录
- 测试时会自动加载该客户的镜像文件

### 3. 验证寄存器地址配置

确认验证寄存器地址（这些地址由硬件定义）：

```python
PATCH_VERIFY_REG_HOST_ADDR = 0x40010088  # HOST侧地址（只读）
PATCH_VERIFY_REG_HSM_ADDR = 0x30001104   # HSM侧地址（可写）
```

这些地址应该与 `ehsm_patch_demo` 工程中的定义一致。

### 4. Patch 镜像加载地址

Patch 镜像在 HOST 侧共享内存的固定加载地址：

```python
PATCH_IMAGE_HOST_ADDR = 0x6000d000  # 必须与 CONFIG_BL_PATCH_IMAGE_HOST_ADDR 一致
```

## 测试用例说明

测试代码中已包含详细的实现步骤和注释，此处仅列出各测试用例的核心目标和预期结果。

### EHSM_P001: 验证Naked类型PATCH镜像加载和执行

- **测试目标**：验证 naked 类型 patch 镜像可以正常加载，并且 3 个 patch 函数（otp_write、dbgauth、uart_set_baudrate）真正被执行
- **关键步骤**：配置DEV生命周期 → 加载naked镜像 → 重启eHSM → 验证加载成功 → 依次调用3个被patch的函数并验证寄存器魔数
- **预期结果**：Patch加载成功，寄存器值依次为 0x11111111 → 0x22222222 → 0x33333333

### EHSM_P002-P004: 验证Naked镜像生命周期限制

- **测试目标**：验证 naked 类型镜像仅在 TEST 和 DEV 模式下可用，在 MANU、USER、DEBUG 模式下会被拒绝
- **关键步骤**：配置指定生命周期模式 → 加载naked镜像 → 重启eHSM → 验证加载失败 → 恢复测试环境
- **预期结果**：Naked镜像在非TEST/DEV模式下被拒绝，PATCH_LOAD_FAILED标志置位，Bootloader hold住CPU

### EHSM_P005: 验证Naked类型PATCH前后函数行为对比

- **测试目标**：通过对比有无 patch 时的行为，证明 patch 确实改变了函数行为
- **关键步骤**：场景A-禁用patch调用函数 → 场景B-启用patch调用函数 → 对比寄存器值
- **预期结果**：无patch时寄存器为旧值，有patch时寄存器为0x11111111

### EHSM_P006: 验证Cipher类型PATCH镜像加载和执行

- **测试目标**：验证 cipher 类型 patch 镜像（SM2签名+SM4加密）可以正常加载和执行
- **关键步骤**：配置OTP预装SM2+SM4密钥 → 加载cipher镜像 → 重启eHSM → 验证签名和解密成功 → 验证3个patch函数执行
- **预期结果**：Cipher镜像签名验证和解密成功，寄存器值依次为 0x11111111 → 0x22222222 → 0x33333333

### EHSM_P007: 验证Cipher类型PATCH前后函数行为对比

- **测试目标**：通过对比有无 cipher patch 时的行为，证明 cipher patch 工作正常
- **关键步骤**：与P005相同，但使用cipher镜像
- **预期结果**：无patch时寄存器为旧值，有cipher patch时寄存器为0x11111111

### EHSM_P008: 验证PATCH功能禁用的处理

- **测试目标**：验证 patch_enable=false 时，即使提供了 patch 镜像也不会加载
- **关键步骤**：配置OTP禁用PATCH → 调用函数 → 验证寄存器未被更新
- **预期结果**：Patch功能被禁用，执行原函数，寄存器未更新为patch魔数

### EHSM_P009: 验证Cipher镜像签名验证失败处理

- **测试目标**：验证签名错误的 cipher 镜像会被拒绝
- **关键步骤**：配置OTP预装SM2密钥 → 加载cipher镜像并破坏前32字节签名 → 重启eHSM → 验证加载失败 → 恢复测试环境
- **预期结果**：镜像签名验证失败，Patch加载失败，Bootloader hold住CPU

## 运行测试

### 运行所有patch测试

```bash
cd <python_embedded_tests_path>
pytest testcases/ehsm_bl/test_patch.py -v
```

### 运行特定测试用例

```bash
# 运行 P001（Naked完整验证）
pytest testcases/ehsm_bl/test_patch.py::test_ehsm_p001 -v -s

# 运行 P006（Cipher完整验证）
pytest testcases/ehsm_bl/test_patch.py::test_ehsm_p006 -v -s

# 运行所有Naked类型镜像测试
pytest testcases/ehsm_bl/test_patch.py -k "p001 or p002 or p003 or p004 or p005" -v
```

### 生成Allure测试报告

```bash
# 运行测试并生成allure数据
pytest testcases/ehsm_bl/test_patch.py --alluredir=./allure-results

# 生成并打开报告
allure serve ./allure-results
```

## 测试输出示例

### 成功的测试输出（P001）

```
testcases/ehsm_bl/test_patch.py::test_ehsm_p001 PASSED

Loaded patch image: 2048 bytes from resource/patch/4022/patch_naked_image.bin
Patch image written to HOST memory at 0x6000d000
✅ EHSM restarted
✅ Patch loaded successfully by bootloader
Calling bl_otp_write...
PATCH_VERIFY_REG (0x40010088) = 0x11111111  ✅
✅ otp_write patch verified
Calling bl_debug_auth...
PATCH_VERIFY_REG (0x40010088) = 0x22222222  ✅
✅ dbgauth_srv_handler patch verified
Calling bl_set_baud_rate...
PATCH_VERIFY_REG (0x40010088) = 0x33333333  ✅
✅ uart_set_baudrate patch verified
```

### 失败的测试输出（P002）

```
testcases/ehsm_bl/test_patch.py::test_ehsm_p002 PASSED

Loaded patch image: 2048 bytes from resource/patch/4022/patch_naked_image.bin
✅ BL_ERR set as expected (naked image rejected in MANU mode)
✅ Naked image rejected in MANU mode as expected
✅ Test environment restored to Test mode
```

## 关键验证点总结

### 1. Naked镜像生命周期限制（P001-P004）

- ✅ Naked 镜像仅在 TEST 和 DEV 模式下可用
- ✅ 在 MANU、USER、DEBUG 模式下被拒绝
- ✅ 镜像类型校验机制正常工作

### 2. Patch执行验证（P001, P006）

通过寄存器检查点机制，证明：
- ✅ Patch 函数真正被执行（不是仅仅加载到内存）
- ✅ 函数调用被正确拦截并跳转到 patch 函数
- ✅ 多个 patch 函数都能正确执行
- ✅ 寄存器值依次更新，证明执行顺序正确

### 3. Cipher镜像验证（P006, P009）

- ✅ SM2 签名验证机制正常工作
- ✅ SM4 解密机制正常工作
- ✅ 签名错误的镜像被拒绝
- ✅ 加载失败时 bootloader 正确处理（hold CPU）

### 4. 行为对比验证（P005, P007）

- ✅ 证明 patch 确实改变了函数行为
- ✅ 无patch时执行原函数
- ✅ 有patch时执行patch函数

### 5. 功能开关验证（P008）

- ✅ patch_enable 开关生效
- ✅ 禁用时不加载 patch 镜像

## 注意事项

1. **镜像类型限制**：
   - Naked 镜像仅在 TEST 和 DEV 模式下可用
   - Cipher 镜像在所有模式下可用（需要密钥）
   - 选择正确的镜像类型和生命周期配置

2. **寄存器特性**：
   - 验证寄存器对 HOST 侧只读，无法清除
   - 测试用例间可能会保留前一个测试的值
   - 通过调用不同的函数来避免混淆

3. **Patch镜像**：
   - 确保使用正确的镜像（naked/cipher）
   - 镜像签名/加密算法必须与 OTP 配置匹配
   - Demo工程的密钥配置必须与 OTP 工具一致

4. **测试依赖**：
   - 镜像会在测试运行前自动构建（通过 `ensure_patch_images` fixture）
   - 或手动构建：`python tools/patch_update_current.py`
   - 需要支持 OTP 写入功能

5. **调试建议**：
   - 使用 `-s` 参数查看详细日志输出
   - 检查寄存器地址配置是否正确
   - 验证 patch 镜像是否正确生成
   - 确认 OTP 配置与镜像加密算法匹配

## 问题排查

### Q1: 测试报错 "PATCH load should succeed"

**可能原因**：
- Patch 镜像签名/加密算法与 OTP 配置不匹配
- Patch 镜像文件路径错误
- OTP 未正确配置 patch_enable

**解决方法**：
1. 检查 `external/ehsm_patch_demo` 工程的 Makefile 配置
2. 确认签名密钥和加密密钥与 OTP 一致
3. 重新构建 patch 镜像：`python tools/patch_update_current.py --force`

### Q2: 寄存器值不是预期的魔数

**可能原因**：
- Patch 函数未被执行（patch 未加载成功）
- 寄存器地址配置错误
- 寄存器保留了前一个测试的值

**解决方法**：
1. 先验证 patch 加载成功（`host.bl_patch_success(1)`）
2. 检查寄存器地址配置
3. 调用不同的 API 来更新寄存器值

### Q3: 找不到 patch 镜像文件

**可能原因**：
- 镜像文件未生成或未放置在正确的客户 ID 目录下
- 客户 ID 配置与实际目录不匹配

**解决方法**：
1. 先构建 patch 镜像：`python tools/patch_update_current.py`
2. 检查镜像文件是否生成：`ls external/ehsm_patch_demo/dist/`
3. 确认配置文件中的 `TEST_CUSTOM_ID`（如 `0x4022`）
4. 确认镜像已拷贝到对应的 `resource/patch/<客户ID>/` 目录（如 `resource/patch/4022/`）

### Q4: Naked镜像在DEV模式下也失败

**可能原因**：
- 生命周期配置错误
- OTP 配置中的 vrf_sign_algo 或 vrf_enc_algo 设置不正确

**解决方法**：
确保 Naked 镜像使用 `EHSM_VERIFY_ALGO_INVALID` 和 `EHSM_VRF_ENC_ALGO_INVALID`

## 参考文档

1. **设计文档**：
   - `OSR_eHSM_Bootloader_TRM_4022_1.1.pdf` Chapter 8: 补丁机制

2. **Patch Demo工程**：
   - `external/ehsm_patch_demo/README.md` - Patch开发说明
   - `external/ehsm_patch_demo/src/patch_funcs.c` - Patch函数实现

3. **相关测试**：
   - `testcases/ehsm_bl/test_otp.py` - OTP测试参考
   - `testcases/ehsm_bl/test_upgrade.py` - 升级测试参考

## 结论

本测试模块通过**寄存器检查点机制**，全面验证了 patch 功能的各个方面：

1. ✅ Naked 类型镜像测试（生命周期限制验证）
2. ✅ Cipher 类型镜像测试（签名和加密验证）
3. ✅ Patch 函数真正执行（关键验证）
4. ✅ Patch 失败处理和回退
5. ✅ 功能开关和行为对比

特别是通过 magic number 机制，从 HOST 侧读取 HSM 侧写入的寄存器值，**无可辩驳地证明了 patch 函数确实被执行**，而不仅仅是加载到了内存。这是本测试的核心价值。
