# Acceptance

> **SUPERSEDED（2026-08-21）**：以下验收条件不再适用于当前包格式；现行验收以`native-header-load-address-v1/acceptance.md`为准。

- Vendor `Image_Type=0/1/2/3`和公共源码保持不变。
- FMC/GSP/PMP/RMP/MMP及Die1 SoC stage包固定`Image_Type=1`，产品入口拒绝type 2/3。
- preflight和eHSM PASS后均精确校验`Code_Size == package_size - 1024`。
- Manifest v1固定128字节、little-endian，包含`uint32_t version`、16字节`rollback_counter`和64位baremetal System Address load/entry。
- v1不包含ABI major/minor、地址domain、board/Measurement/component绑定、digest元数据、expected digest或TLV；payload固定offset128。
- `version`供Host工具读取且不参与防回滚；Header `Version_Counter`与Manifest `rollback_counter`逐octet相等。
- loader按Profile对认证源payload和目标回读分别计算摘要并常量时间比较，成功后的目标摘要进入Measurement。
- 公共registry、状态、错误和reserved/unknown规则唯一且可生成。
- verify、loader、Measurement和release职责分离。
- 截短、尾随、溢出、未知required、rollback-counter/源目标摘要/System Address错误均有negative case。
- Profile 1、2、3全部通过完整启动链、更新链和负向测试。
- 产品/EMU发布使用真实签名/加密，无stub/simulated success/零签名fallback。
- OpenSpec、ADR、需求、详设、测试矩阵和Evidence引用一致。
- 当前阶段不修改代码仓、不执行Git操作。
