# Workflow Rules

1. 不得跳过 constraints。
2. 不得跳过 baseline。
3. 不得跳过 implementation design。
4. 不得在目录结构变化后不更新 skill / prompts / README。
5. 不得把假设写成 [CONFIRMED]。
6. 不得静默改变文件职责边界。
7. 凡是影响两个以上文件、影响 baseline、影响 boot/key/cert/attestation/debug/interface/manufacturing 任一安全主路径的变更，必须先生成 Change Request，不得直接改正文。
8. Codex 不拥有安全架构裁决权；ChatGPT / 用户 / security owner / accepted CR / signed-off baseline 才能冻结安全结论。
9. 没有批准来源时，只能写 `[PROPOSED]`、`[ASSUMED]` 或 `[TBD]`，不得新增或升级 `[CONFIRMED]`。
10. Codex 生成的正文不能反向作为 source of truth；必须追溯到输入、CR、decision_log、baseline 或官方资料。
