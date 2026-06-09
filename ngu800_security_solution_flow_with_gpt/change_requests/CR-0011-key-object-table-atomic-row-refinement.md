# CR-0011 密钥对象表按单对象单行细化

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0011` |
| Title | 密钥对象表按单对象单行细化 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-05-19 |
| Source / Context Pack | 用户 review：8.8 密钥对象表中部分名称把多个 key 合在一行，阅读和评审不清楚 |
| Related Decision ID | CR-0010 密钥体系三域模型 |

## 2. 背景

第 8.8 节当前采用三域模型，但表格中仍存在多个对象合并到一行的情况，例如：

- `Firmware Update / Key Rotation Signing Key`
- `SOC FW Verify / Upgrade Verify Handle`
- `SOC Debug Verify / User Auth Verify Handle`
- `Image CEK / wrapped CEK`
- `Version Counter / Lifecycle / Control Field`

这会导致项目组无法快速区分每个 key/anchor/state 的职责、位置、设备侧保存内容和冻结状态。

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| 单对象单行 | 第 8.8 节密钥对象表必须一个 key/anchor/state 一行，不把两个不同对象用 `/` 合并到同一行 | 便于评审、实现映射和 eHSM owner 冻结 exact ID | `[CONFIRMED]` |
| 命名格式 | 表中增加“对象 ID / 中文名称”两列，英文对象名表达实现/接口名，中文名称表达评审语义 | 兼顾工程实现和项目组阅读 | `[CONFIRMED]` |
| 不冻结 exact ID | 仍不自行冻结 eHSM physical key ID / level / purpose | exact mapping 由 eHSM owner 冻结 | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 细化第 8.8 节密钥对象表 |
| `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` | Yes | 同步汇报版第 8.8 节 |

## 5. 验收标准

- [x] 8.8.1 外部签名 / 授权密钥表不再把多个 key 合并到同一行。
- [x] 8.8.2 设备内部 eHSM 密钥与状态表不再把多个 key/state 合并到同一行。
- [x] 8.8.3 证书与锚点对象表不再把多个 anchor/cert/policy 合并到同一行。
- [x] 旧的合并命名检索无匹配。
- [x] `git diff --check` 通过。

## 6. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-19 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`；`docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` |
| 未完成项 | exact eHSM physical key ID / key level / key purpose 仍待 eHSM owner 按 TRM 与项目 key slot 策略冻结 |
| 执行说明 | 第 8.8 节改为“一个 key/anchor/state 一行”，增加对象 ID 和中文名称；同步拆分 8.5 架构图与 key slot mapping 表中合并显示的 key/handle/anchor 名称，避免图表表述不一致。 |
