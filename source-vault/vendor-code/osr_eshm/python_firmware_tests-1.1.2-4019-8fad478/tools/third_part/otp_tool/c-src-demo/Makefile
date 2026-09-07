# user Makefile demo

# windows program needs to link with ntdll library
ifeq ($(OS),Windows_NT)
	DEST := test.exe
	EXTRA_LIB := -lntdll
	LIB_DIR ?= ../x86_64-windows/lib
	INC_DIR ?= ../x86_64-windows/include
else
	DEST := test
	EXTRA_LIB :=
	LIB_DIR ?= ../x86_64-linux/lib
	INC_DIR ?= ../x86_64-linux/include
endif

DEST_DIR ?= .

$(DEST_DIR)/$(DEST): test.c
	gcc $< -Wall -g -I $(LIB_DIR) -L $(LIB_DIR) -lotptool_clib $(EXTRA_LIB) -o $@
	
