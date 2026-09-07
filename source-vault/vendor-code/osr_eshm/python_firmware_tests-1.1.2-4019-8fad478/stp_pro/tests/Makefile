
# 检测操作系统
UNAME_S := $(shell uname -s)
DEST_NAME := stp_server

# 根据系统设置变量
ifeq ($(UNAME_S),Linux)
    HAL_SRC = linux_uart_hal.c
    DEST = $(DEST_NAME)
    LIBS = -lrt
else
    # 默认假设为Windows环境
    HAL_SRC = win_uart_hal.c
    DEST = $(DEST_NAME).exe
    LIBS = 
endif

# 源文件列表
CORE_SRC = $(wildcard ../c/*.c)
TEST_SRC = test_main.c $(HAL_SRC)
SRC = $(CORE_SRC) $(TEST_SRC)

# 依赖文件
CORE_DEPS = $(wildcard ../c/*.h) $(wildcard ../c/*.c)
TEST_DEPS = $(wildcard *.h) $(wildcard *.c)
DEPS = $(CORE_DEPS) $(TEST_DEPS)

# 编译选项
CFLAGS = -O0 -g -Wall -I ../c
LDFLAGS = $(LIBS)

# 主目标
$(DEST): $(DEPS)
	@echo "Building for $(UNAME_S) using $(HAL_SRC)"
	gcc $(CFLAGS) $(SRC) $(LDFLAGS) -o $@

# 清理目标
clean:
	-rm $(DEST_NAME)
	-rm $(DEST_NAME).exe
	-rm -rf __pycache__

# 显示当前配置
info:
	@echo "Operating System: $(UNAME_S)"
	@echo "HAL Source: $(HAL_SRC)" 
	@echo "Target: $(DEST)"
	@echo "Libraries: $(LIBS)"
	@echo "Source Files: $(SRC)"

.PHONY: clean info
