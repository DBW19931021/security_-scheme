#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


@dataclass
class Finding:
    check: str
    status: str
    detail: str


def markdown_files(base: Path):
    if not base.exists():
        return []
    results = []
    skipped = {".git", ".codex", ".agents", "source-vault"}
    for current, directories, files in os.walk(base):
        directories[:] = [name for name in directories if name not in skipped]
        folder = Path(current)
        results.extend(folder / name for name in files if name.endswith(".md"))
    return sorted(results)


def check_frontmatter() -> Finding:
    missing = []
    for path in markdown_files(ROOT / "docs"):
        text = path.read_text(encoding="utf-8")
        if not text.startswith("---\n") or "\n---\n" not in text[4:]:
            missing.append(path.relative_to(ROOT).as_posix())
    if missing:
        return Finding("frontmatter", "FAIL", "缺少有效头部：" + ", ".join(missing))
    return Finding("frontmatter", "PASS", f"{len(markdown_files(ROOT / 'docs'))} 个设计文档具备 frontmatter")


def source_ids() -> set[str]:
    index = ROOT / "sources/source-index.yaml"
    text = index.read_text(encoding="utf-8") if index.exists() else ""
    ids = set(re.findall(r"(?m)^\s*-\s+id:\s*[\"']?(SRC-[A-Za-z0-9_-]+)", text))
    for card in (ROOT / "sources/source-cards").glob("SRC-*.md"):
        ids.add(card.stem)
    return ids


def check_source_references() -> Finding:
    known = source_ids()
    missing: dict[str, list[str]] = {}
    for path in markdown_files(ROOT / "docs"):
        refs = set(re.findall(r"\bSRC-[0-9]{4,}\b", path.read_text(encoding="utf-8")))
        unknown = sorted(refs - known)
        if unknown:
            missing[path.relative_to(ROOT).as_posix()] = unknown
    if missing:
        detail = "; ".join(f"{p}: {','.join(v)}" for p, v in missing.items())
        return Finding("source-ids", "FAIL", detail)
    return Finding("source-ids", "PASS", f"文档 Source ID 均可解析；当前登记 {len(known)} 项")


def check_requirement_ids() -> Finding:
    path = ROOT / "requirements/security-requirements.yaml"
    text = path.read_text(encoding="utf-8") if path.exists() else ""
    ids = re.findall(r"(?m)^\s*-\s+id:\s*[\"']?(SEC-REQ-[A-Za-z0-9_-]+)", text)
    duplicates = sorted({item for item in ids if ids.count(item) > 1})
    if duplicates:
        return Finding("requirement-ids", "FAIL", "重复 Requirement ID：" + ", ".join(duplicates))
    return Finding("requirement-ids", "PASS", f"Requirement ID 唯一；当前登记 {len(ids)} 项")


def check_traceability_paths() -> Finding:
    path = ROOT / "requirements/requirement-traceability.yaml"
    text = path.read_text(encoding="utf-8") if path.exists() else ""
    candidates = re.findall(r"(?m)^\s*-?\s*(?:path:)?\s*[\"']?((?:docs|tests|evidence|decisions|openspec)/[^\"'\s]+)", text)
    missing = [item for item in candidates if not (ROOT / item).exists()]
    if missing:
        return Finding("traceability", "FAIL", "不存在的追踪路径：" + ", ".join(sorted(set(missing))))
    return Finding("traceability", "PASS", f"{len(candidates)} 个追踪路径均存在")


def child_dirs(path: Path):
    if not path.exists():
        return []
    return sorted(p for p in path.iterdir() if p.is_dir())


