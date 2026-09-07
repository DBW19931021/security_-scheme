#!/usr/bin/env python3
# -*- coding:utf-8 -*-

"""检查最大栈深度是否超过链接脚本中定义的 __STACK_SIZE 的指定百分比。

该脚本通过解析 GCC 生成的 .su (stack usage) 和 .cgraph (call graph) 文件，
构建函数调用图，计算从根函数出发的最大栈深度，并与链接脚本中定义的
__STACK_SIZE 进行对比。

用法:
    python3 check_stack_size.py --build-dir <build_dir> --linker-script <ld_file>
"""

import argparse
import os
import re
import sys


class CallGraph:
    """函数调用图，用于计算最大栈深度。"""

    def __init__(self):
        self.funcs = {}

    def get_function(self, name):
        """获取或创建一个函数节点。"""
        if name not in self.funcs:
            self.funcs[name] = Function(name)
        return self.funcs[name]

    def add_call(self, caller_name, callee_name):
        """添加一条调用关系。"""
        caller = self.get_function(caller_name)
        callee = self.get_function(callee_name)
        caller.callees[callee_name] = callee
        callee.callers[caller_name] = caller

    def get_max_stack_from_root(self, root="main"):
        """从指定根函数出发，计算最深调用栈的总大小及完整调用路径。

        使用记忆化 DFS，时间复杂度 O(V+E)。

        Returns:
            (max_size, path_string): 最大栈大小和最深调用路径描述
        """
        memo = {}
        in_stack = set()

        def dfs(name):
            if name in memo:
                return memo[name][0]
            if name not in self.funcs:
                return 0
            if name in in_stack:
                print(f"  warning: recursion detected at '{name}', skipping")
                return 0

            func = self.funcs[name]
            in_stack.add(name)

            best_callee = None
            max_callee_stack = 0
            for callee_name in func.callees:
                s = dfs(callee_name)
                if s > max_callee_stack:
                    max_callee_stack = s
                    best_callee = callee_name

            in_stack.remove(name)
            result = func.stack + max_callee_stack
            memo[name] = (result, best_callee)
            return result

        total = dfs(root)

        # 回溯最深调用路径
        path_parts = []
        cur = root
        while cur is not None and cur in memo:
            func = self.funcs[cur]
            path_parts.append(f"{cur}/{func.stack}")
            cur = memo[cur][1]

        return total, " -> ".join(path_parts)


class Function:
    """调用图中的函数节点。"""

    def __init__(self, name):
        self.name = name
        self.stack = 0
        self.callers = {}
        self.callees = {}

    def set_stack_size(self, size):
        """设置函数栈用量，取最大值。"""
        self.stack = max(self.stack, size)


def collect_files(build_dir, extension):
    """遍历构建目录收集指定扩展名的文件。"""
    result = []
    for dirpath, _dirnames, filenames in os.walk(build_dir):
        for fname in filenames:
            if fname.endswith(extension):
                result.append(os.path.join(dirpath, fname))
    return result


