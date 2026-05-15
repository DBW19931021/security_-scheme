# SEC2 微核固件验证与 Release 机制设计草案

日期：2026-04-28  
状态：brainstorming 设计草案，已获用户认可，待评审  
范围：NGU800P 中 SEC2 对 Host 下发 PM / RAS / Codec 微核固件的验证、装载与 release 机制

## 1. 设计目标

本设计约束 Host 下发 PM / RAS / Codec 微核固件后的安全验证与执行放行流程。

核心目标如下：

- Host 只能投递固件，不能拥有执行放行权。
- SEC2 是 PM / RAS / Codec 固件验证编排、状态维护和 release 的唯一安全裁决方。
- eHSM 是安全服务面，向 SEC2 提供 hash、签名验签、rollback counter 等基础安全启动能力。
- PM / RAS / Codec 在验证通过前必须保持 reset / hold，不得执行。
- PM / RAS / Codec 采用单微核独立验证和独立 release，单个微核失败不影响其他微核。

## 2. 设计原则

### 2.1 Host 不可信原则

Host 只负责将固件投递到普通 staging buffer，并通过 doorbell 通知 SEC2。Host 不得直接写 release、reset、boot vector、start、enable、load done、verify done 或安全 staging ownership 等安全敏感寄存器。

### 2.2 SEC2 安全裁决原则

SEC2 负责消费 Host 请求、复制镜像到安全 staging、解析 header、调用 eHSM、维护状态机、装载镜像并写 release 寄存器。SEC2 不得把 Host 写入的状态、doorbell 或普通 buffer 内容直接视为安全结论。

### 2.3 安全 staging 原则

Host 投递的镜像必须先由 SEC2 复制到 Host 不可修改的安全 staging buffer。后续 header 解析、hash、签名验证、rollback 检查、装载和 release 都基于安全 staging 中的副本，避免 Host 在验证前后篡改被执行对象。

### 2.4 Release 晚于 Verify 原则

release 是独立动作，必须发生在验证通过之后。只有对应微核状态机进入 `VERIFIED`，并成功装载到目标执行区后，SEC2 才允许写 boot vector 和 release / start / enable 相关寄存器。

## 3. 职责边界

| 实体 | 允许职责 | 禁止职责 |
|---|---|---|
| Host | 投递 PM / RAS / Codec 固件到普通 staging buffer；触发投递完成 doorbell；读取 `SUCCESS` / `FAIL` / `ERROR_CODE` | 直接写 release / reset / vector；直接访问安全 staging；直接调用 eHSM；伪造验证通过状态；伪造 load done |
| SEC2 | 消费 Host 请求；复制镜像到安全 staging；解析 header；调用 eHSM；维护单微核状态机；装载镜像；写 release / vector / start 寄存器 | 在未验证通过时 release 微核；信任 Host 的 verify done / load done；允许 Host 修改安全 staging |
| eHSM | 执行 hash、签名验签、rollback counter 检查等基础安全启动服务 | 直接面向 Host；直接 release 微核；理解 Host 业务语义 |
| PM / RAS / Codec | 在 SEC2 release 后从指定 boot vector 执行 | 在验证前运行；自修改启动入口；绕过 SEC2 release |

## 4. 主流程

1. Host 将某个微核镜像写入普通 staging buffer。
2. Host 通过 doorbell 通知 SEC2，通知内容包括 `core_id`、`image_addr`、`image_len` 和请求序号。
3. SEC2 检查请求基本合法性，包括地址范围、长度、对齐、目标微核状态是否允许接收新镜像。
4. SEC2 将镜像从普通 staging buffer 复制到安全 staging buffer。
5. SEC2 基于安全 staging 副本解析 firmware header。
6. SEC2 调用 eHSM 执行基础安全启动检查。
7. eHSM 返回 PASS 后，SEC2 将对应微核状态置为 `VERIFIED`。
8. SEC2 将镜像装载到目标执行区，并配置对应微核 boot vector。
9. SEC2 写 release / start / enable 相关寄存器，放行该微核执行。
10. Host 读取结果，只能看到 `SUCCESS` 或 `FAIL + ERROR_CODE`。

## 5. 验证通过条件

首版验证通过条件采用基础安全启动检查，必须同时满足：

| 检查项 | 执行方 | 要求 |
|---|---|---|
| Header 合法性 | SEC2 + eHSM | magic、长度、版本、签名区域、payload 区间必须合法 |
| `image_type` / `core_id` 匹配 | SEC2 | 镜像声明类型必须与目标 PM / RAS / Codec 一致 |
| Payload hash | eHSM | hash 结果必须与签名保护区域中的摘要一致 |
| 签名验签 | eHSM | 签名必须通过可信公钥或 anchor 校验 |
| Version / rollback counter | eHSM | 镜像版本不得低于对应微核 rollback floor |

