# CE-SEC-011：Vendor 16字节版本计数器与通用Counter合同调查

## Evidence metadata

- Evidence ID：CE-SEC-011
- 日期：2026-07-24
- 父任务：INV-SEC-011
- 资料基线：SRC-0018中的`ehsm_bl-2.3.5-4019-72f8fdc`、`ehsm_fw-2.3.2-4019-5a4a0a9`、`ehsm_host-2.3.1-4019-2ee044d`和`python_firmware_tests-1.1.2-4019-8fad478`
- 适用范围：NGU800P D0单一16字节global `security_epoch`、BootROM/FMC/GSP anti-rollback状态机
- 操作：只读复核Vendor BL/FW/Host/测试源码并计算关键文件SHA-256；未修改Vendor、`gsp-pmp-rmp-omp`或`baremetal`，未构建、未测试、未执行Git命令

## 结论摘要

1. Vendor安全启动`Version_Counter`是位于原生Header offset 608、eHSM OTP `+0x30/+0x40`位置的16字节单向编码字段，不是Host API中的64位通用counter。
2. eHSM BL验证非naked SoC镜像成功后，总会把Header中的16字节候选值和valid标志写入eHSM DRAM；`check_version=OFF`只关闭旧值比较，不关闭候选值暂存。
3. eHSM BL不在该验证命令内写OTP。eHSM FW启动时读取DRAM候选值，若更高则写`OTP_SOC_VERSION_ADDRESS`；底层写函数执行写后回读比较。
4. 当前产品设计规定eHSM FW由GSP初始化阶段加载，因此按照当前Vendor机制，FMC不能在释放GSP前完成OTP更新。BL/FW通用OTP读写命令又受LCS限制，不能作为产品LCS下的更新/回读接口。
5. eHSM FW运行期`SOC_VERIFY`是另一条路径：版本检查开启且验证成功时，它在返回Host前直接更新SOC版本OTP。这又早于Host对NGU protected Manifest、policy和payload digest的后置检查。
6. Host中明确存在`ehsm_read_counter(ctx, counter_id, uint64_t *counter_value)`及配套`create/increase/delete` API；不能把“存在Host接口”误写成“没有接口”。但该接口的公开合同是带`counter_id`的64位通用计数器，匹配FW快照没有对应command ID、dispatch或handler正文；它是否属于不同FW版本/可选配置，以及是否与16字节安全启动Version Counter存在Vendor定义的映射，当前证据均未证明。
7. 由此建立OPEN-CONFLICT-009。后续负责人决定本轮不继续收敛物理绑定，当前按16字节值并默认存在FMC可用接口继续逻辑详设；本报告的代码差距保留为实现/EMU前的绑定门禁。

## FACT-01：16字节Version_Counter的物理字段和当前编码

- `VENDOR_IMPLEMENTATION`：BL `src/component/fw_verify.h:25,79`定义Header `VERSION_COUNTER_OFFSET=608`，长度为`OTP_VERSION_LENGTH=16`。
- `VENDOR_IMPLEMENTATION`：FW `src/component/otp_map.h:19-20,27`定义：
  - `OTP_EHSM_VERSION_ADDRESS = CONFIG_EHSM_OTP_BASE_ADDR + 0x30`；
  - `OTP_SOC_VERSION_ADDRESS = CONFIG_EHSM_OTP_BASE_ADDR + 0x40`；
  - `OTP_VERSION_SIZE = 16`。
- `VENDOR_IMPLEMENTATION`：当前FW `inc/config.h:11`配置`CONFIG_EHSM_OTP_DEFAULT_BIT_0=1`，即采用OTP默认位为0的分支。
- `TEST_FIXTURE_FACT`：`python_firmware_tests/.../utils/key.py:603-627`给出的Vendor测试向量为：

| 物理默认值 | VC0 | VC1 | VC2 | VC3 |
|---|---|---|---|---|
| 默认1 | `FF..FF` | `FE FF..FF` | `FC FF..FF` | `F8 FF..FF` |
| 默认0 | `00..00` | `01 00..00` | `03 00..00` | `07 00..00` |

这证明当前测试合同使用逐bit单向推进的thermometer/unary编码，并从16字节数组的第一个octet开始推进。项目ABI应把它当作opaque `uint8_t security_epoch[16]`逐octet传递和比较，不自行转换成普通发布整数。当前证据只冻结上述octet向量和Vendor比较行为；完整128级编码、允许跳级规则、寿命与耗尽策略仍需Vendor确认。

