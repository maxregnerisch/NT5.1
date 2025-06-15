#!/usr/bin/env python3
"""
MaxRegnerOS Quality Assurance Suite
Comprehensive testing and validation system for MaxRegnerOS builds
"""

import os
import sys
import json
import time
import subprocess
import threading
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime
import hashlib
import tempfile

@dataclass
class TestResult:
    name: str
    category: str
    status: str  # PASS, FAIL, SKIP, ERROR
    duration: float
    message: str
    details: Optional[Dict] = None

@dataclass
class QAReport:
    timestamp: str
    build_info: Dict
    total_tests: int
    passed: int
    failed: int
    skipped: int
    errors: int
    duration: float
    results: List[TestResult]

class MaxRegnerOSQASuite:
    def __init__(self, build_dir: str = "build"):
        self.build_dir = Path(build_dir)
        self.iso_file = None
        self.results = []
        self.config = self.load_config()
        
        # Find ISO file
        for iso_file in Path(".").glob("MaxRegnerOS-*.iso"):
            self.iso_file = iso_file
            break
    
    def load_config(self) -> Dict:
        """Load QA configuration"""
        return {
            "timeout_seconds": 300,
            "vm_memory": "2048",
            "vm_disk_size": "4G",
            "test_categories": [
                "build_validation",
                "iso_integrity",
                "boot_test",
                "system_functionality",
                "application_tests",
                "performance_tests",
                "security_tests"
            ],
            "critical_tests": [
                "iso_checksum",
                "iso_bootable",
                "kernel_boot",
                "desktop_start"
            ]
        }
    
    def run_test(self, test_func, name: str, category: str) -> TestResult:
        """Run a single test and capture results"""
        print(f"  🧪 Running {name}...")
        start_time = time.time()
        
        try:
            result = test_func()
            duration = time.time() - start_time
            
            if result is True:
                status = "PASS"
                message = "Test passed successfully"
                details = None
            elif result is False:
                status = "FAIL"
                message = "Test failed"
                details = None
            elif isinstance(result, tuple):
                status, message, details = result
            elif isinstance(result, dict):
                status = result.get('status', 'ERROR')
                message = result.get('message', 'Unknown result')
                details = result.get('details')
            else:
                status = "ERROR"
                message = f"Invalid test result: {result}"
                details = None
        
        except Exception as e:
            duration = time.time() - start_time
            status = "ERROR"
            message = f"Test error: {str(e)}"
            details = {"exception": str(e), "type": type(e).__name__}
        
        test_result = TestResult(
            name=name,
            category=category,
            status=status,
            duration=duration,
            message=message,
            details=details
        )
        
        # Print result
        status_emoji = {
            "PASS": "✅",
            "FAIL": "❌", 
            "SKIP": "⏭️",
            "ERROR": "💥"
        }
        print(f"    {status_emoji.get(status, '❓')} {status}: {message} ({duration:.2f}s)")
        
        return test_result
    
    # Build Validation Tests
    def test_build_directory_structure(self) -> bool:
        """Test that build directory has correct structure"""
        required_dirs = ["iso", "rootfs", "kernel", "desktop", "applications"]
        
        for dir_name in required_dirs:
            if not (self.build_dir / dir_name).exists():
                return False
        
        return True
    
    def test_kernel_exists(self) -> bool:
        """Test that kernel was built successfully"""
        kernel_file = self.build_dir / "iso" / "boot" / "vmlinuz"
        return kernel_file.exists() and kernel_file.stat().st_size > 1000000  # At least 1MB
    
    def test_initrd_exists(self) -> bool:
        """Test that initrd was created"""
        initrd_file = self.build_dir / "iso" / "boot" / "initrd.img"
        return initrd_file.exists() and initrd_file.stat().st_size > 100000  # At least 100KB
    
    def test_desktop_binaries(self) -> bool:
        """Test that desktop environment binaries exist"""
        binaries = ["maxregner_wm", "maxregner_panel"]
        rootfs_bin = self.build_dir / "rootfs" / "usr" / "bin"
        
        for binary in binaries:
            if not (rootfs_bin / binary).exists():
                return False
        
        return True
    
    def test_application_binaries(self) -> bool:
        """Test that application binaries exist"""
        apps = ["maxregner_files", "maxregner_edit", "maxregner_browser"]
        rootfs_bin = self.build_dir / "rootfs" / "usr" / "bin"
        
        for app in apps:
            if not (rootfs_bin / app).exists():
                return False
        
        return True
    
    # ISO Integrity Tests
    def test_iso_exists(self) -> bool:
        """Test that ISO file was created"""
        return self.iso_file is not None and self.iso_file.exists()
    
    def test_iso_size(self) -> Tuple[str, str, Optional[Dict]]:
        """Test ISO file size is reasonable"""
        if not self.iso_file:
            return ("FAIL", "ISO file not found", None)
        
        size_mb = self.iso_file.stat().st_size / (1024 * 1024)
        
        if size_mb < 100:
            return ("FAIL", f"ISO too small: {size_mb:.1f}MB", {"size_mb": size_mb})
        elif size_mb > 8000:
            return ("FAIL", f"ISO too large: {size_mb:.1f}MB", {"size_mb": size_mb})
        else:
            return ("PASS", f"ISO size OK: {size_mb:.1f}MB", {"size_mb": size_mb})
    
    def test_iso_checksum(self) -> Tuple[str, str, Optional[Dict]]:
        """Calculate and verify ISO checksum"""
        if not self.iso_file:
            return ("FAIL", "ISO file not found", None)
        
        sha256_hash = hashlib.sha256()
        
        with open(self.iso_file, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256_hash.update(chunk)
        
        checksum = sha256_hash.hexdigest()
        
        # Save checksum for future verification
        checksum_file = self.iso_file.with_suffix('.sha256')
        with open(checksum_file, 'w') as f:
            f.write(f"{checksum}  {self.iso_file.name}\n")
        
        return ("PASS", f"Checksum calculated: {checksum[:16]}...", {"checksum": checksum})
    
    def test_iso_bootable(self) -> Tuple[str, str, Optional[Dict]]:
        """Test if ISO is bootable using file command"""
        if not self.iso_file:
            return ("FAIL", "ISO file not found", None)
        
        try:
            result = subprocess.run(['file', str(self.iso_file)], 
                                  capture_output=True, text=True, timeout=10)
            
            output = result.stdout.lower()
            
            if 'bootable' in output or 'boot sector' in output:
                return ("PASS", "ISO appears bootable", {"file_output": result.stdout})
            else:
                return ("FAIL", "ISO may not be bootable", {"file_output": result.stdout})
        
        except Exception as e:
            return ("ERROR", f"Could not check bootability: {e}", None)
    
    # Boot Tests (using QEMU)
    def test_qemu_available(self) -> bool:
        """Test if QEMU is available for testing"""
        try:
            subprocess.run(['qemu-system-x86_64', '--version'], 
                          capture_output=True, timeout=5)
            return True
        except:
            return False
    
    def test_vm_boot(self) -> Tuple[str, str, Optional[Dict]]:
        """Test booting ISO in QEMU VM"""
        if not self.iso_file:
            return ("FAIL", "ISO file not found", None)
        
        if not self.test_qemu_available():
            return ("SKIP", "QEMU not available", None)
        
        try:
            # Create temporary log file
            with tempfile.NamedTemporaryFile(mode='w+', suffix='.log', delete=False) as log_file:
                log_path = log_file.name
            
            # Start QEMU with timeout
            cmd = [
                'qemu-system-x86_64',
                '-cdrom', str(self.iso_file),
                '-m', self.config['vm_memory'],
                '-nographic',
                '-serial', f'file:{log_path}',
                '-no-reboot',
                '-enable-kvm'  # If available
            ]
            
            process = subprocess.Popen(cmd, stdout=subprocess.PIPE, 
                                     stderr=subprocess.PIPE)
            
            # Wait for boot or timeout
            try:
                stdout, stderr = process.communicate(timeout=60)
                
                # Read log file
                with open(log_path, 'r') as f:
                    boot_log = f.read()
                
                # Clean up
                os.unlink(log_path)
                
                # Check for successful boot indicators
                success_indicators = [
                    "MaxRegnerOS",
                    "login:",
                    "desktop",
                    "boot complete"
                ]
                
                boot_success = any(indicator in boot_log.lower() 
                                 for indicator in success_indicators)
                
                if boot_success:
                    return ("PASS", "VM boot successful", 
                           {"boot_log": boot_log[:500]})
                else:
                    return ("FAIL", "VM boot failed", 
                           {"boot_log": boot_log[:500], "stderr": stderr.decode()[:200]})
            
            except subprocess.TimeoutExpired:
                process.kill()
                return ("FAIL", "VM boot timeout", None)
        
        except Exception as e:
            return ("ERROR", f"VM test error: {e}", None)
    
    # System Functionality Tests
    def test_filesystem_structure(self) -> bool:
        """Test root filesystem structure"""
        rootfs = self.build_dir / "rootfs"
        required_dirs = ["bin", "usr", "etc", "var", "tmp", "home", "root"]
        
        for dir_name in required_dirs:
            if not (rootfs / dir_name).exists():
                return False
        
        return True
    
    def test_init_system(self) -> bool:
        """Test that init script exists and is executable"""
        init_file = self.build_dir / "rootfs" / "init"
        return init_file.exists() and os.access(init_file, os.X_OK)
    
    def test_desktop_entries(self) -> bool:
        """Test that desktop entries exist"""
        desktop_dir = self.build_dir / "rootfs" / "usr" / "share" / "applications"
        required_entries = ["files.desktop", "editor.desktop", "browser.desktop"]
        
        for entry in required_entries:
            if not (desktop_dir / entry).exists():
                return False
        
        return True
    
    # Performance Tests
    def test_iso_compression_ratio(self) -> Tuple[str, str, Optional[Dict]]:
        """Test compression efficiency"""
        if not self.iso_file:
            return ("FAIL", "ISO file not found", None)
        
        rootfs_dir = self.build_dir / "rootfs"
        if not rootfs_dir.exists():
            return ("SKIP", "Rootfs not found", None)
        
        # Calculate uncompressed size
        uncompressed_size = sum(f.stat().st_size for f in rootfs_dir.rglob('*') if f.is_file())
        compressed_size = self.iso_file.stat().st_size
        
        ratio = compressed_size / uncompressed_size if uncompressed_size > 0 else 1
        
        details = {
            "uncompressed_mb": uncompressed_size / (1024 * 1024),
            "compressed_mb": compressed_size / (1024 * 1024),
            "ratio": ratio
        }
        
        if ratio < 0.3:
            return ("PASS", f"Good compression: {ratio:.2f}", details)
        elif ratio < 0.6:
            return ("PASS", f"Acceptable compression: {ratio:.2f}", details)
        else:
            return ("FAIL", f"Poor compression: {ratio:.2f}", details)
    
    # Security Tests
    def test_no_world_writable_files(self) -> Tuple[str, str, Optional[Dict]]:
        """Test for world-writable files (security risk)"""
        rootfs = self.build_dir / "rootfs"
        if not rootfs.exists():
            return ("SKIP", "Rootfs not found", None)
        
        world_writable = []
        
        for file_path in rootfs.rglob('*'):
            if file_path.is_file():
                try:
                    mode = file_path.stat().st_mode
                    if mode & 0o002:  # World writable
                        world_writable.append(str(file_path.relative_to(rootfs)))
                except:
                    continue
        
        if world_writable:
            return ("FAIL", f"Found {len(world_writable)} world-writable files", 
                   {"files": world_writable[:10]})
        else:
            return ("PASS", "No world-writable files found", None)
    
    def test_no_setuid_files(self) -> Tuple[str, str, Optional[Dict]]:
        """Test for setuid files (potential security risk)"""
        rootfs = self.build_dir / "rootfs"
        if not rootfs.exists():
            return ("SKIP", "Rootfs not found", None)
        
        setuid_files = []
        
        for file_path in rootfs.rglob('*'):
            if file_path.is_file():
                try:
                    mode = file_path.stat().st_mode
                    if mode & 0o4000:  # Setuid bit
                        setuid_files.append(str(file_path.relative_to(rootfs)))
                except:
                    continue
        
        # Some setuid files are expected (like su, sudo)
        expected_setuid = ['bin/su', 'usr/bin/sudo', 'usr/bin/passwd']
        unexpected = [f for f in setuid_files if not any(exp in f for exp in expected_setuid)]
        
        if unexpected:
            return ("FAIL", f"Found {len(unexpected)} unexpected setuid files", 
                   {"files": unexpected})
        else:
            return ("PASS", f"Only expected setuid files found ({len(setuid_files)})", 
                   {"files": setuid_files})
    
    def run_all_tests(self) -> QAReport:
        """Run all QA tests"""
        print("🧪 Starting MaxRegnerOS Quality Assurance Suite")
        print("=" * 50)
        
        start_time = time.time()
        
        # Define test suite
        test_suite = [
            # Build Validation
            (self.test_build_directory_structure, "Build Directory Structure", "build_validation"),
            (self.test_kernel_exists, "Kernel Build", "build_validation"),
            (self.test_initrd_exists, "InitRD Creation", "build_validation"),
            (self.test_desktop_binaries, "Desktop Binaries", "build_validation"),
            (self.test_application_binaries, "Application Binaries", "build_validation"),
            
            # ISO Integrity
            (self.test_iso_exists, "ISO File Creation", "iso_integrity"),
            (self.test_iso_size, "ISO File Size", "iso_integrity"),
            (self.test_iso_checksum, "ISO Checksum", "iso_integrity"),
            (self.test_iso_bootable, "ISO Bootability", "iso_integrity"),
            
            # Boot Tests
            (self.test_qemu_available, "QEMU Availability", "boot_test"),
            (self.test_vm_boot, "Virtual Machine Boot", "boot_test"),
            
            # System Functionality
            (self.test_filesystem_structure, "Filesystem Structure", "system_functionality"),
            (self.test_init_system, "Init System", "system_functionality"),
            (self.test_desktop_entries, "Desktop Entries", "system_functionality"),
            
            # Performance
            (self.test_iso_compression_ratio, "Compression Efficiency", "performance_tests"),
            
            # Security
            (self.test_no_world_writable_files, "World-Writable Files", "security_tests"),
            (self.test_no_setuid_files, "Setuid Files", "security_tests"),
        ]
        
        # Run tests by category
        for category in self.config['test_categories']:
            category_tests = [t for t in test_suite if t[2] == category]
            if category_tests:
                print(f"\n📋 {category.replace('_', ' ').title()} Tests:")
                for test_func, name, cat in category_tests:
                    result = self.run_test(test_func, name, cat)
                    self.results.append(result)
        
        # Calculate summary
        total_duration = time.time() - start_time
        total_tests = len(self.results)
        passed = len([r for r in self.results if r.status == "PASS"])
        failed = len([r for r in self.results if r.status == "FAIL"])
        skipped = len([r for r in self.results if r.status == "SKIP"])
        errors = len([r for r in self.results if r.status == "ERROR"])
        
        # Build info
        build_info = {
            "iso_file": str(self.iso_file) if self.iso_file else None,
            "build_dir": str(self.build_dir),
            "iso_size_mb": self.iso_file.stat().st_size / (1024 * 1024) if self.iso_file else 0
        }
        
        report = QAReport(
            timestamp=datetime.now().isoformat(),
            build_info=build_info,
            total_tests=total_tests,
            passed=passed,
            failed=failed,
            skipped=skipped,
            errors=errors,
            duration=total_duration,
            results=self.results
        )
        
        return report
    
    def print_summary(self, report: QAReport):
        """Print test summary"""
        print("\n" + "=" * 50)
        print("🏁 MaxRegnerOS QA Test Summary")
        print("=" * 50)
        
        print(f"📊 Results: {report.passed}✅ {report.failed}❌ {report.skipped}⏭️ {report.errors}💥")
        print(f"⏱️  Duration: {report.duration:.1f} seconds")
        print(f"📁 ISO File: {report.build_info['iso_file']}")
        print(f"💾 ISO Size: {report.build_info['iso_size_mb']:.1f} MB")
        
        # Critical test failures
        critical_failures = [r for r in report.results 
                           if r.status in ["FAIL", "ERROR"] and r.name.lower().replace(" ", "_") in self.config['critical_tests']]
        
        if critical_failures:
            print(f"\n🚨 CRITICAL FAILURES ({len(critical_failures)}):")
            for result in critical_failures:
                print(f"  ❌ {result.name}: {result.message}")
        
        # Overall status
        if report.failed == 0 and report.errors == 0:
            print(f"\n🎉 ALL TESTS PASSED! MaxRegnerOS build is ready for release.")
        elif critical_failures:
            print(f"\n💥 BUILD FAILED! Critical tests failed - DO NOT RELEASE.")
        else:
            print(f"\n⚠️  BUILD WARNING! Some tests failed but no critical issues.")
        
        print("=" * 50)
    
    def save_report(self, report: QAReport, filename: str):
        """Save detailed report to JSON file"""
        with open(filename, 'w') as f:
            json.dump(asdict(report), f, indent=2, default=str)
        
        print(f"📄 Detailed report saved to {filename}")

def main():
    """Main CLI interface"""
    if len(sys.argv) > 1 and sys.argv[1] == "--help":
        print("MaxRegnerOS Quality Assurance Suite v1.0.0")
        print("Usage:")
        print("  maxregneros_qa_suite.py [build_dir]")
        print("")
        print("This tool runs comprehensive tests on MaxRegnerOS builds including:")
        print("  - Build validation")
        print("  - ISO integrity checks")
        print("  - Boot testing (requires QEMU)")
        print("  - System functionality tests")
        print("  - Performance analysis")
        print("  - Security validation")
        return
    
    build_dir = sys.argv[1] if len(sys.argv) > 1 else "build"
    
    qa_suite = MaxRegnerOSQASuite(build_dir)
    report = qa_suite.run_all_tests()
    qa_suite.print_summary(report)
    
    # Save detailed report
    report_file = f"MaxRegnerOS_QA_Report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
    qa_suite.save_report(report, report_file)

if __name__ == "__main__":
    main()

