#!/usr/bin/env python3
"""
OpenOCD Server Launcher
Starts OpenOCD as a GDB server for debugging and programming
"""

import os
import sys
import argparse
import subprocess
import signal
import time
import platform
import configparser
from pathlib import Path

# ANSI color codes
class Colors:
    """Terminal color definitions"""
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    NC = '\033[0m'  # No Color

# Windows console color support detection
if platform.system() == 'Windows':
    try:
        import colorama
        colorama.init()
    except ImportError:
        Colors.RED = ''
        Colors.GREEN = ''
        Colors.YELLOW = ''
        Colors.BLUE = ''
        Colors.NC = ''

class OpenOCDServer:
    """OpenOCD Server Manager"""
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.openocd_process = None
        self.default_openocd_path = self.get_default_openocd_path()
    
    def get_default_openocd_path(self) -> str:
        """Get default OpenOCD path from config file or platform defaults"""
        # Try to load from config file first
        config_file = self.script_dir / 'flash_loader_config.ini'
        if config_file.exists():
            config = configparser.ConfigParser()
            config.read(config_file)
            
            system = platform.system()
            if system == 'Windows' and config.has_option('OpenOCD', 'windows_path'):
                path = config.get('OpenOCD', 'windows_path')
                if Path(path).exists():
                    return path
        
        # Fallback to hardcoded paths
        if platform.system() == 'Windows':
            openocd_paths = [
                r'D:\A04_Works\07_ai_event\04_openocd\openocd-0.12.0\bin\openocd.exe',
                r'C:\openocd\bin\openocd.exe',
                'openocd.exe'
            ]
            for path in openocd_paths:
                if Path(path).exists():
                    return path
        
        return 'openocd'
    
    def parse_arguments(self) -> argparse.Namespace:
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description='OpenOCD Server Launcher - Start OpenOCD as GDB server',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog='''
Examples:
  %(prog)s                     # Start with default config (m130, JTAG)
  %(prog)s -m swd             # Start with SWD mode
  %(prog)s --daemon           # Run in background (daemon mode)
  %(prog)s --kill             # Kill running OpenOCD server
            '''
        )
        
        parser.add_argument('-m', '--mode',
                          default='jtag',
                          choices=['jtag', 'swd'],
                          help='Connection mode (default: jtag)')
        
        parser.add_argument('--daemon',
                          action='store_true',
                          help='Run OpenOCD in background')
        
        parser.add_argument('--kill',
                          action='store_true',
                          help='Kill running OpenOCD server')
        
        parser.add_argument('--openocd-path',
                          type=str,
                          default=None,
                          help='OpenOCD executable path')
        
        parser.add_argument('--verbose',
                          action='store_true',
                          help='Show OpenOCD output')
        
        return parser.parse_args()
    
    def find_config_file(self, mode: str) -> Path:
        """Find configuration file"""
        if mode == 'jtag':
            config_name = 'openocd_jlink_jtag.cfg'
        else:
            config_name = 'openocd_jlink_swd.cfg'
        
        config_path = self.script_dir / config_name
        
        if not config_path.exists():
            raise FileNotFoundError(f"Configuration file not found: {config_path}")
        
        return config_path
    
    def kill_openocd(self):
        """Kill any running OpenOCD processes"""
        if platform.system() == 'Windows':
            subprocess.run(['taskkill', '/F', '/IM', 'openocd.exe'], 
                         capture_output=True)
        else:
            subprocess.run(['pkill', 'openocd'], capture_output=True)
        
        print(f"{Colors.YELLOW}Killed OpenOCD processes{Colors.NC}")
    
    def start_server(self, openocd_path: str, config_file: Path, 
                    daemon: bool, verbose: bool):
        """Start OpenOCD server"""
        cmd = [openocd_path, '-f', str(config_file)]
        
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print(f"{Colors.GREEN}    OpenOCD Server{Colors.NC}")
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print(f"{Colors.YELLOW}OpenOCD Path:{Colors.NC} {openocd_path}")
        print(f"{Colors.YELLOW}Config File:{Colors.NC} {config_file}")
        print(f"{Colors.YELLOW}GDB Port:{Colors.NC} 3339 (JTAG) or 3335 (SWD)")
        print(f"{Colors.YELLOW}Telnet Port:{Colors.NC} 4449 (JTAG) or 4445 (SWD)")
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print()
        
        if daemon:
            # Run in background
            if platform.system() == 'Windows':
                DETACHED_PROCESS = 0x00000008
                self.openocd_process = subprocess.Popen(
                    cmd,
                    stdout=subprocess.DEVNULL if not verbose else None,
                    stderr=subprocess.DEVNULL if not verbose else None,
                    creationflags=DETACHED_PROCESS
                )
            else:
                self.openocd_process = subprocess.Popen(
                    cmd,
                    stdout=subprocess.DEVNULL if not verbose else None,
                    stderr=subprocess.DEVNULL if not verbose else None,
                    preexec_fn=os.setsid
                )
            
            # Wait a bit to check if it started successfully
            time.sleep(2)
            if self.openocd_process.poll() is None:
                print(f"{Colors.GREEN}[OK] OpenOCD server started in background (PID: {self.openocd_process.pid}){Colors.NC}")
                print(f"{Colors.YELLOW}To connect with GDB:{Colors.NC}")
                print(f"  python gdb_loader.py")
                print(f"{Colors.YELLOW}To kill the server:{Colors.NC}")
                print(f"  python openocd_server.py --kill")
            else:
                print(f"{Colors.RED}[FAIL] Failed to start OpenOCD server{Colors.NC}")
                return False
        else:
            # Run in foreground
            print(f"{Colors.YELLOW}Starting OpenOCD server (Press Ctrl+C to stop)...{Colors.NC}")
            print()
            
            try:
                self.openocd_process = subprocess.Popen(cmd)
                self.openocd_process.wait()
            except KeyboardInterrupt:
                print(f"\n{Colors.YELLOW}Stopping OpenOCD server...{Colors.NC}")
                if self.openocd_process:
                    self.openocd_process.terminate()
                    self.openocd_process.wait()
                print(f"{Colors.GREEN}[OK] OpenOCD server stopped{Colors.NC}")
        
        return True
    
    def run(self):
        """Main function"""
        try:
            # Parse arguments
            args = self.parse_arguments()
            
            # Handle kill command
            if args.kill:
                self.kill_openocd()
                return 0
            
            # Determine OpenOCD path
            if args.openocd_path is None:
                args.openocd_path = self.default_openocd_path
            
            # Check if OpenOCD is available
            try:
                subprocess.run([args.openocd_path, '--version'], 
                             capture_output=True, timeout=2, check=True)
            except:
                print(f"{Colors.RED}Error: OpenOCD not found at: {args.openocd_path}{Colors.NC}")
                print(f"Please check if OpenOCD is installed")
                return 1
            
            # Find configuration file
            config_file = self.find_config_file(args.mode)
            
            # Start server
            success = self.start_server(
                args.openocd_path, config_file, 
                args.daemon, args.verbose
            )
            
            return 0 if success else 1
            
        except FileNotFoundError as e:
            print(f"{Colors.RED}Error: {e}{Colors.NC}")
            return 1
        except Exception as e:
            print(f"{Colors.RED}An error occurred: {e}{Colors.NC}")
            if args.verbose:
                import traceback
                traceback.print_exc()
            return 1

def main():
    """Program entry point"""
    server = OpenOCDServer()
    sys.exit(server.run())

if __name__ == '__main__':
    main()