def check_tasks() -> Finding:
    problems = []
    for task in child_dirs(ROOT / "tasks/active"):
        for required in ("BRIEF.md", "PLAN.md", "ACCEPTANCE.md"):
            if not (task / required).exists():
                problems.append(f"{task.name}/{required}")
    for task in child_dirs(ROOT / "tasks/completed"):
        result = task / "RESULT.md"
        if not result.exists():
            problems.append(f"{task.name}/RESULT.md")
            continue
        text = result.read_text(encoding="utf-8")
        if not re.search(r"修改文件[^\n]*：\s*\S+", text):
            problems.append(f"{task.name}/RESULT.md 缺少实际修改文件")
        if not re.search(r"未提交 diff 摘要[^\n]*：\s*\S+", text):
            problems.append(f"{task.name}/RESULT.md 缺少未提交 diff 摘要")
        if not re.search(r"(?i)evidence[^\n]*：\s*\S+", text):
            problems.append(f"{task.name}/RESULT.md 缺少 Evidence")
    if problems:
        return Finding("tasks", "FAIL", "任务不完整：" + ", ".join(problems))
    active = len(child_dirs(ROOT / "tasks/active"))
    completed = len(child_dirs(ROOT / "tasks/completed"))
    return Finding("tasks", "PASS", f"任务结构完整；active={active}, completed={completed}")


def check_obsolete_sources() -> Finding:
    obsolete = set()
    for card in (ROOT / "sources/source-cards").glob("SRC-*.md"):
        text = card.read_text(encoding="utf-8")
        if re.search(r"(?im)^\s*(?:-\s*)?Status:\s*OBSOLETE\s*$", text):
            obsolete.add(card.stem)
    bad = []
    for path in markdown_files(ROOT / "docs"):
        text = path.read_text(encoding="utf-8")
        used = sorted(item for item in obsolete if item in text)
        if used:
            bad.append(f"{path.relative_to(ROOT).as_posix()}: {','.join(used)}")
    if bad:
        return Finding("obsolete-sources", "FAIL", "; ".join(bad))
    return Finding("obsolete-sources", "PASS", "正式文档未引用已识别的 OBSOLETE 来源")