首版不把 lifecycle、解密策略、key slot、measurement、eHSM release token 作为 release 的硬性通过条件。这些能力可作为后续增强项。

## 6. 单微核状态机

PM / RAS / Codec 各自维护独立状态机。

正常路径：

```text
IDLE
  -> HOST_LOADED
  -> SEC2_COPIED
  -> VERIFYING
  -> VERIFIED
  -> LOADED
  -> RELEASED
```

失败路径：

```text
FAILED_HOST_LOAD
FAILED_COPY
FAILED_HEADER
FAILED_VERIFY
FAILED_ROLLBACK
FAILED_LOAD
FAILED_RELEASE
FAILED_TIMEOUT
```

状态转移规则：

| 当前状态 | 允许动作 | 下一状态 |
|---|---|---|
| `IDLE` | 接收 Host 投递完成通知 | `HOST_LOADED` |
| `HOST_LOADED` | SEC2 复制到安全 staging 成功 | `SEC2_COPIED` |
| `SEC2_COPIED` | SEC2 发起 eHSM 验证请求 | `VERIFYING` |
| `VERIFYING` | eHSM 返回 PASS | `VERIFIED` |
| `VERIFIED` | SEC2 装载镜像到目标执行区成功 | `LOADED` |
| `LOADED` | SEC2 写 release / start / enable 成功 | `RELEASED` |
| 任一非 `RELEASED` 状态 | 阶段失败或超时 | 对应 `FAILED_*` |

约束：

- 只有 `VERIFIED` 状态允许进入 `LOADED`。
- 只有 `LOADED` 状态允许 release。
- 任一失败状态下，对应微核必须保持 reset / hold。
- 失败后允许 Host 重新投递该微核镜像。
- 某一微核失败不影响其他微核独立验证或 release。

## 7. 错误码

Host 可见错误码按阶段分类。

| 错误码 | 触发条件 | SEC2 响应 | Host 可见结果 |
|---|---|---|---|
| `HOST_LOAD_ERR` | Host 请求格式错误、长度非法、地址非法、目标状态不允许接收 | 拒绝处理，保持 reset / hold | `FAIL + HOST_LOAD_ERR` |
| `COPY_ERR` | SEC2 复制到安全 staging 失败 | 清理安全 staging，保持 reset / hold | `FAIL + COPY_ERR` |
| `HEADER_ERR` | header magic、长度、签名区间、`image_type` 或 `core_id` 不合法 | 拒绝验证，保持 reset / hold | `FAIL + HEADER_ERR` |
| `VERIFY_ERR` | hash 或签名失败 | 保持 reset / hold，允许重新投递 | `FAIL + VERIFY_ERR` |
| `ROLLBACK_ERR` | version 低于 rollback counter | 保持 reset / hold；旧镜像重投仍会失败 | `FAIL + ROLLBACK_ERR` |
| `LOAD_ERR` | 装载目标执行区失败 | 保持 reset / hold | `FAIL + LOAD_ERR` |
| `RELEASE_ERR` | release / start / vector 写入或确认失败 | 保持 reset / hold，记录本地日志 | `FAIL + RELEASE_ERR` |
| `TIMEOUT_ERR` | Host 投递、SEC2 拷贝、eHSM 验证或 release 超时 | 保持 reset / hold，记录内部 timeout 子原因 | `FAIL + TIMEOUT_ERR` |

SEC2 内部可以细分 timeout 子原因：

```text
TIMEOUT_HOST_LOAD
TIMEOUT_SEC2_COPY
TIMEOUT_EHSM_VERIFY
TIMEOUT_RELEASE_WRITE
```

内部 timeout 子原因只进入 SEC2 本地 debug / audit 日志，不暴露给 Host，也不进入 attestation / report。

## 8. Host 可见状态

Host 只允许读取最小结果集合：

```text
SUCCESS
FAIL
ERROR_CODE
```

Host 不可读取以下 SEC2 内部信息：

- 完整微核状态机状态；
- retry counter；
- eHSM 内部失败原因；
- timeout 子原因；
- 安全 staging ownership；
- firewall / DMA 当前配置；
- release 寄存器写入细节；
- SEC2 内部策略判断过程。

## 9. SEC2 独占控制寄存器

以下寄存器或控制位必须只允许 SEC2 secure master 写入：

