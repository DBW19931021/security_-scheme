# Impact

- 主详设第3、5、6、7、8、9、12、15章及附录改为无Manifest合同。
- 固件包、verify/loader、RAM布局、Measurement、BootROM/FMC/GSP专题同步。
- `secure-package-manifest-v1`被本change替代；旧OpenSpec只保留历史。
- 目标代码需删除Manifest parser/registry依赖，增加Header tail解析和fixed-stage policy。
- 包开销减少128字节；完整输出原地加载的最大Code Region增加128字节。
- 现有代码、制包器和测试fixture尚未实现，产品实现仍未授权。
