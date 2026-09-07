# CE-SEC-009：安全固件包、验签解密与Loader合同调查

## Evidence metadata

- Evidence ID：CE-SEC-009
- 日期：2026-07-23
- 父任务：INV-SEC-009
- 资料基线：SRC-0012、SRC-0014、SRC-0016、SRC-0017、SRC-0018；`gsp-pmp-rmp-omp`和`baremetal`当前只读工作区
- 适用范围：NGU800P D0 FMC/GSP/PMP/RMP/MMP安全固件包、BootROM/FMC/GSP验证和加载链
- 操作：读取Vendor手册相关页并复核Vendor Host/Bootloader函数体；只读复核两个公司代码仓的安全组件和制包工具；未修改代码仓、未构建/测试、未执行Git命令

## FACT-01：Vendor原生包是固定1024字节头加Code Region

- `DOCUMENTED`：SRC-0012第29页定义固定1024字节Header，关键字段为：
  - `Signature[256]`：offset 0；
  - `Public_Key[320]`：offset 256；
  - `Encrypt_IV[16]`：offset 576；
  - `Valid_Flag[4]`：offset 592，合法值`0x8E97645D`；
  - `Image_Type[1]`：offset 596；
  - `Plain_Flag[1]`：offset 597；
  - `Naked_Flag[1]`：offset 598；
  - `Reserved[5]`：offset 599；
  - `Code_Size[4]`：offset 604；
  - `Version_Counter[16]`：offset 608；
  - `Public_Key_Ext[400]`：offset 624；
  - `Code`：offset 1024，长度语义由`Code_Size`给出。
- `DOCUMENTED`：SRC-0012把原生`Image_Type`定义为`0=eHSM FW`、`1=SoC镜像使用SoC密钥`、`2=SoC镜像使用eHSM密钥`、`3=eHSM Patch`。
- `DOCUMENTED`：SRC-0016第7～12页要求FMC/GSP/Runtime采用Vendor原生安全包，NGU Manifest位于被签名/加密保护的Code Region内，只有eHSM PASS后才允许解释Manifest。

结论：项目不得在Vendor Header外再加第二个原生Header，也不得把NGU image type复用到Vendor `Image_Type`字节。eHSM Vendor FW保持Vendor type 0；FMC/GSP/PMP/RMP/MMP使用Vendor type 1或2并由受保护NGU Manifest区分项目语义。

## FACT-02：Vendor公共verify API只返回原始状态码

- `DOCUMENTED`：SRC-0014第214页和Host头`include/ehsmdrv/basic/api.h:3014-3029`定义：

```c
uint32_t ehsm_verify_image(ehsm_ctx_st *ctx,
                           const uint8_t *image,
                           uint32_t image_size,
                           bool_t check_version,
                           bool_t boot,
                           uint8_t *image_out);
```

- `VENDOR_IMPLEMENTATION`：`include/ehsmdrv/basic/bl_api.h:43-46`中`ehsm_bl_verify_image`直接映射到上述API。
- `VENDOR_IMPLEMENTATION`：`src/bl_mb.h:143-171`定义BL verify命令`0xff06`，命令带`check_version`、`boot_after_verify`、`image_addr`、`image_size`、`image_out_addr`、`code_addr`和`only_copy_code`；响应只有32位`ret_code`。
- `VENDOR_IMPLEMENTATION`：`src/api.c:3104-3123`清零context后填充命令；公共API没有暴露`only_copy_code`，所以SoC镜像成功输出包含被复制的1024字节Header和解密后的Code Region。
- `VENDOR_IMPLEMENTATION`：同一公共API没有返回实际输出长度、已验证counter、digest、算法profile、transaction id或“命令是否已接受”状态。

结论：项目adapter必须把Vendor raw status完整保留，并在PASS后从输出中的已认证Header和Code Region构建项目结果；不能把`EHSM_OK`直接等价为“允许加载/释放”。

## FACT-03：Vendor直接verify路径按命令长度处理Code Region

- `VENDOR_IMPLEMENTATION`：Bootloader `src/component/fw_verify.c:798-820`只检查`image_size > 1024`，不读取Header内`Code_Size`做精确相等校验。
- `VENDOR_IMPLEMENTATION`：`fw_verify.c:289-400`对SoC镜像按`cmd->image_size - 1024`复制或解密，并把完整`cmd->image_size`传给签名验证。
- `VENDOR_IMPLEMENTATION`：SM2路径`fw_verify.c:450-520`以及其他签名路径都从`image_size - 1024`计算实际hash长度；Header的受保护区域也进入hash。
- `VENDOR_IMPLEMENTATION`：当前直接verify路径未引用`CODE_SIZE_OFFSET`；该字段在upgrade路径另有使用。

