# MaxRegnerOS - Complete Custom Operating System

![MaxRegnerOS Logo](https://img.shields.io/badge/MaxRegnerOS-v1.0.0-blue?style=for-the-badge)

## 🚀 Overview

MaxRegnerOS is a complete, custom Linux-based operating system built from scratch with:

- **Custom Linux Kernel** - Optimized for performance and hardware compatibility
- **MaxRegner Desktop Environment** - Built from the ground up with custom UI/UX
- **Complete Application Suite** - File manager, text editor, web browser, and more
- **Multi-GB Bootable ISO** - Full-featured live system
- **Custom Design Language** - Unique visual identity throughout

## ✨ Features

### 🔧 Core System
- **Custom Linux Kernel 6.6** - Compiled with MaxRegnerOS optimizations
- **Custom Init System** - Fast boot and service management
- **Live Boot Capability** - Run directly from USB/DVD without installation
- **Hardware Detection** - Automatic driver loading and configuration

### 🎨 Desktop Environment
- **MaxRegner Window Manager** - Custom X11-based window management
- **MaxRegner Panel** - Bottom taskbar with system information
- **Custom Themes** - MaxRegner blue color scheme throughout
- **Desktop Integration** - Seamless application launching

### 📱 Applications Suite
- **MaxRegner Files** - Full-featured file manager with directory browsing
- **MaxRegner Editor** - Text editor with syntax highlighting
- **MaxRegner Browser** - Custom web browser with modern rendering
- **System Tools** - Task manager, settings, terminal emulator
- **Media Player** - Audio and video playback capabilities

### 🎯 Technical Specifications
- **Architecture**: x86_64 (64-bit)
- **Kernel**: Linux 6.6 (custom compiled)
- **Display Server**: X11
- **File Systems**: EXT4, SquashFS, ISO9660, VFAT
- **Boot Loader**: GRUB2
- **Package Format**: Custom .mrpkg format
- **ISO Size**: ~2-4 GB (full-featured)

## 🛠️ Building MaxRegnerOS

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential git wget curl xorriso squashfs-tools debootstrap

# Fedora/RHEL
sudo dnf groupinstall "Development Tools"
sudo dnf install git wget curl xorriso squashfs-tools debootstrap

# Arch Linux
sudo pacman -S base-devel git wget curl libisoburn squashfs-tools debootstrap
```

### Build Process
```bash
# Clone the repository
git clone https://github.com/maxregnerisch/NT5.1.git
cd NT5.1

# Make build script executable
chmod +x build_maxregneros.sh

# Start the build (takes 30-60 minutes)
./build_maxregneros.sh
```

### Build Output
- `MaxRegnerOS-1.0.0.iso` - Bootable ISO image
- `build/` - Build artifacts and intermediate files
- `build/rootfs/` - Complete root filesystem
- `build/kernel/` - Compiled kernel and modules

## 🚀 Running MaxRegnerOS

### Virtual Machine (Recommended for Testing)
```bash
# VirtualBox
VBoxManage createvm --name "MaxRegnerOS" --register
VBoxManage modifyvm "MaxRegnerOS" --memory 2048 --vram 128
VBoxManage storagectl "MaxRegnerOS" --name "IDE" --add ide
VBoxManage storageattach "MaxRegnerOS" --storagectl "IDE" --port 0 --device 0 --type dvddrive --medium MaxRegnerOS-1.0.0.iso

# QEMU
qemu-system-x86_64 -cdrom MaxRegnerOS-1.0.0.iso -m 2048 -enable-kvm
```

### Physical Hardware
```bash
# Create bootable USB (Linux)
sudo dd if=MaxRegnerOS-1.0.0.iso of=/dev/sdX bs=4M status=progress

# Create bootable USB (Windows)
# Use Rufus or similar tool to write ISO to USB drive
```

## 📁 Project Structure

```
NT5.1/
├── build_maxregneros.sh          # Main build script
├── MaxRegnerOS_README.md          # This file
├── build/                         # Build directory (created during build)
│   ├── kernel/                    # Kernel source and compilation
│   ├── desktop/                   # Desktop environment source
│   ├── applications/              # Application source code
│   ├── rootfs/                    # Root filesystem
│   └── iso/                       # ISO construction directory
├── Source/                        # Original Windows XP source (legacy)
├── dist/                          # Distribution files
└── tools/                         # Additional build tools
```

## 🎨 Customization

### Themes and Appearance
- Edit `build/desktop/src/maxregner_wm.c` for window manager behavior
- Modify color schemes in application source files
- Custom wallpapers in `build/rootfs/usr/share/backgrounds/`

### Adding Applications
1. Create source code in `build/applications/src/`
2. Add compilation commands to `build_applications()` function
3. Create desktop entry in `build/rootfs/usr/share/applications/`

### Kernel Configuration
- Modify kernel config in `build_kernel()` function
- Add/remove drivers and features as needed
- Recompile with `make -j$(nproc)`

## 🔧 Development

### Building Individual Components
```bash
# Build only kernel
./build_maxregneros.sh build_kernel

# Build only desktop
./build_maxregneros.sh build_desktop

# Build only applications
./build_maxregneros.sh build_applications

# Create ISO from existing build
./build_maxregneros.sh create_iso
```

### Debugging
- Boot with `debug` kernel parameter for verbose output
- Check `/var/log/` for system logs
- Use `maxregner_debug` tool for system diagnostics

## 📊 Performance

### System Requirements
- **Minimum**: 1GB RAM, 2GB storage, x86_64 CPU
- **Recommended**: 2GB RAM, 4GB storage, modern x86_64 CPU
- **Optimal**: 4GB+ RAM, 8GB+ storage, multi-core CPU

### Boot Times
- **Cold Boot**: ~15-30 seconds (depending on hardware)
- **Desktop Ready**: ~45-60 seconds total
- **Application Launch**: <2 seconds average

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Linux Kernel Community
- X.Org Foundation
- GNU Project
- All open source contributors

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/maxregnerisch/NT5.1/issues)
- **Discussions**: [GitHub Discussions](https://github.com/maxregnerisch/NT5.1/discussions)
- **Email**: maxregner@maxregneros.com

---

**MaxRegnerOS** - *Your Operating System, Your Way* 🚀

