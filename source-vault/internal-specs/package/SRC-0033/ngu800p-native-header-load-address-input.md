# NGU800P取消Manifest并复用Native Header尾部输入

- Source ID：SRC-0033
- 日期：2026-08-21
- Owner：项目负责人
- 状态：用户确认的设计输入

## 用户确认内容

1. NGU800P SoC安全固件包不再携带原NGU Manifest头，Manifest整体删除。
2. Vendor Native Header保持1024字节物理长度。
3. Native Header中的`Public_Key_Ext`字段总长400字节，当前代码只使用前384字节。
4. 使用其末尾16字节承载NGU800P加载信息：前8字节为`load_addr`，后8字节暂时保留。
5. 原Manifest中的其他字段均不再放入镜像包。

## 工程解释

- `load_addr`采用little-endian 64位System Address。
- 末尾8字节当前固定为0；任何非0输入拒绝，不建立隐式扩展。
- 本输入只改变NGU800P Vendor type 1 SoC stage包；eHSM Vendor FW的type 0原生包不增加NGU overlay。
- 具体安全门禁、字段替代来源、加载长度、入口语义和兼容策略由ADR-0030及主详设定义。

## 截止本次审视的代码事实

当前`../fsp`为`master@a5193e31dfb93238595b822d2d94e07143ee903d`。Vendor BL/FW把`Public_Key_Ext`定义在Header偏移624；RSA-3072只复制384字节模数，即使用624～1007，未读取1008～1023。Vendor签名/CMAC计算覆盖Header偏移592～1023及Code Region，因此1008～1023位于认证范围。准确证据见CE-SEC-016。

这些代码事实属于`VENDOR_IMPLEMENTATION`，不自动扩大为未来Vendor版本承诺；升级Header或增加更长公钥算法时必须重新验证末尾16字节是否仍可复用。
