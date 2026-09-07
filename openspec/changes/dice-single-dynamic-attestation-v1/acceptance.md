# Acceptance

## 设计验收

- BootROM只生产FMC Measurement，FMC只生产GSP Measurement，二者不存在X.509/KDF/Key调用。
- GSP拒绝缺失、重复、错误producer或非final-readback的FMC/GSP Entry。
- UDS、CDI和Alias private key不离开eHSM。
- 静态Device Attestation Issuer满足CA/pathLen/KeyUsage，动态Leaf满足CA=false/digitalSignature。
- 动态证书扩展、Report和Measurement中的FMC/GSP/状态完全一致。
- Measurement 16 KiB不用于动态证书或scratch。

## 实现验收场景

1. FMC/GSP digest任一变化时，TCB digest和Alias Public Key随之变化。
2. 相同设备、Profile和TCB状态允许稳定重建相同Alias Public Key。
3. Host nonce变化时证书可不变，但Report/SPDM transcript签名必须覆盖新nonce。
4. 静态Issuer不是CA、pathLen不为0、KeyUsage错误或动态Leaf超4096字节必须fail-close。
5. 动态证书扩展与Measurement不一致、签名错误或使用其他设备Issuer必须拒绝。
6. eHSM timeout/unknown completion时CDI/Alias handle和工作区进入quarantine，不得生成成功Report。
7. reset后动态证书和Alias handle失效，下一次安全启动必须重建。
