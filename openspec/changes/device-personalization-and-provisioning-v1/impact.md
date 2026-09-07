# Impact

- `security_-scheme`：主详设、ADR、Open Question、专题、Baseline、状态和测试追溯。
- `gsp-pmp-rmp-omp`：未来实现Provisioning client/证书读取/受控服务，本轮不修改。
- `baremetal`：实现eHSM能力、掉电和制造接口测试；其slot manifest和fixture说明只派生SRC-0024，不形成独立基线。
- Vendor：未来交付BL typed制造query/install/generate/proof/finalize/LCS、对象partial状态/掉电恢复，以及定制轮换能力；本轮不修改Vendor代码。
- RTL/DFT/制造/KMS/CA：需要提供逐die个性化和系统接口输入。
- BootROM/平台：未来生成两个非安全子Profile及mode reason绑定；不增加第三个顶层模式，不在BootROM加入OTP写驱动。
