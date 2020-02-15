BINARY := sorgente
KERNEL := /lib/modules/$(shell uname -r)/build

KMOD_DIR := $(shell pwd)
TARGET_PATH := /lib/modules/$(shell uname -r)/kernel/drivers/char

obj-m := $(BINARY).o

$(BINARY).ko: prototipi.h
	make -C $(KERNEL) M=$(KMOD_DIR) modules
	

