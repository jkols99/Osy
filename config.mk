

TARGET = mipsel-unknown-linux-gnu
TOOLCHAIN_DIR = /usr/

CC = $(TOOLCHAIN_DIR)/bin/$(TARGET)-gcc
LD = $(TOOLCHAIN_DIR)/bin/$(TARGET)-ld
OBJCOPY = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objcopy
OBJDUMP = $(TOOLCHAIN_DIR)/bin/$(TARGET)-objdump
GDB = $(TOOLCHAIN_DIR)/bin/$(TARGET)-gdb

KERNEL_TEST_SOURCES = tests/sem/prodcons/test.c
KERNEL_EXTRA_CFLAGS = -DKERNEL_DEBUG -DKERNEL_TEST

