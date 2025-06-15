#!/usr/bin/env python3
"""
MaxRegner Package Manager (mrpkg)
Advanced package management system for MaxRegnerOS
"""

import os
import sys
import json
import hashlib
import tarfile
import sqlite3
import requests
import subprocess
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass
from datetime import datetime

@dataclass
class Package:
    name: str
    version: str
    description: str
    dependencies: List[str]
    size: int
    checksum: str
    install_path: str
    files: List[str]

class MRPKGManager:
    def __init__(self, root_path: str = "/"):
        self.root_path = Path(root_path)
        self.db_path = self.root_path / "var/lib/mrpkg/packages.db"
        self.cache_path = self.root_path / "var/cache/mrpkg"
        self.config_path = self.root_path / "etc/mrpkg/mrpkg.conf"
        
        # Ensure directories exist
        self.db_path.parent.mkdir(parents=True, exist_ok=True)
        self.cache_path.mkdir(parents=True, exist_ok=True)
        self.config_path.parent.mkdir(parents=True, exist_ok=True)
        
        self.init_database()
        self.load_config()
    
    def init_database(self):
        """Initialize the package database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS packages (
                name TEXT PRIMARY KEY,
                version TEXT NOT NULL,
                description TEXT,
                dependencies TEXT,
                size INTEGER,
                checksum TEXT,
                install_path TEXT,
                files TEXT,
                install_date TEXT,
                status TEXT DEFAULT 'installed'
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS repositories (
                name TEXT PRIMARY KEY,
                url TEXT NOT NULL,
                enabled INTEGER DEFAULT 1,
                priority INTEGER DEFAULT 100
            )
        ''')
        
        # Add default repositories
        cursor.execute('''
            INSERT OR IGNORE INTO repositories (name, url, priority)
            VALUES ('main', 'https://packages.maxregneros.com/main', 100)
        ''')
        
        cursor.execute('''
            INSERT OR IGNORE INTO repositories (name, url, priority)
            VALUES ('updates', 'https://packages.maxregneros.com/updates', 90)
        ''')
        
        conn.commit()
        conn.close()
    
    def load_config(self):
        """Load configuration from file"""
        default_config = {
            "auto_update": True,
            "check_signatures": True,
            "parallel_downloads": 4,
            "cache_size_limit": "1GB",
            "log_level": "INFO"
        }
        
        if self.config_path.exists():
            with open(self.config_path, 'r') as f:
                self.config = json.load(f)
        else:
            self.config = default_config
            self.save_config()
    
    def save_config(self):
        """Save configuration to file"""
        with open(self.config_path, 'w') as f:
            json.dump(self.config, f, indent=2)
    
    def update_repositories(self):
        """Update package lists from repositories"""
        print("🔄 Updating package repositories...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute("SELECT name, url FROM repositories WHERE enabled = 1")
        repos = cursor.fetchall()
        
        for repo_name, repo_url in repos:
            try:
                print(f"  📦 Updating {repo_name}...")
                
                # Download package list
                response = requests.get(f"{repo_url}/Packages.json", timeout=30)
                response.raise_for_status()
                
                packages = response.json()
                
                # Update local cache
                cache_file = self.cache_path / f"{repo_name}_packages.json"
                with open(cache_file, 'w') as f:
                    json.dump(packages, f, indent=2)
                
                print(f"  ✅ {repo_name}: {len(packages)} packages")
                
            except Exception as e:
                print(f"  ❌ Failed to update {repo_name}: {e}")
        
        conn.close()
        print("✅ Repository update complete")
    
    def search_packages(self, query: str) -> List[Dict]:
        """Search for packages matching query"""
        results = []
        
        # Search in cached package lists
        for cache_file in self.cache_path.glob("*_packages.json"):
            try:
                with open(cache_file, 'r') as f:
                    packages = json.load(f)
                
                for pkg in packages:
                    if (query.lower() in pkg['name'].lower() or 
                        query.lower() in pkg['description'].lower()):
                        results.append(pkg)
            
            except Exception as e:
                print(f"Error reading {cache_file}: {e}")
        
        return results
    
    def install_package(self, package_name: str, version: str = "latest"):
        """Install a package"""
        print(f"📦 Installing {package_name}...")
        
        # Find package in repositories
        package_info = self.find_package(package_name, version)
        if not package_info:
            print(f"❌ Package {package_name} not found")
            return False
        
        # Check dependencies
        if not self.check_dependencies(package_info.get('dependencies', [])):
            print("❌ Dependency check failed")
            return False
        
        # Download package
        package_file = self.download_package(package_info)
        if not package_file:
            print("❌ Failed to download package")
            return False
        
        # Verify checksum
        if not self.verify_checksum(package_file, package_info['checksum']):
            print("❌ Checksum verification failed")
            return False
        
        # Extract and install
        if self.extract_package(package_file, package_info):
            self.register_package(package_info)
            print(f"✅ {package_name} installed successfully")
            return True
        else:
            print(f"❌ Failed to install {package_name}")
            return False
    
    def remove_package(self, package_name: str):
        """Remove an installed package"""
        print(f"🗑️  Removing {package_name}...")
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute("SELECT * FROM packages WHERE name = ?", (package_name,))
        package = cursor.fetchone()
        
        if not package:
            print(f"❌ Package {package_name} is not installed")
            conn.close()
            return False
        
        # Remove files
        files = json.loads(package[7]) if package[7] else []
        for file_path in files:
            full_path = self.root_path / file_path.lstrip('/')
            try:
                if full_path.exists():
                    full_path.unlink()
            except Exception as e:
                print(f"Warning: Could not remove {file_path}: {e}")
        
        # Remove from database
        cursor.execute("DELETE FROM packages WHERE name = ?", (package_name,))
        conn.commit()
        conn.close()
        
        print(f"✅ {package_name} removed successfully")
        return True
    
    def list_installed(self):
        """List all installed packages"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute("SELECT name, version, description FROM packages ORDER BY name")
        packages = cursor.fetchall()
        
        if not packages:
            print("No packages installed")
            return
        
        print("📦 Installed packages:")
        print("-" * 60)
        for name, version, description in packages:
            print(f"{name:20} {version:10} {description[:30]}")
        
        conn.close()
    
    def find_package(self, name: str, version: str = "latest") -> Optional[Dict]:
        """Find package in repositories"""
        for cache_file in self.cache_path.glob("*_packages.json"):
            try:
                with open(cache_file, 'r') as f:
                    packages = json.load(f)
                
                for pkg in packages:
                    if pkg['name'] == name:
                        if version == "latest" or pkg['version'] == version:
                            return pkg
            
            except Exception as e:
                continue
        
        return None
    
    def check_dependencies(self, dependencies: List[str]) -> bool:
        """Check if all dependencies are satisfied"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        for dep in dependencies:
            cursor.execute("SELECT name FROM packages WHERE name = ?", (dep,))
            if not cursor.fetchone():
                print(f"❌ Missing dependency: {dep}")
                conn.close()
                return False
        
        conn.close()
        return True
    
    def download_package(self, package_info: Dict) -> Optional[Path]:
        """Download package file"""
        url = package_info['download_url']
        filename = package_info['filename']
        
        local_file = self.cache_path / filename
        
        try:
            print(f"  📥 Downloading {filename}...")
            response = requests.get(url, stream=True, timeout=60)
            response.raise_for_status()
            
            with open(local_file, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            
            return local_file
        
        except Exception as e:
            print(f"  ❌ Download failed: {e}")
            return None
    
    def verify_checksum(self, file_path: Path, expected_checksum: str) -> bool:
        """Verify file checksum"""
        sha256_hash = hashlib.sha256()
        
        with open(file_path, 'rb') as f:
            for chunk in iter(lambda: f.read(4096), b""):
                sha256_hash.update(chunk)
        
        return sha256_hash.hexdigest() == expected_checksum
    
    def extract_package(self, package_file: Path, package_info: Dict) -> bool:
        """Extract package contents"""
        try:
            with tarfile.open(package_file, 'r:xz') as tar:
                tar.extractall(path=self.root_path)
            return True
        
        except Exception as e:
            print(f"  ❌ Extraction failed: {e}")
            return False
    
    def register_package(self, package_info: Dict):
        """Register package in database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT OR REPLACE INTO packages 
            (name, version, description, dependencies, size, checksum, 
             install_path, files, install_date, status)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            package_info['name'],
            package_info['version'],
            package_info['description'],
            json.dumps(package_info.get('dependencies', [])),
            package_info['size'],
            package_info['checksum'],
            package_info.get('install_path', '/usr'),
            json.dumps(package_info.get('files', [])),
            datetime.now().isoformat(),
            'installed'
        ))
        
        conn.commit()
        conn.close()