## FACT-02：BL比较允许相同版本并采用16字节整体比较

- `VENDOR_IMPLEMENTATION`：BL `fw_verify.c:934-958`把OTP值和Header值复制为4个`uint32_t`，两次调用`uint32_BigNumCmp`，仅当`image >= otp`时通过。
- `VENDOR_IMPLEMENTATION`：FW `socvrfy_srv.c:881-914`执行相同双重比较；OTP默认位为1时先对两份值逐word取反，使逻辑版本方向保持一致。
- `VENDOR_IMPLEMENTATION`：FW `socvrfy_update_otp_ver_cnt()`和`secboot_update_ver_cnt()`只在候选逻辑版本严格高于OTP时写入，相同版本不写。

这与已批准的“低版本拒绝、相同版本允许且不写、高版本才推进”产品语义一致，但项目代码不得脱离Vendor编码自己实现普通128位加法或大小比较。

## FACT-03：BL verify成功后暂存候选值，check_version不能禁止暂存

- `VENDOR_IMPLEMENTATION`：BL `fw_verify.c:29-34`把SoC候选值暂存区固定在eHSM DRAM `0x20000000 + OTP_DATA_COPY_SIZE`，格式为32位valid `0xA55A5AA5`加16字节值。
- `VENDOR_IMPLEMENTATION`：`fw_verify.c:324-378`先按`cmd->check_version`执行Header比较，再解密/复制和验签；成功后无条件调用`fwverify_save_soc_version_counter()`。
- `VENDOR_IMPLEMENTATION`：`fw_verify.c:245-249`写valid并复制Header中的16字节值；该函数没有`check_version`参数。

因此：

- `check_version=ON`：比较旧值，通过后暂存候选值；
- `check_version=OFF`：不比较旧值，但成功验签后仍暂存候选值；
- `check_version`不能被解释为“compare-only/commit-off”开关；
- 后续任何eHSM FW启动都会消费最后一次成功非naked SoC verify留下的候选值，产品必须保证失败包不会绕过release路径触发FW启动。

## FACT-04：eHSM FW启动时提交暂存值，写后会内部回读

- `VENDOR_IMPLEMENTATION`：FW `src/secboot.c:190-209`检查SOC valid标志，读取`OTP_SOC_VERSION_ADDRESS`，候选更高时调用`misc_write_otp_data()`；写失败登记`FW_ERROR_OTP_WRITE_FAILED`。
- `VENDOR_IMPLEMENTATION`：`secboot.c:227-240`在`secboot_init_hsm()`成功后调用`secboot_update_ver_cnt()`，随后设置`SYS_STA0_HSM_READY`并启动scheduler。
- `VENDOR_IMPLEMENTATION`：FW `src/service/misc_srv.c:223-264`中的`misc_write_otp_data()`先按OTP写粒度写入，再调用`misc_check_otp_data()`回读逐单元比较；不一致返回`EHSM_ERR_OTP_WRITE_CMP_ERROR`。

内部写后回读是可复用的Vendor安全属性，但当前Host没有得到`stored_counter_after[16]`响应。还需Vendor确认`FW_ERROR_OTP_WRITE_FAILED`如何稳定映射为Host可观察的`firmware_err/raw error`，以及该错误是否足以证明“不允许GSP release”。

## FACT-05：当前GSP加载eHSM FW的职责与提交点冲突

已批准设计规定：

1. FMC完整验证GSP并校验FMC/GSP epoch一致；
2. FMC作为唯一Owner更新global counter并取得可证明回读；
3. FMC提交GSP Measurement；
4. FMC最后release GSP；
5. GSP初始化阶段才加载eHSM Vendor FW。

Vendor BL/FW实际顺序是：

1. FMC调用BL verify验证GSP；
2. BL只把候选值暂存在eHSM DRAM；
3. eHSM FW启动时才写OTP；
4. 当前eHSM FW由GSP启动，故OTP写入发生在GSP已经被FMC release之后。

所以当前Vendor机制无法同时满足“GSP负责加载eHSM FW”和“FMC在release GSP前完成counter update/readback”两个已批准条件。

## FACT-06：BL/FW通用OTP读写不能作为产品Counter接口

