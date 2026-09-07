#!/bin/bash

# 编译脚本：通过参数修改-DPORT配置

# 默认PORT值
DEFAULT_PORT="port/cm3"

# 帮助信息函数
show_help() {
    echo "用法: $0 [选项] [PORT值]"
    echo "选项:"
    echo "  -h, --help      显示帮助信息"
    echo "  -c, --clean     清理构建目录后再编译"
    echo "  -b, --build     仅执行构建(不重新cmake配置)"
    echo ""
    echo "示例:"
    echo "  $0 port/cm3         # 使用PORT=port/cm3配置并编译"
    echo "  $0 -c port/m130      # 清理后使用PORT=port/m130配置并编译"
    echo "  $0 -b                # 仅执行构建(使用上次的配置)"
    exit 0
}

# 初始化变量
PORT="$DEFAULT_PORT"
CLEAN=false
BUILD_ONLY=false

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            show_help
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -b|--build)
            BUILD_ONLY=true
            shift
            ;;
        *)
            # 非选项参数视为PORT值
            PORT="$1"
            shift
            ;;
    esac
done

# 检查PORT值是否有效
VALID_PORTS=("port/cm3" "port/m130")
IS_VALID=false
for p in "${VALID_PORTS[@]}"; do
    if [[ "$PORT" == "$p" ]]; then
        IS_VALID=true
        break
    fi
done

if [[ "$IS_VALID" == false ]]; then
    echo "错误: 无效的PORT值 '$PORT'，有效值为: ${VALID_PORTS[*]}"
    exit 1
fi

# 创建build目录（如果不存在）
BUILD_DIR="build"
if [[ ! -d "$BUILD_DIR" ]]; then
    mkdir -p "$BUILD_DIR"
fi

# 清理构建目录
if [[ "$CLEAN" == true ]]; then
    echo "清理构建目录 $BUILD_DIR..."
    rm -rf "$BUILD_DIR"/*
fi

# 进入构建目录
cd "$BUILD_DIR"

# 执行cmake配置（如果不是仅构建模式）
if [[ "$BUILD_ONLY" == false ]]; then
    echo "使用PORT=$PORT配置项目..."
    cmake -DPORT=$PORT .. -G "Unix Makefiles"
    if [[ $? -ne 0 ]]; then
        echo "错误: cmake配置失败"
        exit 1
    fi
fi

# 执行构建
echo "开始构建..."
make
if [[ $? -ne 0 ]]; then
    echo "错误: 构建失败"
    exit 1
fi

echo "构建成功! 输出文件位于 $BUILD_DIR 目录"

exit 0