| 类别 | 寄存器 / 控制位 | 权限要求 |
|---|---|---|
| 微核执行控制 | PM / RAS / Codec reset deassert | SEC2 独占写 |
| 微核执行控制 | PM / RAS / Codec release / start / enable | SEC2 独占写 |
| 启动入口 | PM / RAS / Codec boot vector | SEC2 独占写 |
| 装载控制 | 目标执行区 load control / load done | SEC2 独占写 |
| Staging 控制 | 安全 staging buffer ownership / lock | SEC2 独占写 |
| Firewall | Host / DMA / 微核执行区访问权限切换 | SEC2 独占写 |
| DMA | DMA 启动、目的地址白名单、执行区写窗口 | SEC2 独占写 |
| Host 请求消费 | Host doorbell clear / request consumed | SEC2 独占写 |

Host 仅允许写：

- 普通 staging buffer；
- Host request doorbell；
- 非安全普通数据区。

## 10. Host 绕过路径与防护

| 绕过路径 | 风险 | 防护设计 |
|---|---|---|
| Host 直接写 release / reset / start | 未验证固件被执行 | release / reset / start 只接受 SEC2 secure master 写入 |
| Host 修改 boot vector | 微核跳转到未验证代码 | boot vector 由 SEC2 在 `LOADED` 后写入，Host 不可写 |
| Host 投递后篡改镜像 | 验证对象与执行对象不一致 | SEC2 先复制到安全 staging，验证和装载均基于安全副本 |
| Host 伪造 verify done / load done | 状态机被欺骗 | `VERIFIED` / `LOADED` 仅由 SEC2 内部状态转移产生 |
| Host 重放旧版本签名镜像 | 回滚到有漏洞版本 | eHSM 执行 version / rollback counter 检查 |
| Host 或外设 DMA 写执行区 | 绕过验证写入目标代码区 | firewall / DMA 白名单由 SEC2 独占控制 |
| Host 反复触发 doorbell 干扰状态机 | 状态错乱或拒绝服务 | SEC2 只在目标微核状态允许时消费请求，并由 SEC2 清除 doorbell |

## 11. 失败、超时与回滚处理

### 11.1 验证失败

当 hash 或签名失败时，SEC2 将对应微核状态置为 `FAILED_VERIFY`，保持该微核 reset / hold，并向 Host 返回 `FAIL + VERIFY_ERR`。Host 可重新投递该微核镜像。

### 11.2 回滚失败

当 version 低于 rollback counter 时，SEC2 将对应微核状态置为 `FAILED_ROLLBACK`，保持该微核 reset / hold，并向 Host 返回 `FAIL + ROLLBACK_ERR`。重新投递同一旧版本镜像仍会失败，只有更高版本或满足 counter 策略的新镜像才可能通过。

### 11.3 超时失败

Host 投递超时、SEC2 拷贝超时、eHSM 验证超时、release 写寄存器超时，对 Host 统一返回 `FAIL + TIMEOUT_ERR`。SEC2 内部记录 timeout 子原因到本地 debug / audit 日志。

### 11.4 Release 失败

如果 release / start / enable / vector 写入失败或确认失败，SEC2 将对应微核状态置为 `FAILED_RELEASE`，保持该微核 reset / hold，并向 Host 返回 `FAIL + RELEASE_ERR`。不得进入半 release 状态。

### 11.5 单微核隔离

PM、RAS、Codec 的失败互不连带。PM 失败不阻塞 RAS / Codec，RAS 失败不回滚已经 release 的 PM，Codec 失败不影响 PM / RAS 的状态。

## 12. 首版范围与后续增强项

首版纳入：

- SEC2 安全裁决模型；
- Host 普通 staging 到 SEC2 安全 staging 的拷贝；
- PM / RAS / Codec 单微核独立状态机；
- 基础安全启动检查；
- SEC2 独占 release / vector / staging / firewall / DMA 控制；
- Host 最小结果可见性；
- 阶段分类错误码；
- 单微核隔离失败与重新投递。

后续增强项：

- lifecycle 策略纳入 release 硬条件；
- 按 `image_type / policy` 的解密策略；
- key slot 绑定；
- measurement / attestation report；
- eHSM release token；
- retry counter 和失败锁定；
- PM / RAS / Codec 依赖图 release；
- A/B 槽位与 recovery 策略。

## 13. 评审关注点

评审时建议重点确认：

1. SEC2 独占寄存器清单是否与 RTL 权限模型一致。
2. 安全 staging buffer 是否具备 Host 不可修改属性。
3. firewall / DMA 权限切换是否能阻断 Host 写执行区。
4. 单微核失败不连带其他微核是否符合系统可用性目标。
5. Host 仅可见 `SUCCESS / FAIL / ERROR_CODE` 是否满足联调诊断需求。
6. 首版暂不纳入 lifecycle、解密、measurement 是否满足当前安全启动目标。