def main():
    """Main CLI interface"""
    if len(sys.argv) < 2:
        print("MaxRegner Package Manager (mrpkg) v1.0.0")
        print("Usage:")
        print("  mrpkg update                    - Update package repositories")
        print("  mrpkg search <query>           - Search for packages")
        print("  mrpkg install <package>        - Install a package")
        print("  mrpkg remove <package>         - Remove a package")
        print("  mrpkg list                     - List installed packages")
        print("  mrpkg info <package>           - Show package information")
        return
    
    manager = MRPKGManager()
    command = sys.argv[1]
    
    if command == "update":
        manager.update_repositories()
    
    elif command == "search":
        if len(sys.argv) < 3:
            print("Usage: mrpkg search <query>")
            return
        
        query = sys.argv[2]
        results = manager.search_packages(query)
        
        if not results:
            print(f"No packages found matching '{query}'")
            return
        
        print(f"📦 Found {len(results)} packages:")
        print("-" * 60)
        for pkg in results[:10]:  # Show first 10 results
            print(f"{pkg['name']:20} {pkg['version']:10} {pkg['description'][:30]}")
    
    elif command == "install":
        if len(sys.argv) < 3:
            print("Usage: mrpkg install <package>")
            return
        
        package = sys.argv[2]
        manager.install_package(package)
    
    elif command == "remove":
        if len(sys.argv) < 3:
            print("Usage: mrpkg remove <package>")
            return
        
        package = sys.argv[2]
        manager.remove_package(package)
    
    elif command == "list":
        manager.list_installed()
    
    elif command == "info":
        if len(sys.argv) < 3:
            print("Usage: mrpkg info <package>")
            return
        
        package = sys.argv[2]
        info = manager.find_package(package)
        if info:
            print(f"📦 Package: {info['name']}")
            print(f"Version: {info['version']}")
            print(f"Description: {info['description']}")
            print(f"Size: {info['size']} bytes")
            print(f"Dependencies: {', '.join(info.get('dependencies', []))}")
        else:
            print(f"Package '{package}' not found")
    
    else:
        print(f"Unknown command: {command}")

if __name__ == "__main__":
    main()