def validate_yaml_subset(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    errors = []
    if not text.strip():
        return ["文件为空"]
    try:
        json.loads(text)
        return []
    except json.JSONDecodeError:
        pass
    block_indent = None
    for number, raw in enumerate(text.splitlines(), 1):
        if "\t" in raw:
            errors.append(f"line {number}: 包含 Tab")
        stripped = raw.strip()
        if not stripped or stripped.startswith("#") or stripped in {"---", "..."}:
            continue
        indent = len(raw) - len(raw.lstrip(" "))
        if block_indent is not None and indent > block_indent:
            continue
        block_indent = None
        candidate = stripped[2:].strip() if stripped.startswith("- ") else stripped
        if stripped == "-":
            continue
        if stripped.startswith("- ") and ":" not in candidate:
            continue
        if ":" not in candidate:
            errors.append(f"line {number}: 不是可识别的 key/value 或列表项")
            continue
        _, value = candidate.split(":", 1)
        if value.strip() in {"|", ">", "|-", ">-"}:
            block_indent = indent
        if value.count("[") != value.count("]") or value.count("{") != value.count("}"):
            errors.append(f"line {number}: 行内括号不平衡")
    return errors


def check_yaml() -> Finding:
    bad = []
    yaml_files = []
    skipped = {".git", ".codex", "source-vault"}
    for current, directories, files in os.walk(ROOT):
        directories[:] = [name for name in directories if name not in skipped]
        folder = Path(current)
        yaml_files.extend(folder / name for name in files if name.endswith(".yaml"))
    yaml_files.sort()
    for path in yaml_files:
        errors = validate_yaml_subset(path)
        if errors:
            bad.append(f"{path.relative_to(ROOT).as_posix()}: {'; '.join(errors)}")
    if bad:
        return Finding("yaml", "FAIL", " | ".join(bad))
    return Finding("yaml", "PASS", f"{len(yaml_files)} 个 YAML 文件通过保守子集校验")


def check_links() -> Finding:
    broken = []
    pattern = re.compile(r"\[[^\]]+\]\(([^)]+)\)")
    for path in markdown_files(ROOT):
        for target in pattern.findall(path.read_text(encoding="utf-8")):
            target = target.strip().strip("<>")
            if target.startswith(("http://", "https://", "mailto:", "#")):
                continue
            local = target.split("#", 1)[0]
            if not local:
                continue
            if not (path.parent / local).resolve().exists():
                broken.append(f"{path.relative_to(ROOT).as_posix()} -> {target}")
    if broken:
        return Finding("broken-links", "FAIL", "失效本地链接：" + ", ".join(broken))
    return Finding("broken-links", "PASS", "Markdown 本地链接均可解析")


def check_assumptions() -> Finding:
    count = 0
    for path in markdown_files(ROOT / "docs"):
        count += len(re.findall(r"(?im)^evidence_state:\s*ASSUMPTION\s*$", path.read_text(encoding="utf-8")))
    if count:
        return Finding("assumptions", "WARN", f"{count} 个初始设计入口按要求标为 ASSUMPTION，需随资料入库逐项收敛")
    return Finding("assumptions", "PASS", "未发现文档级开放 Assumption")


def check_skills() -> Finding:
    problems = []
    skills_root = ROOT / ".agents/skills"
    skills = child_dirs(skills_root)
    for skill in skills:
        text = (skill / "SKILL.md").read_text(encoding="utf-8") if (skill / "SKILL.md").exists() else ""
        match = re.match(r"^---\n(.*?)\n---", text, re.DOTALL)
        if not match:
            problems.append(f"{skill.name}: SKILL.md frontmatter 无效")
            continue
        front = match.group(1)
        name_match = re.search(r"(?m)^name:\s*([a-z0-9-]+)\s*$", front)
        desc_match = re.search(r"(?m)^description:\s*[\"']?(.+?)[\"']?\s*$", front)
        if not name_match or name_match.group(1) != skill.name:
            problems.append(f"{skill.name}: name 不匹配")
        if not desc_match or not desc_match.group(1).strip():
            problems.append(f"{skill.name}: description 缺失")
        agent_yaml = skill / "agents/openai.yaml"
        if not agent_yaml.exists() or f"${skill.name}" not in agent_yaml.read_text(encoding="utf-8"):
            problems.append(f"{skill.name}: agents/openai.yaml 缺失或 default_prompt 未引用 Skill")
        if not (skill / "templates").is_dir():
            problems.append(f"{skill.name}: templates 缺失")
    if problems:
        return Finding("skills", "FAIL", "; ".join(problems))
    return Finding("skills", "PASS", f"{len(skills)} 个项目级 Skill 结构有效")


CHECKS = {
    "frontmatter": check_frontmatter,
    "source-ids": check_source_references,
    "requirement-ids": check_requirement_ids,
    "traceability": check_traceability_paths,
    "tasks": check_tasks,
    "obsolete-sources": check_obsolete_sources,
    "yaml": check_yaml,
    "broken-links": check_links,
    "assumptions": check_assumptions,
    "skills": check_skills,
}


def write_bootstrap_report(path: Path, findings: list[Finding]) -> None:
    overall = "PASS" if not any(item.status == "FAIL" for item in findings) else "FAIL"
    files = 0
    skipped = {".git", "source-vault"}
    for current, directories, names in os.walk(ROOT):
        directories[:] = [name for name in directories if name not in skipped]
        files += len(names)
    rows = "\n".join(f"| {f.check} | {f.status} | {f.detail.replace('|', '/')} |" for f in findings)
    text = f"""# 工作空间初始化报告

    - 日期：2026-07-20（Asia/Shanghai）
    - 总体检查：`{overall}`
    - 安全工程仓库：`Z:\\code\\ngu800\\secure\\security_-scheme`
    - 公司代码仓库：`Z:\\code\\ngu800\\secure\\gsp-pmp-rmp-omp`

    ## 创建结果

    已直接在现有 `security_-scheme` Git 仓库中创建安全工程结构，没有创建嵌套 Git 仓库。当前共识别 {files} 个非 Git 文件，包括基础文档、41 个安全文档入口、OpenSpec 人工兼容模板、任务/ADR/Source Card/Requirement 模板、六个项目级 Skill 和轻量检查工具。

    ## 当前架构

    - Work 在本仓库读取资料、编写方案、维护 OpenSpec/ADR/任务并评审结果。
    - 远端 Codex 仅在批准任务下进入独立的 `gsp-pmp-rmp-omp` 仓库实现。
    - OpenSpec 位于 `openspec/`；用户已执行 init，当前 schema 为 `spec-driven`，CLI 版本未知。
    - Evidence 位于 `evidence/`，大文件可以保存不可变路径和哈希引用。
    - 本仓库是正式事实、问题、决策和追踪关系的唯一来源。

    ## 识别结果

    - `security_-scheme`：初始化前为干净 `master`，远端为 `git@github.com:DBW19931021/security_-scheme.git`，原提交树为空。
    - `gsp-pmp-rmp-omp`：`master@08b29c7b7a29...`，远端为公司 GitLab HTTP 地址，盘点时存在大量既有未提交修改。
    - 资料入库：已登记 15 份 Vendor PDF、2 份 NGU800P 初步方案基线、1 份 OSR eHSM 代码快照和 1 套历史 Review 文档集；ADR-0001/ADR-0002 已确认方案权威层级、Vendor 边界、PDF 基线控制和冲突升级机制。
    - 可用工具：Git 2.53.0、Python 3.12.13；OpenSpec 已由用户初始化，但当前 Codex PATH 不包含 CLI。

    ## 自动检查

    | 检查 | 状态 | 结果 |
    |---|---|---|
    {rows}

    ## 未完成项和风险

    - OpenSpec 已初始化；当前 Codex PATH 无 CLI，因此版本和原生 `openspec validate` 结果待在实际 OpenSpec 终端补充。
    - `skill-creator` 自带 `quick_validate.py` 因 bundled Python 缺少 PyYAML 无法运行；已由仓库内无第三方依赖检查完成等价结构校验。
    - 公司代码仓库既有修改的归属和保护范围待确认；允许默认分支工作区修改，但不得覆盖无关改动或执行任何 Git 提交。
    - Vendor 资料/代码适用的 eHSM/Core 版本、SRC-0018 正式交付来源，以及两份内部方案的 Owner/批准版本仍待确认；Vendor 文档在本工程中不保密。
    - SRC-0018 的 Bootloader 目录后缀与 `SW_changelist.md` 不一致；SRC-0019 中 1 条候选发现已复核并完成裁决，其余 16 条待重新验证。
    - eHSM BL/Host自检位图差异已按Vendor回复和ADR-0003关闭：采用Bootloader定义，bit18/`0x40000`=`TRNG`，Host定义错误，bit19/`0x80000`保持unknown/reserved；原始Vendor回复材料待补录。
    - 原始大文件进入 Git 前需确认保密和 Git LFS 策略。
    - Git 对网络共享报告所有者不一致；本次没有修改全局 `safe.directory`。

    ## 优先下一步

    1. 确认 Vendor 资料/代码适用的 eHSM/Core 版本、SRC-0018 交付来源和两份内部方案的 Owner/批准版本。
    2. 核对已批准密钥轮换策略是否完整进入芯片安全软件方案，并重新验证历史候选项；发现明显冲突立即提交负责人裁决。
    3. 对决定采纳的 Vendor 建议先更新受控 amendment/芯片安全软件方案，再建立 issue/OpenSpec/Evidence 闭环。
    """
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(line[4:] if line.startswith("    ") else line for line in text.splitlines()).strip() + "\n", encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="检查 SoC 安全工程仓库的一致性")
    parser.add_argument("--only", choices=sorted(CHECKS))
    parser.add_argument("--write-bootstrap-report", action="store_true")
    args = parser.parse_args()
    names = [args.only] if args.only else list(CHECKS)
    findings = [CHECKS[name]() for name in names]
    for item in findings:
        print(f"[{item.status}] {item.check}: {item.detail}")
    if args.write_bootstrap_report:
        write_bootstrap_report(ROOT / "outputs/bootstrap-report.md", findings)
        print("[PASS] report: outputs/bootstrap-report.md 已更新")
    return 1 if any(item.status == "FAIL" for item in findings) else 0


if __name__ == "__main__":
    raise SystemExit(main())
