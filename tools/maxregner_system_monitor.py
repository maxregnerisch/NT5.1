#!/usr/bin/env python3
"""
MaxRegner System Monitor
Advanced system monitoring and diagnostic tool for MaxRegnerOS
"""

import os
import sys
import time
import json
import psutil
import socket
import threading
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass, asdict

@dataclass
class SystemMetrics:
    timestamp: str
    cpu_percent: float
    memory_percent: float
    memory_used: int
    memory_total: int
    disk_usage: Dict[str, Dict]
    network_io: Dict
    processes: int
    uptime: float
    load_average: List[float]
    temperature: Optional[float] = None

class MaxRegnerSystemMonitor:
    def __init__(self):
        self.running = False
        self.metrics_history = []
        self.max_history = 1000
        self.log_file = Path("/var/log/maxregner_monitor.log")
        self.config_file = Path("/etc/maxregner/monitor.conf")
        
        self.load_config()
        self.setup_logging()
    
    def load_config(self):
        """Load monitoring configuration"""
        default_config = {
            "update_interval": 1.0,
            "log_metrics": True,
            "alert_thresholds": {
                "cpu_percent": 90.0,
                "memory_percent": 85.0,
                "disk_percent": 90.0,
                "temperature": 80.0
            },
            "network_monitoring": True,
            "process_monitoring": True,
            "auto_cleanup": True
        }
        
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    self.config = json.load(f)
            except:
                self.config = default_config
        else:
            self.config = default_config
            self.save_config()
    
    def save_config(self):
        """Save configuration to file"""
        self.config_file.parent.mkdir(parents=True, exist_ok=True)
        with open(self.config_file, 'w') as f:
            json.dump(self.config, f, indent=2)
    
    def setup_logging(self):
        """Setup logging directory and file"""
        self.log_file.parent.mkdir(parents=True, exist_ok=True)
    
    def get_system_metrics(self) -> SystemMetrics:
        """Collect current system metrics"""
        # CPU metrics
        cpu_percent = psutil.cpu_percent(interval=0.1)
        
        # Memory metrics
        memory = psutil.virtual_memory()
        
        # Disk metrics
        disk_usage = {}
        for partition in psutil.disk_partitions():
            try:
                usage = psutil.disk_usage(partition.mountpoint)
                disk_usage[partition.mountpoint] = {
                    'total': usage.total,
                    'used': usage.used,
                    'free': usage.free,
                    'percent': (usage.used / usage.total) * 100
                }
            except:
                continue
        
        # Network metrics
        network_io = psutil.net_io_counters()._asdict()
        
        # Process count
        processes = len(psutil.pids())
        
        # System uptime
        uptime = time.time() - psutil.boot_time()
        
        # Load average
        try:
            load_avg = list(os.getloadavg())
        except:
            load_avg = [0.0, 0.0, 0.0]
        
        # Temperature (if available)
        temperature = self.get_cpu_temperature()
        
        return SystemMetrics(
            timestamp=datetime.now().isoformat(),
            cpu_percent=cpu_percent,
            memory_percent=memory.percent,
            memory_used=memory.used,
            memory_total=memory.total,
            disk_usage=disk_usage,
            network_io=network_io,
            processes=processes,
            uptime=uptime,
            load_average=load_avg,
            temperature=temperature
        )
    
    def get_cpu_temperature(self) -> Optional[float]:
        """Get CPU temperature if available"""
        try:
            # Try different temperature sources
            temp_files = [
                "/sys/class/thermal/thermal_zone0/temp",
                "/sys/class/hwmon/hwmon0/temp1_input",
                "/sys/class/hwmon/hwmon1/temp1_input"
            ]
            
            for temp_file in temp_files:
                if os.path.exists(temp_file):
                    with open(temp_file, 'r') as f:
                        temp = int(f.read().strip())
                        # Convert millidegrees to degrees
                        return temp / 1000.0 if temp > 1000 else temp
        except:
            pass
        
        return None
    
    def check_alerts(self, metrics: SystemMetrics):
        """Check for alert conditions"""
        alerts = []
        thresholds = self.config['alert_thresholds']
        
        # CPU alert
        if metrics.cpu_percent > thresholds['cpu_percent']:
            alerts.append(f"HIGH CPU: {metrics.cpu_percent:.1f}%")
        
        # Memory alert
        if metrics.memory_percent > thresholds['memory_percent']:
            alerts.append(f"HIGH MEMORY: {metrics.memory_percent:.1f}%")
        
        # Disk alerts
        for mount, usage in metrics.disk_usage.items():
            if usage['percent'] > thresholds['disk_percent']:
                alerts.append(f"HIGH DISK {mount}: {usage['percent']:.1f}%")
        
        # Temperature alert
        if metrics.temperature and metrics.temperature > thresholds['temperature']:
            alerts.append(f"HIGH TEMP: {metrics.temperature:.1f}°C")
        
        # Log alerts
        if alerts:
            alert_msg = f"[ALERT] {', '.join(alerts)}"
            self.log_message(alert_msg)
            print(f"🚨 {alert_msg}")
    
    def log_message(self, message: str):
        """Log message to file"""
        if self.config['log_metrics']:
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            with open(self.log_file, 'a') as f:
                f.write(f"[{timestamp}] {message}\n")
    
    def format_bytes(self, bytes_value: int) -> str:
        """Format bytes to human readable format"""
        for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
            if bytes_value < 1024.0:
                return f"{bytes_value:.1f}{unit}"
            bytes_value /= 1024.0
        return f"{bytes_value:.1f}PB"
    
    def format_uptime(self, seconds: float) -> str:
        """Format uptime to human readable format"""
        days = int(seconds // 86400)
        hours = int((seconds % 86400) // 3600)
        minutes = int((seconds % 3600) // 60)
        
        if days > 0:
            return f"{days}d {hours}h {minutes}m"
        elif hours > 0:
            return f"{hours}h {minutes}m"
        else:
            return f"{minutes}m"
    
    def display_metrics(self, metrics: SystemMetrics):
        """Display metrics in terminal"""
        # Clear screen
        os.system('clear')
        
        print("🖥️  MaxRegner System Monitor")
        print("=" * 50)
        print(f"Time: {metrics.timestamp}")
        print(f"Uptime: {self.format_uptime(metrics.uptime)}")
        print()
        
        # CPU and Memory
        print("📊 CPU & Memory:")
        print(f"  CPU Usage: {metrics.cpu_percent:6.1f}% {'🔥' if metrics.cpu_percent > 80 else '✅'}")
        print(f"  Memory:    {metrics.memory_percent:6.1f}% ({self.format_bytes(metrics.memory_used)}/{self.format_bytes(metrics.memory_total)}) {'🔥' if metrics.memory_percent > 80 else '✅'}")
        print(f"  Load Avg:  {metrics.load_average[0]:.2f}, {metrics.load_average[1]:.2f}, {metrics.load_average[2]:.2f}")
        
        if metrics.temperature:
            temp_status = '🔥' if metrics.temperature > 70 else '❄️' if metrics.temperature < 40 else '🌡️'
            print(f"  CPU Temp:  {metrics.temperature:6.1f}°C {temp_status}")
        
        print()
        
        # Disk Usage
        print("💾 Disk Usage:")
        for mount, usage in metrics.disk_usage.items():
            status = '🔥' if usage['percent'] > 80 else '✅'
            print(f"  {mount:15} {usage['percent']:6.1f}% ({self.format_bytes(usage['used'])}/{self.format_bytes(usage['total'])}) {status}")
        print()
        
        # Network
        print("🌐 Network I/O:")
        print(f"  Bytes Sent:     {self.format_bytes(metrics.network_io['bytes_sent'])}")
        print(f"  Bytes Received: {self.format_bytes(metrics.network_io['bytes_recv'])}")
        print(f"  Packets Sent:   {metrics.network_io['packets_sent']:,}")
        print(f"  Packets Recv:   {metrics.network_io['packets_recv']:,}")
        print()
        
        # Processes
        print(f"🔄 Active Processes: {metrics.processes}")
        print()
        
        # Top processes
        self.display_top_processes()
        
        print("\nPress Ctrl+C to exit")
    
    def display_top_processes(self, limit: int = 5):
        """Display top processes by CPU usage"""
        try:
            processes = []
            for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent']):
                try:
                    processes.append(proc.info)
                except:
                    continue
            
            # Sort by CPU usage
            processes.sort(key=lambda x: x['cpu_percent'] or 0, reverse=True)
            
            print("🔝 Top Processes (by CPU):")
            print("  PID    NAME                CPU%   MEM%")
            print("  " + "-" * 40)
            
            for proc in processes[:limit]:
                pid = proc['pid']
                name = (proc['name'] or 'Unknown')[:15]
                cpu = proc['cpu_percent'] or 0
                mem = proc['memory_percent'] or 0
                print(f"  {pid:5d}  {name:15} {cpu:6.1f} {mem:6.1f}")
        
        except Exception as e:
            print(f"Error getting process info: {e}")
    
    def start_monitoring(self):
        """Start continuous monitoring"""
        self.running = True
        print("🚀 Starting MaxRegner System Monitor...")
        
        try:
            while self.running:
                metrics = self.get_system_metrics()
                
                # Store metrics
                self.metrics_history.append(metrics)
                if len(self.metrics_history) > self.max_history:
                    self.metrics_history.pop(0)
                
                # Check for alerts
                self.check_alerts(metrics)
                
                # Display metrics
                self.display_metrics(metrics)
                
                # Wait for next update
                time.sleep(self.config['update_interval'])
        
        except KeyboardInterrupt:
            print("\n👋 Stopping monitor...")
            self.running = False
    
    def get_system_info(self) -> Dict:
        """Get detailed system information"""
        info = {
            'system': {
                'hostname': socket.gethostname(),
                'platform': os.uname().sysname,
                'release': os.uname().release,
                'version': os.uname().version,
                'machine': os.uname().machine,
                'processor': os.uname().processor
            },
            'cpu': {
                'physical_cores': psutil.cpu_count(logical=False),
                'logical_cores': psutil.cpu_count(logical=True),
                'max_frequency': psutil.cpu_freq().max if psutil.cpu_freq() else None,
                'current_frequency': psutil.cpu_freq().current if psutil.cpu_freq() else None
            },
            'memory': {
                'total': psutil.virtual_memory().total,
                'available': psutil.virtual_memory().available,
                'swap_total': psutil.swap_memory().total,
                'swap_used': psutil.swap_memory().used
            },
            'disk': {},
            'network': {}
        }
        
        # Disk information
        for partition in psutil.disk_partitions():
            try:
                usage = psutil.disk_usage(partition.mountpoint)
                info['disk'][partition.mountpoint] = {
                    'device': partition.device,
                    'fstype': partition.fstype,
                    'total': usage.total,
                    'used': usage.used,
                    'free': usage.free
                }
            except:
                continue
        
        # Network interfaces
        for interface, addresses in psutil.net_if_addrs().items():
            info['network'][interface] = []
            for addr in addresses:
                info['network'][interface].append({
                    'family': str(addr.family),
                    'address': addr.address,
                    'netmask': addr.netmask,
                    'broadcast': addr.broadcast
                })
        
        return info
    
    def export_metrics(self, filename: str):
        """Export metrics history to JSON file"""
        data = {
            'export_time': datetime.now().isoformat(),
            'config': self.config,
            'system_info': self.get_system_info(),
            'metrics': [asdict(m) for m in self.metrics_history]
        }
        
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)
        
        print(f"📊 Metrics exported to {filename}")

def main():
    """Main CLI interface"""
    if len(sys.argv) < 2:
        print("MaxRegner System Monitor v1.0.0")
        print("Usage:")
        print("  maxregner_monitor start         - Start real-time monitoring")
        print("  maxregner_monitor info          - Show system information")
        print("  maxregner_monitor snapshot      - Take single metrics snapshot")
        print("  maxregner_monitor export <file> - Export metrics to JSON")
        print("  maxregner_monitor config        - Show current configuration")
        return
    
    monitor = MaxRegnerSystemMonitor()
    command = sys.argv[1]
    
    if command == "start":
        monitor.start_monitoring()
    
    elif command == "info":
        info = monitor.get_system_info()
        print("🖥️  MaxRegnerOS System Information")
        print("=" * 40)
        
        print("\n💻 System:")
        for key, value in info['system'].items():
            print(f"  {key.title():15}: {value}")
        
        print("\n🔧 CPU:")
        for key, value in info['cpu'].items():
            if value is not None:
                if 'frequency' in key:
                    print(f"  {key.replace('_', ' ').title():15}: {value:.0f} MHz")
                else:
                    print(f"  {key.replace('_', ' ').title():15}: {value}")
        
        print("\n💾 Memory:")
        for key, value in info['memory'].items():
            print(f"  {key.replace('_', ' ').title():15}: {monitor.format_bytes(value)}")
        
        print("\n💿 Disks:")
        for mount, disk_info in info['disk'].items():
            print(f"  {mount}:")
            print(f"    Device: {disk_info['device']}")
            print(f"    Type:   {disk_info['fstype']}")
            print(f"    Size:   {monitor.format_bytes(disk_info['total'])}")
            print(f"    Used:   {monitor.format_bytes(disk_info['used'])}")
            print(f"    Free:   {monitor.format_bytes(disk_info['free'])}")
    
    elif command == "snapshot":
        metrics = monitor.get_system_metrics()
        print("📊 System Metrics Snapshot")
        print("=" * 30)
        print(f"Time: {metrics.timestamp}")
        print(f"CPU:  {metrics.cpu_percent:.1f}%")
        print(f"RAM:  {metrics.memory_percent:.1f}% ({monitor.format_bytes(metrics.memory_used)}/{monitor.format_bytes(metrics.memory_total)})")
        print(f"Processes: {metrics.processes}")
        print(f"Uptime: {monitor.format_uptime(metrics.uptime)}")
        if metrics.temperature:
            print(f"CPU Temp: {metrics.temperature:.1f}°C")
    
    elif command == "export":
        if len(sys.argv) < 3:
            print("Usage: maxregner_monitor export <filename>")
            return
        
        filename = sys.argv[2]
        monitor.export_metrics(filename)
    
    elif command == "config":
        print("⚙️  Current Configuration:")
        print(json.dumps(monitor.config, indent=2))
    
    else:
        print(f"Unknown command: {command}")

if __name__ == "__main__":
    main()

