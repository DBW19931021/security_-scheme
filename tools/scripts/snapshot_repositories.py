#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path


def git(git_exe: str, repo: Path, *args: str) -> str:
    cmd = [git_exe, "-c", f"safe.directory={repo.as_posix()}", "-C", str(repo), *args]
    return subprocess.run(cmd, check=True, capture_output=True, text=True, encoding="utf-8", errors="replace").stdout.strip()


def main() -> int:
    parser = argparse.ArgumentParser(description="只读快照一个或多个 Git 仓库")
    parser.add_argument("repositories", nargs="+", type=Path)
    parser.add_argument("--git", default="git")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = []
    for repo in args.repositories:
        resolved = repo.resolve()
        status = git(args.git, resolved, "status", "--porcelain=v1", "-uno")
        result.append({
            "path": str(resolved),
            "remote": git(args.git, resolved, "config", "--get", "remote.origin.url"),
            "branch": git(args.git, resolved, "branch", "--show-current"),
            "commit": git(args.git, resolved, "rev-parse", "HEAD"),
            "tracked_modification_count": len(status.splitlines()) if status else 0,
            "dirty": bool(status),
        })
    text = json.dumps({"repositories": result}, ensure_ascii=False, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8", newline="\n")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
