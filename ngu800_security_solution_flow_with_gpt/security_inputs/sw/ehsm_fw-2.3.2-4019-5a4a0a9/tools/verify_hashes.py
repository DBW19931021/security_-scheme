#!/usr/bin/env python3
# -*- coding:utf-8  -*-
"""校验文件 hash 完整性，与 pack_code.py 生成的 file_hashes.sha256 配合使用。"""

import hashlib
import sys
import argparse as ap
from pathlib import Path

MANIFEST_NAME = "file_hashes.sha256"


def normalize_text(content: bytes) -> bytes:
    """将文本内容的换行符归一化为 LF，与 pack_code.py 生成端逻辑一致。"""
    return content.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def compute_hash(filepath: Path, file_type: str) -> str:
    """根据文件类型计算 SHA-256 hash。

    Args:
        filepath: 文件路径
        file_type: "T" 表示文本文件（归一化后计算），"B" 表示二进制文件（原始内容计算）
    """
    content = filepath.read_bytes()
    if file_type == "T":
        content = normalize_text(content)
    return hashlib.sha256(content).hexdigest()


def parse_manifest(manifest_path: Path) -> list[tuple[str, str, str]]:
    """解析 hash 清单文件。

    Returns:
        列表，每项为 (hash, type, filepath)
    """
    entries: list[tuple[str, str, str]] = []
    text = manifest_path.read_text(encoding="utf-8")
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("  ", 2)
        if len(parts) != 3:
            print(f"[WARN] 无法解析行: {line}", file=sys.stderr)
            continue
        entries.append((parts[0], parts[1], parts[2]))
    return entries


def verify(root_dir: Path, strict: bool) -> bool:
    """执行校验。

    Returns:
        True 表示全部通过，False 表示有失败
    """
    manifest_path = root_dir / MANIFEST_NAME
    if not manifest_path.exists():
        print(f"错误: 未找到清单文件 {manifest_path}", file=sys.stderr)
        return False

    entries = parse_manifest(manifest_path)
    if not entries:
        print("错误: 清单文件为空或格式不正确", file=sys.stderr)
        return False

    ok_count = 0
    fail_count = 0
    miss_count = 0
    new_count = 0

    print("Verifying file hashes...")

    manifest_paths: set[str] = set()
    for expected_hash, file_type, rel_path in entries:
        manifest_paths.add(rel_path)
        filepath = root_dir / Path(rel_path)

        if not filepath.exists():
            print(f"[MISS] {rel_path}")
            miss_count += 1
            continue

        actual_hash = compute_hash(filepath, file_type)
        if actual_hash == expected_hash:
            print(f"[OK]   {rel_path}")
            ok_count += 1
        else:
            print(f"[FAIL] {rel_path}")
            fail_count += 1

    if strict:
        for filepath in sorted(root_dir.rglob("*")):
            if not filepath.is_file() or filepath.is_symlink():
                continue
            if filepath == manifest_path:
                continue
            rel_path = filepath.relative_to(root_dir).as_posix()
            if rel_path not in manifest_paths:
                print(f"[NEW]  {rel_path}")
                new_count += 1

    print()
    summary_parts = [f"{ok_count} OK", f"{fail_count} FAILED", f"{miss_count} MISSING"]
    if strict:
        summary_parts.append(f"{new_count} NEW")
    print(f"Verification complete: {', '.join(summary_parts)}")

    passed = fail_count == 0 and miss_count == 0
    if strict:
        passed = passed and new_count == 0

    print(f"Result: {'PASSED' if passed else 'FAILED'}")
    return passed


def cli() -> ap.Namespace:
    parser = ap.ArgumentParser(description="校验文件 hash 完整性")
    parser.add_argument(
        "--dir",
        default=None,
        help="指定校验目录，默认为本脚本所在目录的上级目录",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="严格模式：同时检测清单中没有但文件系统中多出的文件",
    )
    return parser.parse_args()


def main():
    args = cli()

    if args.dir:
        root_dir = Path(args.dir)
    else:
        root_dir = Path(__file__).parent.parent

    root_dir = root_dir.resolve()

    if not root_dir.is_dir():
        print(f"错误: 目录不存在 {root_dir}", file=sys.stderr)
        sys.exit(1)

    passed = verify(root_dir, strict=args.strict)
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
