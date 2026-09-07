# 自动检查工具

统一入口：`python tools/scripts/project_check.py`。脚本只读取仓库内容；只有显式传入 `--write-bootstrap-report` 时才更新初始化报告。

当前实现不依赖第三方 Python 包。YAML 检查采用保守子集校验；安装 PyYAML 后可在后续迭代中升级为完整语义解析。