- `VENDOR_IMPLEMENTATION`：BL `mbcmd_parser.c:290-335`仅在`MCUTEST`或`DEVELOP` LCS允许`BL_OTP_READ/WRITE`，其他LCS返回`EHSM_ERR_EHSM_LIFECYCLE_LIMIT`。
- `VENDOR_IMPLEMENTATION`：FW `misc_srv.c:272-302,354-389`的OTP write/read只允许`MCUTEST/DEVELOP/MANUFACTURE`，不允许`USER/DEBUG/DESTROY`。
- 这些接口接受任意OTP地址和原始buffer，不是绑定`OTP_SOC_VERSION_ADDRESS`、单向编码、candidate authorization和防回滚状态机的typed产品接口。

因此不得在产品设计中让FMC直接调用通用OTP write/read来模拟版本计数器提交和回读。

## FACT-07：FW运行期SOC_VERIFY会在Host后置策略前直接写OTP

- `VENDOR_IMPLEMENTATION`：FW `socvrfy_srv.c:386-408`直接读取并更新`OTP_SOC_VERSION_ADDRESS`。
- `VENDOR_IMPLEMENTATION`：`socvrfy_srv.c:431-459`在解密/复制和签名成功后，只要非naked且`check_version=ON`，就在返回Host前调用更新函数。
- Vendor FW不了解NGU protected Manifest的image type、64位load/entry、FMC/GSP epoch关系、board/LCS项目policy和payload digest二次校验。

因此GSP运行期验证PMP/RMP/MMP时，必须先按已提交global epoch做项目级全16-octet相等检查，且不能依赖FW `SOC_VERIFY`自动推进counter。当前API缺少“验证但永不改counter”的完整承诺，因为`check_version=OFF`在FW路径不会调用更新函数，但该行为和产品Runtime固定epoch策略仍需进入typed operation profile与测试。

## FACT-08：`ehsm_read_counter`确实存在，但当前证据不能把它等同于Version Counter

- `VENDOR_IMPLEMENTATION`：Host `src/mb.h:1757-1839`定义`CREATE/READ/INCREASE/DELETE_COUNTER`命令`0x0010～0x0013`，值由`low_value/high_value`组成64位并带32位`counter_id`。
- `VENDOR_IMPLEMENTATION`：Host `src/api.c:3186-3259`明确提供`ehsm_create_counter()`、`ehsm_read_counter()`、`ehsm_increase_counter()`和`ehsm_delete_counter()`；`ehsm_mb_int()`把两个32位response word组合为`uint64_t`。
- `DELIVERY_GAP`：匹配FW `inc/mb.h`没有这些command ID；`src/schedule/schedule.c:208-307`没有counter dispatch；`src/service/counter_srv.c`和`src/driver/counter_driver.c`没有函数正文。四套交付build map中两个对象的`.text`均为`0x0`，证明当前交付二进制也未链接对应handler。
- `UNCONFIRMED_VENDOR_MAPPING`：Host接口是否面向不同FW版本、可选feature或尚未完成的64位硬件counter，需要Vendor/release note确认。其`uint64_t + counter_id`合同无法直接表达Header/OTP中的16字节thermometer Version Counter；除非Vendor明确给出资源映射和转换合同，否则不能自动替代。

结论：`ehsm_read_counter`接口确实存在；当前缺的是“匹配FW可执行性”和“它与16字节SOC Version Counter的资源映射”证据。不得把Host声明存在直接等同于当前FW功能可用，也不得在没有Vendor映射合同的情况下用其替换已批准16字节global epoch。

## 目标设计差距与停止条件

| 目标合同 | 当前Vendor能力 | 结论 |
|---|---|---|
| BootROM只比较不提交 | BL可比较，但成功后仍暂存候选 | 可接受但必须把暂存副作用纳入状态机 |
| FMC唯一更新Owner | BL verify不直接提交；FW启动自动提交 | 不满足 |
| update/readback后才release GSP | FW当前由GSP加载 | 不满足 |
| 产品LCS可读确切counter-after | 通用OTP read受LCS限制 | 不满足 |
| Runtime只验证固定epoch、不独立推进 | FW `SOC_VERIFY check_version=ON`会直接更新 | 必须固定typed wrapper为不推进路径并做项目级相等校验 |
| 复用Host通用counter | Host 64位声明与FW快照不配套 | 不可用且语义错误 |

按负责人后续决定，允许以默认存在的16字节Counter抽象接口继续完善ABI、状态机、错误和测试设计。准确Vendor函数/command/FW绑定在Counter适配实现或EMU前补齐；此前不得把通用OTP/counter API擅自写入产品路径，也不得声称当前Vendor快照已经证明硬件提交闭环。

## 需要Vendor/项目补充的输入

