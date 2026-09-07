# Secure Package Requirements

> **SUPERSEDED（2026-08-21）**：本规格中的NGU Manifest要求已由`openspec/changes/native-header-load-address-v1/specs/secure-package/spec.md`替代，仅供历史追溯。

## Requirement: Vendor native package compatibility

系统必须保持Vendor Header 1024字节布局及`Image_Type` 0/1/2/3语义，不得要求Vendor公共源码理解NGU项目image type。

### Scenario: SoC image type

- Given 一个FMC/GSP/PMP/RMP/MMP发布包
- When release工具写Vendor Header
- Then Vendor `Image_Type`必须为1
- And 具体项目类型只存在于认证NGU Manifest
- And Vendor type 2/3必须被NGU800P产品stage包入口拒绝

## Requirement: Exact package length

系统必须在提交前和eHSM PASS后验证`Code_Size == package_size - 1024`。

### Scenario: Preflight mismatch

- Given Header `Code_Size`与实际Code Region长度不等
- When adapter执行preflight
- Then Vendor命令不得提交
- And completion为`NOT_SUBMITTED`

### Scenario: Authenticated mismatch

- Given eHSM返回PASS但认证Header长度与调用包长度不等
- When adapter执行post-check
- Then 返回`AUTHENTICATED_FORMAT_ERROR`
- And Manifest、loader、Measurement和release均不得执行

## Requirement: NGU Manifest v1

SoC镜像必须使用ADR-0024冻结的128字节little-endian NGU Manifest v1，包含供Host工具读取的`uint32_t version`、16字节`rollback_counter`和64位baremetal System Address `load_addr/entry_addr`。`version`不得用于防回滚。Manifest不携带expected payload digest；摘要算法由完整Profile唯一派生。

Manifest v1不得包含`abi_major/abi_minor`、`load_addr_domain/entry_addr_domain`、`board_binding_policy`、`measurement_slot`、`component_id`、`digest_algorithm/digest_size/digest_offset`、`extension_offset/extension_size`、expected digest对象或TLV。`payload_offset=128`固定；不兼容变化使用新magic。

### Scenario: Epoch mismatch

- Given 认证Vendor Header `Version_Counter`与Manifest `rollback_counter`不等
- When policy门禁执行
- Then 镜像必须被拒绝
- And counter、Measurement和release不得执行

### Scenario: Removed extension fields

- Given 一个包在Manifest v1中使用旧extension字段或追加TLV
- When parser执行
- Then 镜像必须被拒绝
- And 不得把尾随TLV解释为Manifest元数据

## Requirement: Layered release gate

Vendor PASS不得直接导致镜像执行。只有Header、Manifest、policy、loader源摘要/复制/目标回读摘要比较、counter和Measurement门禁全部满足后，stage owner才可release。

### Scenario: Measurement commit failure

- Given 镜像已验证并加载
- When Measurement commit失败
- Then stage owner不得release该镜像

## Requirement: Production packaging

产品和EMU包必须使用受控真实签名/加密工具链；零签名、固定IV、Naked、伪密文和simulated success不得进入可达路径。

## Requirement: Three product algorithm profiles

SoC产品Secure Package必须完整选择以下三套Profile之一，不得混搭：

- Profile 1：SHA-256 + RSA-2048/RSASSA-PSS + AES-128-CBC；
- Profile 2：SHA-256 + ECDSA/secp256r1 + AES-128-CBC；
- Profile 3：SM3 + SM2 + SM4-CBC。

所有CBC均使用16字节IV并follow Vendor无padding合同；`expected_algorithm_profile`只允许1、2、3。其他Vendor算法不得进入产品package/policy/release允许集合。

产品软件、制包器和BootROM/FMC/GSP路径必须同时实现Profile 1、2、3，三套都必须通过完整启动链、更新链和负向测试；任何SKU配置不得删除其中一套实现。

### Scenario: Mixed algorithm profile

- Given Manifest声明Profile 1但签名、加密或loader摘要算法来自其他Profile
- When stage执行post-Vendor policy校验
- Then 镜像必须被拒绝
- And loader、Measurement、counter和release不得执行

### Scenario: CBC alignment padding

- Given Code Region使用批准Profile且payload长度不是16字节对齐
- When release工具生成包
- Then 只可在payload之后添加0～15字节的全0认证填充
- And 填充计入Vendor `Code_Size`但不计入`payload_size`和Measurement digest

### Scenario: Loaded target mismatch

- Given 已认证源payload已经计算Profile派生摘要
- When loader复制后从目标Region回读重算摘要
- Then 两个摘要必须常量时间比较相等
- And 不相等时目标保持NX且Measurement和release不可达

### Scenario: Vendor-only algorithm

- Given 一个使用RSA-3072、AES-256、CMAC、MD5、DES/TDES或其他非1/2/3组合的包
- When 产品parser/policy执行
- Then 必须fail-close
- And 该算法即使存在Vendor Demo能力测试也不得被产品release
