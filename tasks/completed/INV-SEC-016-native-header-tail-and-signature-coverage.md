# INV-SEC-016：Native Header尾部使用与签名覆盖调查

## 目标

确认当前Vendor BL/FW对1024字节Native Header中`Public_Key_Ext[400]`的真实读取范围，以及Header末尾16字节是否进入签名/CMAC认证范围，为删除NGU Manifest和存放`load_addr`提供代码证据。

## 约束

- 只读检查`../fsp`和`../gsp-pmp-rmp-omp`；只修改`security_-scheme`文档。
- 不修改Vendor、产品固件、baremetal或测试工作簿。
- 只执行Git只读状态/基线查询，不写索引、历史或远端。
- 不把当前Vendor代码事实扩大为未来Header版本承诺。

## 输出

- `evidence/code-investigations/CE-SEC-016-native-header-tail-and-signature-coverage.md`
- ADR-0030、固件包专题和主详设第3章回填。

## 验收

- [x] 锁定FSP remote、branch、commit和关键文件工作区差异性质。
- [x] 证明RSA-3072只消费`Public_Key_Ext`前384字节。
- [x] 证明Header偏移1008～1023位于Vendor签名/CMAC覆盖范围。
- [x] 明确当前产品代码尚未实现该overlay和无Manifest解析。
- [x] 未修改三个代码仓、Vendor或测试工作簿。
