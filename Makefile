# MaxRegnerOS Complete Build System Makefile
# Comprehensive build orchestration for MaxRegnerOS
# Version: 1.0.0

# Configuration
OS_NAME := MaxRegnerOS
OS_VERSION := 1.0.0
BUILD_DIR := build
ISO_NAME := $(OS_NAME)-$(OS_VERSION).iso
KERNEL_VERSION := 6.6.0
ARCH := x86_64

# Build tools
CC := gcc
MAKE := make
PYTHON := python3
SHELL := /bin/bash

# Parallel build jobs
JOBS := $(shell nproc)

# Colors for output
RED := \033[0;31m
GREEN := \033[0;32m
BLUE := \033[0;34m
YELLOW := \033[1;33m
NC := \033[0m # No Color

# Default target
.PHONY: all
all: info dependencies build test package

# Display build information
.PHONY: info
info:
	@echo -e "$(BLUE)🚀 MaxRegnerOS Build System$(NC)"
	@echo -e "$(BLUE)=============================$(NC)"
	@echo -e "OS Name:      $(OS_NAME)"
	@echo -e "Version:      $(OS_VERSION)"
	@echo -e "Architecture: $(ARCH)"
	@echo -e "Build Jobs:   $(JOBS)"
	@echo -e "Build Dir:    $(BUILD_DIR)"
	@echo -e "Target ISO:   $(ISO_NAME)"
	@echo ""

# Check and install dependencies
.PHONY: dependencies
dependencies:
	@echo -e "$(YELLOW)📦 Checking build dependencies...$(NC)"
	@./scripts/check_dependencies.sh || (echo -e "$(RED)❌ Dependencies check failed$(NC)" && exit 1)
	@echo -e "$(GREEN)✅ All dependencies satisfied$(NC)"

# Clean build directory
.PHONY: clean
clean:
	@echo -e "$(YELLOW)🧹 Cleaning build directory...$(NC)"
	@rm -rf $(BUILD_DIR)
	@rm -f $(OS_NAME)-*.iso
	@rm -f $(OS_NAME)-*.sha256
	@rm -f MaxRegnerOS_QA_Report_*.json
	@echo -e "$(GREEN)✅ Clean completed$(NC)"

# Create build directory structure
.PHONY: setup
setup:
	@echo -e "$(YELLOW)📁 Setting up build directories...$(NC)"
	@mkdir -p $(BUILD_DIR)/{iso,rootfs,kernel,desktop,applications,tools}
	@mkdir -p $(BUILD_DIR)/iso/{boot,live}
	@mkdir -p $(BUILD_DIR)/rootfs/{bin,sbin,usr,var,tmp,home,root,etc,dev,proc,sys}
	@echo -e "$(GREEN)✅ Directory structure created$(NC)"

# Download and prepare kernel source
.PHONY: kernel-source
kernel-source: setup
	@echo -e "$(YELLOW)🔽 Downloading kernel source...$(NC)"
	@cd $(BUILD_DIR)/kernel && \
	if [ ! -d "linux" ]; then \
		wget -q https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$(KERNEL_VERSION).tar.xz && \
		tar -xf linux-$(KERNEL_VERSION).tar.xz && \
		mv linux-$(KERNEL_VERSION) linux; \
	fi
	@echo -e "$(GREEN)✅ Kernel source ready$(NC)"

# Configure kernel
.PHONY: kernel-config
kernel-config: kernel-source
	@echo -e "$(YELLOW)⚙️  Configuring kernel...$(NC)"
	@cd $(BUILD_DIR)/kernel/linux && \
	cp ../../../configs/maxregner_kernel.config .config && \
	make olddefconfig
	@echo -e "$(GREEN)✅ Kernel configured$(NC)"

# Build kernel
.PHONY: kernel
kernel: kernel-config
	@echo -e "$(YELLOW)🔨 Building kernel (this may take a while)...$(NC)"
	@cd $(BUILD_DIR)/kernel/linux && \
	make -j$(JOBS) bzImage modules
	@cp $(BUILD_DIR)/kernel/linux/arch/x86/boot/bzImage $(BUILD_DIR)/iso/boot/vmlinuz
	@echo -e "$(GREEN)✅ Kernel build completed$(NC)"

# Build desktop environment
.PHONY: desktop
desktop: setup
	@echo -e "$(YELLOW)🖥️  Building desktop environment...$(NC)"
	@mkdir -p $(BUILD_DIR)/desktop/src
	@cd $(BUILD_DIR)/desktop/src && \
	cp ../../../src/desktop/* . && \
	$(CC) -o maxregner_wm maxregner_wm.c -lX11 && \
	$(CC) -o maxregner_panel maxregner_panel.c -lX11 -lXft && \
	$(CC) -o maxregner_launcher maxregner_launcher.c -lX11 && \
	mkdir -p ../../rootfs/usr/bin && \
	cp maxregner_* ../../rootfs/usr/bin/
	@echo -e "$(GREEN)✅ Desktop environment built$(NC)"

# Build applications
.PHONY: applications
applications: setup
	@echo -e "$(YELLOW)📱 Building application suite...$(NC)"
	@mkdir -p $(BUILD_DIR)/applications/src
	@cd $(BUILD_DIR)/applications/src && \
	cp ../../../src/applications/* . && \
	$(CC) -o maxregner_files maxregner_files.c -lX11 && \
	$(CC) -o maxregner_edit maxregner_edit.c -lX11 && \
	$(CC) -o maxregner_browser maxregner_browser.c -lX11 && \
	$(CC) -o maxregner_terminal maxregner_terminal.c -lX11 && \
	$(CC) -o maxregner_media maxregner_media.c -lX11 && \
	$(CC) -o maxregner_settings maxregner_settings.c -lX11 && \
	mkdir -p ../../rootfs/usr/bin && \
	cp maxregner_* ../../rootfs/usr/bin/
	@echo -e "$(GREEN)✅ Applications built$(NC)"

# Create root filesystem
.PHONY: rootfs
rootfs: desktop applications
	@echo -e "$(YELLOW)📂 Creating root filesystem...$(NC)"
	@cd $(BUILD_DIR)/rootfs && \
	cp ../../../configs/init . && \
	chmod +x init && \
	mkdir -p usr/bin && \
	cp ../../../scripts/startx usr/bin/ && \
	chmod +x usr/bin/startx && \
	mkdir -p usr/share/applications && \
	cp ../../../configs/desktop_entries/*.desktop usr/share/applications/ && \
	mkdir -p etc/maxregner && \
	cp ../../../maxregneros_config.json etc/maxregner/ && \
	mkdir -p usr/share/backgrounds && \
	cp ../../../assets/wallpapers/* usr/share/backgrounds/ 2>/dev/null || true
	@echo -e "$(GREEN)✅ Root filesystem created$(NC)"

# Install system tools
.PHONY: system-tools
system-tools: rootfs
	@echo -e "$(YELLOW)🔧 Installing system tools...$(NC)"
	@mkdir -p $(BUILD_DIR)/rootfs/usr/bin
	@cp tools/mrpkg_manager.py $(BUILD_DIR)/rootfs/usr/bin/mrpkg
	@cp tools/maxregner_system_monitor.py $(BUILD_DIR)/rootfs/usr/bin/maxregner_monitor
	@chmod +x $(BUILD_DIR)/rootfs/usr/bin/mrpkg
	@chmod +x $(BUILD_DIR)/rootfs/usr/bin/maxregner_monitor
	@echo -e "$(GREEN)✅ System tools installed$(NC)"

# Create initramfs
.PHONY: initramfs
initramfs: rootfs system-tools
	@echo -e "$(YELLOW)📦 Creating initramfs...$(NC)"
	@cd $(BUILD_DIR)/rootfs && \
	find . | cpio -o -H newc | gzip > ../iso/boot/initrd.img
	@echo -e "$(GREEN)✅ Initramfs created$(NC)"

# Create squashfs filesystem
.PHONY: squashfs
squashfs: rootfs system-tools
	@echo -e "$(YELLOW)🗜️  Creating compressed filesystem...$(NC)"
	@mksquashfs $(BUILD_DIR)/rootfs $(BUILD_DIR)/iso/live/filesystem.squashfs -comp xz -Xbcj x86
	@echo -e "$(GREEN)✅ Compressed filesystem created$(NC)"

# Create bootloader configuration
.PHONY: bootloader
bootloader: setup
	@echo -e "$(YELLOW)🥾 Configuring bootloader...$(NC)"
	@mkdir -p $(BUILD_DIR)/iso/boot/grub
	@cp configs/grub.cfg $(BUILD_DIR)/iso/boot/grub/
	@echo -e "$(GREEN)✅ Bootloader configured$(NC)"

# Create ISO image
.PHONY: iso
iso: kernel initramfs squashfs bootloader
	@echo -e "$(YELLOW)💿 Creating ISO image...$(NC)"
	@xorriso -as mkisofs \
		-iso-level 3 \
		-full-iso9660-filenames \
		-volid "$(OS_NAME)" \
		-eltorito-boot boot/grub/grub.cfg \
		-eltorito-catalog boot/grub/boot.cat \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		-output "$(ISO_NAME)" \
		$(BUILD_DIR)/iso/
	@echo -e "$(GREEN)✅ ISO created: $(ISO_NAME)$(NC)"

# Calculate checksums
.PHONY: checksums
checksums: iso
	@echo -e "$(YELLOW)🔐 Calculating checksums...$(NC)"
	@sha256sum $(ISO_NAME) > $(ISO_NAME).sha256
	@md5sum $(ISO_NAME) > $(ISO_NAME).md5
	@echo -e "$(GREEN)✅ Checksums calculated$(NC)"

# Main build target
.PHONY: build
build: setup kernel desktop applications rootfs system-tools iso checksums
	@echo -e "$(GREEN)🎉 MaxRegnerOS build completed successfully!$(NC)"
	@echo -e "$(GREEN)📁 ISO file: $(ISO_NAME)$(NC)"
	@echo -e "$(GREEN)💾 Size: $$(du -h $(ISO_NAME) | cut -f1)$(NC)"

# Run quality assurance tests
.PHONY: test
test: build
	@echo -e "$(YELLOW)🧪 Running quality assurance tests...$(NC)"
	@$(PYTHON) tools/maxregneros_qa_suite.py $(BUILD_DIR)

# Package for distribution
.PHONY: package
package: test
	@echo -e "$(YELLOW)📦 Packaging for distribution...$(NC)"
	@mkdir -p dist
	@cp $(ISO_NAME) dist/
	@cp $(ISO_NAME).sha256 dist/
	@cp $(ISO_NAME).md5 dist/
	@cp MaxRegnerOS_README.md dist/README.md
	@cp MaxRegnerOS_QA_Report_*.json dist/ 2>/dev/null || true
	@cd dist && tar -czf $(OS_NAME)-$(OS_VERSION)-complete.tar.gz *
	@echo -e "$(GREEN)✅ Distribution package created in dist/$(NC)"

# Quick build (skip tests)
.PHONY: quick
quick: setup kernel desktop applications rootfs system-tools iso
	@echo -e "$(GREEN)⚡ Quick build completed!$(NC)"

# Development build (minimal)
.PHONY: dev
dev: setup desktop applications rootfs
	@echo -e "$(GREEN)🔧 Development build completed!$(NC)"

# Install to USB device (requires root)
.PHONY: install-usb
install-usb: iso
	@echo -e "$(YELLOW)💾 Installing to USB device...$(NC)"
	@if [ -z "$(USB_DEVICE)" ]; then \
		echo -e "$(RED)❌ Please specify USB device: make install-usb USB_DEVICE=/dev/sdX$(NC)"; \
		exit 1; \
	fi
	@echo -e "$(RED)⚠️  This will DESTROY all data on $(USB_DEVICE)!$(NC)"
	@read -p "Continue? (y/N): " confirm && [ "$$confirm" = "y" ]
	@sudo dd if=$(ISO_NAME) of=$(USB_DEVICE) bs=4M status=progress oflag=sync
	@echo -e "$(GREEN)✅ USB installation completed$(NC)"

# Virtual machine test
.PHONY: vm-test
vm-test: iso
	@echo -e "$(YELLOW)🖥️  Starting virtual machine test...$(NC)"
	@qemu-system-x86_64 \
		-cdrom $(ISO_NAME) \
		-m 2048 \
		-enable-kvm \
		-vga std \
		-netdev user,id=net0 \
		-device e1000,netdev=net0

# Docker build environment
.PHONY: docker-build
docker-build:
	@echo -e "$(YELLOW)🐳 Building in Docker container...$(NC)"
	@docker build -t maxregneros-builder -f Dockerfile.build .
	@docker run --rm -v $(PWD):/workspace maxregneros-builder make build

# Show build statistics
.PHONY: stats
stats:
	@echo -e "$(BLUE)📊 Build Statistics$(NC)"
	@echo -e "$(BLUE)==================$(NC)"
	@if [ -f "$(ISO_NAME)" ]; then \
		echo -e "ISO Size:     $$(du -h $(ISO_NAME) | cut -f1)"; \
		echo -e "ISO Created:  $$(stat -c %y $(ISO_NAME))"; \
	fi
	@if [ -d "$(BUILD_DIR)" ]; then \
		echo -e "Build Size:   $$(du -sh $(BUILD_DIR) | cut -f1)"; \
		echo -e "Kernel Size:  $$(du -sh $(BUILD_DIR)/kernel 2>/dev/null | cut -f1 || echo 'N/A')"; \
		echo -e "Rootfs Size:  $$(du -sh $(BUILD_DIR)/rootfs 2>/dev/null | cut -f1 || echo 'N/A')"; \
	fi
	@echo -e "Build Jobs:   $(JOBS)"
	@echo -e "Architecture: $(ARCH)"

# Help target
.PHONY: help
help:
	@echo -e "$(BLUE)MaxRegnerOS Build System Help$(NC)"
	@echo -e "$(BLUE)==============================$(NC)"
	@echo ""
	@echo -e "$(YELLOW)Main Targets:$(NC)"
	@echo -e "  all          - Complete build process (default)"
	@echo -e "  build        - Build MaxRegnerOS ISO"
	@echo -e "  test         - Run quality assurance tests"
	@echo -e "  package      - Create distribution package"
	@echo -e "  clean        - Clean build directory"
	@echo ""
	@echo -e "$(YELLOW)Component Targets:$(NC)"
	@echo -e "  kernel       - Build Linux kernel"
	@echo -e "  desktop      - Build desktop environment"
	@echo -e "  applications - Build application suite"
	@echo -e "  rootfs       - Create root filesystem"
	@echo -e "  iso          - Create ISO image"
	@echo ""
	@echo -e "$(YELLOW)Utility Targets:$(NC)"
	@echo -e "  quick        - Quick build (skip tests)"
	@echo -e "  dev          - Development build (minimal)"
	@echo -e "  vm-test      - Test in virtual machine"
	@echo -e "  stats        - Show build statistics"
	@echo -e "  checksums    - Calculate file checksums"
	@echo ""
	@echo -e "$(YELLOW)Installation:$(NC)"
	@echo -e "  install-usb USB_DEVICE=/dev/sdX - Install to USB device"
	@echo ""
	@echo -e "$(YELLOW)Examples:$(NC)"
	@echo -e "  make                    # Full build and test"
	@echo -e "  make quick              # Quick build without tests"
	@echo -e "  make vm-test            # Test in QEMU"
	@echo -e "  make install-usb USB_DEVICE=/dev/sdb"

# Create necessary directories and files
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Dependency tracking
.PHONY: check-tools
check-tools:
	@command -v $(CC) >/dev/null 2>&1 || (echo -e "$(RED)❌ $(CC) not found$(NC)" && exit 1)
	@command -v $(MAKE) >/dev/null 2>&1 || (echo -e "$(RED)❌ $(MAKE) not found$(NC)" && exit 1)
	@command -v xorriso >/dev/null 2>&1 || (echo -e "$(RED)❌ xorriso not found$(NC)" && exit 1)
	@command -v mksquashfs >/dev/null 2>&1 || (echo -e "$(RED)❌ mksquashfs not found$(NC)" && exit 1)

# Phony targets to avoid conflicts with files
.PHONY: all info dependencies clean setup kernel-source kernel-config kernel desktop applications rootfs system-tools initramfs squashfs bootloader iso checksums build test package quick dev install-usb vm-test docker-build stats help check-tools