结论：Vendor签名仍保护调用者指定的实际字节范围以及Header内容，但当前实现没有强制“Header声称长度”和“命令实际长度”是同一个canonical package。这是集成/制包错误和格式歧义风险，不等同于攻击者可以伪造签名。OPEN-CONFLICT-008随后由ADR-0014关闭：release工具/adapter在提交前和eHSM PASS后均强制精确长度，Vendor公共代码保持不变。

## FACT-04：Vendor counter比较使用16字节原始字段

- `VENDOR_IMPLEMENTATION`：`fw_verify.h:17-26`把`Version_Counter`固定在offset 608；`OTP_VERSION_LENGTH`为16字节。
- `VENDOR_IMPLEMENTATION`：`fw_verify.c:939-962`把两份16字节counter复制为4个`uint32_t`后调用`uint32_BigNumCmp`。
- `VENDOR_IMPLEMENTATION`：utility comparator从最高word向最低word比较；在当前little-endian RISC-V实现中，word 0是最低有效32位、word 3是最高有效32位。

限制：本证据只证明当前代码比较表达，不证明物理counter的单向编码、更新/回读命令、未烧写值或寿命；这些仍属于OPEN-CONFLICT-005。

## FACT-05：公司GSP安全组件是早期stub骨架

- `CODE_FACT`：`components/security/include/security/manifest.h:10-54`的Header常量为72字节，但字段offset延伸到84；`version_counter`仍为`uint32_t`，与已批准的16字节epoch冲突。
- `CODE_FACT`：`components/security/src/verify_flow.c:6-84`调用`ehsm_verify_decrypt_stub()`，随后解析Manifest并直接写进程内Measurement；没有真实Vendor结果、loader、release或跨stage commit。
- `CODE_FACT`：`components/security/src/ehsm_stub.c:11-69`只复制字节并模拟成功状态。
- `CODE_FACT`：`components/security/tools/image_packager/pack_image.py:67-143`手工拼接Header、Manifest和明文payload；签名、公钥、IV和16字节counter均未形成有效发布数据，却把`Plain_Flag`标为ciphertext。
- `CODE_FACT`：现有parser在eHSM PASS门禁后解析Manifest这一层次方向正确，但不能抵消真实eHSM和发布制包缺失。

结论：上述实现只能作为历史骨架定位代码落点，不能逐步“补几个字段”后直接成为产品实现。正式实现应按新包/verify/loader合同替换stub、32位counter和测试制包路径。

## FACT-06：baremetal存在另一套不兼容骨架

- `CODE_FACT`：`baremetal/components/ngu_security/include/ngu_security/ngu_manifest.h`声明64字节Header，但字段offset同样延伸到76；load/entry和counter均为32位。
- `CODE_FACT`：`baremetal/components/ngu_security/src/ngu_verify_flow.c`也调用stub并立即记录进程内Measurement。
- `CODE_FACT`：`baremetal/components/ngu_security/tools/image_packager/pack_ngu_image.py`同样只拼接零签名Header和明文payload。

结论：baremetal后续case实现应调用真实移植的Vendor Host能力并使用发布级或受控的真实测试fixture；不得让这套骨架反向定义产品ABI。两个仓库不能各自维护一套不同Manifest。

## 目标设计与代码差距

| 主题 | 当前代码事实 | 目标设计 |
|---|---|---|
| 原生包 | 两套工具手工拼接零签名Header | Vendor格式唯一Owner，使用批准签名/加密工具链 |
| Counter | Manifest/Measurement仍含32位字段 | 端到端`uint8_t[16]` security epoch |
| Verify结果 | 只保留简化状态 | raw Vendor status、completion、authenticated header、Manifest、policy、loader、release分层 |
| Manifest | 两仓布局不同且Header长度自相矛盾 | `NGU Manifest v1`单一wire ABI |
| Loader | 缺失 | 64位地址+domain、range/overlap/W^X/cache/fence/zeroize |
| Measurement | 函数内直接写BSS表 | loader成功后准备，按跨stagecommit合同发布 |
| Release | 未实现 | 只有policy、loader、Measurement/counter门禁均成功后由stage owner执行 |

## 设计回填

- 新增`docs/04-interfaces/secure-firmware-package.md`。
- 新增`docs/04-interfaces/image-verify-loader.md`。
- 建立OPEN-CONFLICT-008和对应冲突报告。
- 更新BootROM/FMC/GSP详设、Feature追溯、任务计划和OpenSpec约束。

## 限制

- 未运行Vendor工具、签名/加密、EMU或真实eHSM测试。
- 未冻结OPEN-CONFLICT-006涉及的最终绝对地址和linker。
- 未冻结OPEN-CONFLICT-005涉及的物理counter update/readback服务。
- 未取得最终算法发布profile、签名密钥域、board binding和lifecycle mask的批准数值。
