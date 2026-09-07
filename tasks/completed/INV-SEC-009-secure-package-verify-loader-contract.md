# INV-SEC-009：Secure Package、Verify与Loader合同

## 调查背景

- 对应Feature：SEC-FEAT-003、004、005、006、009、019。
- 对应正式详设：`docs/03-architecture/secure-boot.md`、`docs/05-software-design/bootrom.md`、`fmc.md`、`gsp.md`。
- 触发问题：B0-R3需要把SRC-0016的eHSM native package、NGU protected Manifest、verify/decrypt、loader和release流程冻结到可直接指导开发的字段/API/状态机粒度。
- 目标资料/代码：SRC-0018 Vendor Host/BL/FW快照、`../gsp-pmp-rmp-omp/components/security`和`../baremetal/components/ngu_security`只读工作区。
- 只读基线：引用`manifests/repositories.yaml`和SRC-0018登记；本任务不执行Git命令。

## 调查目标

确认Vendor native header的真实结构、verify/decrypt入口、输入输出、counter/digest/result语义和错误路径，并核对公司现有package/Manifest/loader骨架与目标方案的差距。

## 需要回答的问题

1. Vendor native header的字段、宽度、Code Region边界和包总长如何由代码表达？
2. Bootloader/Host分别通过哪些API验证SoC image；输入、输出、明文长度、counter、digest和raw错误如何返回？
3. 签名、加密、rollback和image type由Vendor实现检查到什么边界；哪些NGU项目字段必须在PASS后由BootROM/FMC/GSP检查？
4. 当前公司`ehsm_image`、Manifest、policy、measurement、loader和packager实现了什么，哪些只是stub/synthetic或与SRC-0016字段不一致？
5. 是否存在Vendor文档、Vendor代码、SRC-0016和公司实现之间会阻断B0-R3设计的明显冲突？

## 建议检查范围

- SRC-0018 Host `include/`、`src/`、`demo/bl_demo/`及匹配的BL verify实现、secure image结构和工具说明。
- `../gsp-pmp-rmp-omp/components/security`中的image/header/manifest/policy/measurement、BootROM/FMC/GSP调用和制包工具。
- `../baremetal/components/ngu_security`中的同类骨架仅用于对照，不作为目标方案。
- 必须沿入口读取关键函数体、结构体、长度计算、错误返回、buffer ownership和清零路径，不能只根据类型名或注释下结论。

## 证据要求

- 记录仓库/快照、文件、符号和紧凑行范围。
- 记录native package → Vendor verify/decrypt → plaintext Code Region → NGU Manifest → payload loader调用链。
- 区分`CODE_FACT`、`INFERENCE`、`TARGET_DESIGN`、`GAP`、`UNKNOWN`和`CONFLICTING`。
- Vendor格式和行为标为`VENDOR_IMPLEMENTATION`，不得直接外推NGU800P SoC地址、策略或release行为。

## 输出

- 代码证据：`evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md`。
- 正式接口：`docs/04-interfaces/secure-firmware-package.md`、`docs/04-interfaces/image-verify-loader.md`。
- 回填BootROM/FMC/GSP、Feature/任务/OpenSpec/open questions和测试规划。

## 限制和停止条件

- 只修改`security_-scheme`调查和设计资料。
- 不修改Vendor快照、`gsp-pmp-rmp-omp`或`baremetal`，不执行Git、构建或测试。
- 发现有效资料间明显冲突时，停止受影响字段/语义冻结并建立conflict report，在对话中提交负责人裁决。
- OPEN-CONFLICT-005/006未关闭的Vendor counter实现、最终绝对地址和linker字段不得伪造数值。

## 验收

- [x] Vendor结构、入口、关键函数体、长度/错误/输出语义具有可复核证据。
- [x] 公司现有骨架与目标合同的差距明确。
- [x] Manifest和verify/loader合同已形成到字段、API、状态机和阶段门禁粒度。
- [x] 所有未冻结数值和需要负责人裁决的内容均显式列出。

## 调查结果

- 形成CE-SEC-009及两份接口详设。
- 确认Vendor公共verify只返回raw状态，SoC输出包含Header+明文Code Region，项目必须在PASS后构建typed result。
- 确认两个公司仓当前Manifest/stub/packager只是早期骨架，不得作为产品ABI或成功路径。
- 发现原生`Image_Type`说明和`Code_Size`校验Owner冲突，建立OPEN-CONFLICT-008；未受影响的Manifest/loader状态机继续设计。
- 未修改Vendor、`gsp-pmp-rmp-omp`或`baremetal`，未执行Git、构建或测试。
