# ADR-0013：GSP镜像、eHSM能力开放与Runtime隔离范围

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关Feature：SEC-FEAT-004、005、007、010、013～016、019
- 补充关系：补充ADR-0011、ADR-0012

## 决策

### 1. GSP管理的镜像集合

GSP阶段的产品镜像清单固定包含：

1. PMP固件；
2. RMP固件；
3. MMP固件；
4. eHSM Vendor FW固件。

PMP/RMP/MMP分别使用独立image type、package view、load/entry allowlist、counter/epoch记录、Measurement slot、release状态和错误记录，不得合并为一个不可区分的“Runtime”结果。

eHSM Vendor FW同样作为独立受控镜像，由GSP发起加载流程；密码校验、格式接受和最终执行许可仍follow Vendor eHSM BL/FW合同。GSP只有观察到`firmware_done == 1 && firmware_err == 0`后，才发布依赖Vendor FW的服务。

### 2. GSP必须使用的eHSM能力

GSP必须通过统一eHSM service提供：

- 签名；
- 随机数；
- Hash；
- 安全软件方案中各Feature明确需要的其他eHSM能力，包括但不限于secure package验证/解密、counter、key/certificate/rotation及SPDM provider所需能力。

具体算法、模式、强度、key type/slot和参数不从本ADR猜测，继续以当前有效安全软件方案及后续受控amendment为准。Vendor Demo中的算法存在不等于产品自动启用。

本节只定义GSP产品Feature实际依赖的能力，不把Vendor全接口/全算法验证分配给GSP固件。后者由运行在安全核上的独立`baremetal`软件栈执行；baremetal case结果可作为Vendor能力Evidence，但不进入GSP task/service，也不反向定义产品operation。

### 3. 面向其他模块的通用算法服务

1. 通用算法能力需要向其他SoC模块提供，但实施优先级低于安全启动、PMP/RMP/MMP/eHSM FW加载、Measurement和SPDM必需能力。
2. 第一阶段先冻结GSP内部typed service ABI、caller identity、operation allowlist、buffer ownership、quota/serialization、错误和审计；不为了低优先级开放推迟P0启动链。
3. 是否向Host提供外部API尚未裁决。未裁决前默认只允许受信任的SoC内部consumer调用，不创建Host passthrough或任意Vendor command接口。

### 4. LCS与有副作用操作

1. Key安装、生成、轮换、销毁、OTP/LCS/Debug等操作由GSP固件发起。
2. 哪些操作能在特定LCS执行，由eHSM依据其受控策略和状态作最终允许/拒绝；GSP不得覆盖、模拟或降级eHSM拒绝。
3. GSP仍必须执行调用方身份、operation allowlist、参数、地址/长度、key handle/usage和本地产品策略检查。eHSM是最终LCS授权点，不意味着GSP可以把未经授权的Host输入直接透传。
4. ADR-0019后续确认不向Host开放GSP通用算法、Key/Certificate/Rotation、raw eHSM或其他安全服务；仅保留受信任内部SoC typed service。

### 5. Runtime验证失败隔离

1. PMP、RMP或MMP得到确定完成的验证失败时，只把对应Runtime置为`ISOLATED/NOT_RELEASED`，保存独立Measurement/error；其他已经独立验证通过的Runtime不因该密码失败自动隔离。
2. 若存在显式启动依赖，依赖失败Runtime的consumer保持`NOT_RELEASED`，但不把其记录伪装为自身验证失败。
3. Device完成buffer清理后可等待该Runtime的新package；是否重新发送由上位机决定。
4. eHSM transport timeout、异常BUSY或completion unknown是共享service故障，不属于单Runtime密码失败。按ADR-0011 quarantine整个eHSM service，所有尚未闭环的Runtime验证暂停，直至RAS批准并完成恢复。
5. eHSM Vendor FW失败只隔离该FW镜像，但所有明确依赖Vendor FW的服务保持不可用；GSP自身的最小错误上报/管理控制面可以按批准策略继续。

## 优先级

- P0：eHSM FW、PMP、RMP、MMP受控加载；secure package验证/解密；签名、随机数、Hash中被安全启动/Measurement/SPDM直接依赖的路径；错误隔离。
- P1：Key/Certificate/Rotation、SPDM完整provider和安全软件方案明确要求的运行期能力。
- P2：面向其他受信任内部SoC模块的通用算法服务；Host API不排期。

优先级只影响实施顺序，不改变已批准Feature的最终完成要求。

## 影响

- 建立版本化产品operation清单，并据此计算GSP静态descriptor容量和arena总量。
- GSP需要per-image状态表，不能用单一global success/fail表示PMP/RMP/MMP/eHSM FW。
- OPEN-DESIGN-004已由ADR-0017关闭；OPEN-DESIGN-006已由ADR-0019关闭。
- 单Runtime失败测试和共享eHSM service失败测试必须分开，Expected不能混用。

## Review history

- 2026-07-23：负责人批准GSP镜像集合、必需eHSM能力、通用算法低优先级开放、GSP发起/eHSM最终LCS裁决、Host API待定及单Runtime失败局部隔离。
