# OSR OTP tool 变更日志

## 0.3.1

- 增加输出 C 数据文件的命令行参数及 C 接口。

## 0.3.0

- values.toml 中增加引用密钥文件的支持

## 0.2.2

- 更新用户手册

## 0.2.1

- `key` 字段增加 `no_crc32` bool 选项，默认为 `false` ，当设置为 `true` 时，不生成密钥的 CRC32 校验码（此功能通常仅用于测试）

## 0.2.0

- `bits` 字段支持指定整数值，如 `trng_rst_times = 15` ，等价于 `trng_rst_times = "1111"`
- `otptool` 的 `gen` 子命令增加 `--interactive` 模式和 `--watch` 模式，详见用户指南
  - `--interactive` 模式允许用户交互式输入 OTP 配置
  - `--watch` 模式允许用户监视 OTP 配置文件的变更并自动重新生成 OTP
- `otptool` 增加 `show-layout` 子命令，用于查看 OTP 布局信息，详见用户指南

## 0.1.1

- 修正 toml-demo/layout.toml 中 `lifecycle` 的 `debug` 值定义字节序错误问题
- `key` 的 `hash` 字段取值支持 `true` / `false` 以及 "none", "auto", "sm3", "sha256"，详见用户指南

## 0.1.0

- 初版