def parse_su_files(cgraph, file_list):
    """解析 .su 文件，提取每个函数的栈用量。

    .su 文件格式: <file>:<line>:<col>:<function_name>\t<size>\t<type>
    """
    for filepath in file_list:
        with open(filepath, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                cols = line.split("\t")
                if len(cols) < 2:
                    continue
                # 提取函数名: 最后一个 ':' 之后的部分
                text = cols[0].split(":")
                if len(text) < 4:
                    continue
                func_name = text[-1]
                try:
                    stack_size = int(cols[1])
                except ValueError:
                    continue
                cgraph.get_function(func_name).set_stack_size(stack_size)
                # GCC 克隆函数在 .su 中不带数字后缀（如 func.constprop），
                # 但在 .cgraph 中带 .0 后缀（如 func.constprop.0）。
                # 同时注册带 .0 后缀的变体，确保栈用量能关联到调用图节点。
                normalized = re.sub(
                    r"\.(constprop|isra|part)(?!\.0)", r".\1.0", func_name
                )
                if normalized != func_name:
                    cgraph.get_function(normalized).set_stack_size(stack_size)


def parse_cgraph_files(cgraph, file_list):
    """解析 .cgraph 文件，提取函数调用关系。

    cgraph 文件包含多个 section（Initial Symbol table、Optimized Symbol table 等），
    仅解析 Optimized Symbol table section，因为它反映优化后的真实调用图。
    Initial section 中的调用边可能包含已被内联消除的关系，解析它们会导致
    内联函数的栈帧被双重计算。

    注意：
    1. 在 section 边界重置 current_func，防止跨 section 错误归属
    2. 不使用反向引用匹配函数名，因为 GCC 优化克隆函数名前后可能不一致
       （如 func.part.0.isra.0/384 vs (func.part.0.isra)）
    """
    for filepath in file_list:
        current_func = None
        in_optimized = False
        with open(filepath, "r") as f:
            for line in f:
                # 检测 Optimized Symbol table section 开始
                if "Optimized Symbol table:" in line:
                    in_optimized = True
                    current_func = None
                    continue

                # 跳过 Optimized section 之前的内容
                if not in_optimized:
                    continue

                # 匹配函数定义行: name/number (name_variant)
                match = re.match(r"([\w.]+)/\d+ \(", line)
                if match:
                    current_func = match.group(1)
                    continue

                # section 边界（行首非空白、非函数定义）重置 current_func
                if line and not line[0].isspace() and line.strip():
                    current_func = None
                    continue

                # 匹配调用行
                if line.startswith("  Calls:") and len(line) > 10 and current_func:
                    for callee in line[9:].split(" "):
                        callee = callee.strip()
                        if "/" in callee:
                            callee_name = callee[: callee.find("/")]
                            if callee_name:
                                cgraph.add_call(current_func, callee_name)


def parse_stack_size_from_ld(ld_path):
    """从链接脚本中解析 __STACK_SIZE 的值。

    Returns:
        栈大小（字节），解析失败则返回 None
    """
    with open(ld_path, "r") as f:
        content = f.read()

    match = re.search(r"__STACK_SIZE\s*=\s*(0x[0-9a-fA-F]+|\d+)\s*;", content)
    if match:
        value_str = match.group(1)
        return int(value_str, 0)
    return None


def main():
    parser = argparse.ArgumentParser(
        description="检查最大栈深度是否超过 __STACK_SIZE 的指定百分比"
    )
    parser.add_argument(
        "--build-dir",
        required=True,
        help="构建目录路径，用于遍历 .su 和 .cgraph 文件",
    )
    parser.add_argument(
        "--linker-script",
        required=True,
        help="链接脚本路径，用于提取 __STACK_SIZE",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=90,
        help="栈用量阈值百分比（默认 90）",
    )
    parser.add_argument(
        "--root",
        default="main",
        help="调用图根函数名（默认 main）",
    )
    parser.add_argument(
        "--isr-overhead",
        type=int,
        default=0,
        help="中断处理函数的额外栈开销（字节），会加到最大栈深度上",
    )
    args = parser.parse_args()

    if not (0 < args.threshold <= 100):
        print(f"ERROR: --threshold must be between 1 and 100, got {args.threshold}")
        sys.exit(1)

    # 1. 解析链接脚本中的 __STACK_SIZE
    if not os.path.isfile(args.linker_script):
        print(f"ERROR: linker script not found: {args.linker_script}")
        sys.exit(1)
    stack_size = parse_stack_size_from_ld(args.linker_script)
    if stack_size is None:
        print("ERROR: failed to parse __STACK_SIZE from linker script")
        sys.exit(1)
    if stack_size == 0:
        print("ERROR: __STACK_SIZE is 0, cannot perform stack check")
        sys.exit(1)

    # 2. 收集 .su 和 .cgraph 文件
    su_files = collect_files(args.build_dir, ".su")
    cgraph_files = collect_files(args.build_dir, ".cgraph")

    if not su_files:
        print("ERROR: no .su files found in build directory")
        sys.exit(1)
    if not cgraph_files:
        print("ERROR: no .cgraph files found in build directory")
        sys.exit(1)

    # 3. 构建调用图
    cgraph = CallGraph()
    parse_su_files(cgraph, su_files)
    parse_cgraph_files(cgraph, cgraph_files)

    if args.root not in cgraph.funcs:
        print(f"ERROR: root function '{args.root}' not found in call graph")
        sys.exit(1)

    # 4. 计算最大栈深度
    max_stack, path = cgraph.get_max_stack_from_root(args.root)

    # 5. 对比并输出结果
    effective_max = max_stack + args.isr_overhead
    usage_percent = effective_max * 100.0 / stack_size
    threshold_bytes = stack_size * args.threshold // 100

    print(f"__STACK_SIZE  : {stack_size} bytes (0x{stack_size:X})")
    print(f"max stack used: {max_stack} bytes")
    if args.isr_overhead > 0:
        print(f"ISR overhead  : {args.isr_overhead} bytes")
        print(f"effective max : {effective_max} bytes")
    print(f"usage         : {usage_percent:.1f}% (threshold: {args.threshold}%)")
    print(f"deepest path  : {path}")

    if effective_max > threshold_bytes:
        print(
            f"\nERROR: stack usage ({effective_max} bytes) exceeds "
            f"{args.threshold}% of __STACK_SIZE ({threshold_bytes} bytes)"
        )
        sys.exit(1)
    else:
        print("\nPASS: stack usage is within limits")
        sys.exit(0)


if __name__ == "__main__":
    main()
