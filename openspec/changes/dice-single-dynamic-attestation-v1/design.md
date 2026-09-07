# Design

## 启动链

```text
BootROM --FMC final-readback digest--> Measurement Entry 0
FMC     --GSP final-readback digest--> Measurement Entry 1
GSP     --FMC/GSP/state--> TCB digest
eHSM    --UDS + TCB digest--> opaque CDI --> opaque Alias Key
GSP     --Alias Public Key--> dynamic X.509 Firmware Alias Leaf
eHSM    --Device Private slot14--> sign TBSCertificate
eHSM    --Alias private handle--> sign Report/SPDM transcript
```

## 证书链

```text
Root CA
  -> Intermediate/Product CA
    -> Device Attestation Issuer (static, pathLen=0, slot14)
      -> Firmware Alias Leaf (dynamic, current boot)
```

Cert0/Cert1只保存静态前三张及紧凑issuer metadata。动态Leaf只在GSP SRAM存在，每次安全冷启动重建。

## TCB和密钥

TCB上下文固定96字节，包含Profile、Hash算法、Lifecycle、Debug、安全状态、FMC digest和GSP digest。eHSM以slot13 UDS执行域分离KDF，CDI和Alias private key不可导出；GSP只取得Alias Public Key和opaque handle。

## Report

Report绑定动态证书、Measurement stable snapshot、状态和Requester nonce/SPDM transcript hash，由Alias Key签名。Host先验静态链和动态证书，再验Report签名和Measurement策略。

## 内存

动态证明新增GSP峰值预算不超过12 KiB；Measurement固定16 KiB不复用。动态证书DER上限4096字节。