1. 是否提供产品LCS可用、绑定SOC 16字节version counter的typed `read/compare/commit/status`接口，或等价的BL提交命令。
2. eHSM FW启动写入失败是否一定反映到Host检查的`firmware_err/raw error`，ready与error同时出现时的权威门禁。
3. eHSM DRAM candidate valid/value在eHSM reset、SoC reset和掉电下的保留/清除规则；多次成功verify的覆盖规则是否为正式合同。
4. 完整128级编码、允许跳级/多bit写、最大推进次数、寿命、耗尽预警和RMA/恢复规则。
5. `ehsm_read_counter`等64位counter API是否属于另一交付版本/可选feature，当前匹配FW为何未实现；若Vendor认为它能读取SOC Version Counter，需给出固定`counter_id`、16字节到64位的无损/安全映射、权限、持久性和寿命合同。

## 关键文件指纹

| 文件（相对SRC-0018） | 字节 | SHA-256 |
|---|---:|---|
| `ehsm_bl-2.3.5-4019-72f8fdc/src/component/fw_verify.c` | 36328 | `DFA988C30A58B289B81E829FDA6E450A1C12E78585A64992DC3B7234711C6249` |
| `ehsm_bl-2.3.5-4019-72f8fdc/src/component/mbcmd_parser.c` | 22040 | `ECF58CDA8D951FE49D4ECF90C1D0C39D7028B97790063D83010BF39E67C8C670` |
| `ehsm_fw-2.3.2-4019-5a4a0a9/src/secboot.c` | 10537 | `EC439BBFEE2621A5B47080BBEFF0DC80C796AED715D79F988F73F1FBCF228FF8` |
| `ehsm_fw-2.3.2-4019-5a4a0a9/src/service/socvrfy_srv.c` | 32833 | `082ABCAFC734CA713CE6B07E03BDF135522971B7FB12E5CCBC3E067A15C4C2F7` |
| `ehsm_fw-2.3.2-4019-5a4a0a9/src/service/misc_srv.c` | 33250 | `D76A291E87021292D447EBC20A9058CBE128DC293500B9D46A93E54BFBCBB5E1` |
| `ehsm_fw-2.3.2-4019-5a4a0a9/src/service/counter_srv.c` | 351 | `AE690D0A8A36E57B37365AF76BF0DC49E2025F2C434D5B33992A4F769184DF98` |
| `ehsm_fw-2.3.2-4019-5a4a0a9/src/schedule/schedule.c` | 14856 | `53993B70FD53BE2CFAECD9A72C23924FC755C7477EB17DAB59F07967EDC65462` |
| `ehsm_host-2.3.1-4019-2ee044d/src/mb.h` | 94978 | `E361C302C2B884368AA6F9944D326F3240EBC5086271434581A50BCB2B0C2AB1` |
| `ehsm_host-2.3.1-4019-2ee044d/src/api.c` | 131555 | `919CE0140017FE2E6094E30D994081D7C41004954F03A32BB96F9913255952A7` |
| `python_firmware_tests-1.1.2-4019-8fad478/utils/key.py` | 73398 | `DC2393E4B9BD2FB49B3093B73B60A8E75E01D0F2923DE90752E62EE7B4ADA429` |

## 设计回填

- 新增OPEN-CONFLICT-009及`CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md`。
- 更新anti-rollback、FMC/GSP、Host adapter、verify/loader、Measurement和主详设，把“Vendor compare PASS”“DRAM candidate staged”“OTP committed/readback proven”拆为三个不同状态。
- OPEN-CONFLICT-005保留开放，编码/寿命/耗尽等Vendor输入继续管理；提交顺序和Owner冲突由OPEN-CONFLICT-009单独管理。

## 后续负责人处置

- 2026-07-24：Counter本轮先不继续收敛，逻辑值默认16字节，并默认存在FMC可用的相应接口。
- OPEN-CONFLICT-009改为deferred，不再阻断主详设继续编写，也不重开GSP加载Vendor FW职责。
- 本报告中Host/FW配套和Version Counter映射差距保留为实现/EMU前必须复核的Evidence门禁。

## 限制

- 未运行Vendor Python测试、EMU或真实eHSM。
- 未从正式Vendor手册找到产品LCS下独立version counter commit/readback命令；结论以当前SRC-0018代码快照为准。
- 没有把测试代码新增的注释或历史Review结论当成实测通过Evidence。
- 未证明`FW_ERROR_OTP_WRITE_FAILED`到NGU800P Host可见status/error位的精确映射。
