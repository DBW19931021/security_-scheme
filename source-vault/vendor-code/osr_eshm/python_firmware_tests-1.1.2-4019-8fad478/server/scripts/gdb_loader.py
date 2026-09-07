#!/usr/bin/env python3
"""
GDB-based Flash Loader Script
Connects to OpenOCD GDB server and loads program
Compatible with Windows and Linux systems
"""

import os
import sys
import argparse
import subprocess
import time
import platform
import configparser
from pathlib import Path
from typing import Optional, Tuple

# ANSI color codes
class Colors:
    """Terminal color definitions"""
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    NC = '\033[0m'  # No Color
    
    @staticmethod
    def disable():
        """Disable colors in environments that don't support them"""
        Colors.RED = ''
        Colors.GREEN = ''
        Colors.YELLOW = ''
        Colors.BLUE = ''
        Colors.NC = ''

# Windows console color support detection
if platform.system() == 'Windows':
    try:
        import colorama
        colorama.init()
    except ImportError:
        # Disable colors on Windows if colorama is not available
        Colors.disable()

class GDBLoader:
    """GDB-based Program Loader"""
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.build_dir = self.script_dir.parent / 'build'
        self.supported_targets = ['cm3', 'm130']
        self.supported_modes = ['jtag', 'swd']
        self.gdb_executable = self.get_gdb_executable()
    
    def get_gdb_executable(self) -> str:
        """Get appropriate GDB executable based on target"""
        if platform.system() == 'Windows':
            # Check for RISC-V GDB
            riscv_gdb_paths = [
                r'riscv32-unknown-elf-gdb.exe',
                r'riscv32-elf-gdb.exe',
                r'riscv-none-embed-gdb.exe'
            ]
            
            for gdb in riscv_gdb_paths:
                try:
                    subprocess.run([gdb, '--version'], capture_output=True, timeout=2)
                    return gdb
                except:
                    continue
                    
            # Fallback to ARM GDB for CM3
            arm_gdb_paths = [
                r'arm-none-eabi-gdb.exe',
                r'arm-elf-gdb.exe'
            ]
            
            for gdb in arm_gdb_paths:
                try:
                    subprocess.run([gdb, '--version'], capture_output=True, timeout=2)
                    return gdb
                except:
                    continue
        
        # Linux/Unix default
        return 'gdb-multiarch'
    
    def parse_arguments(self) -> argparse.Namespace:
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description='GDB-based Flash Loader - Connects to OpenOCD GDB server',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog='''
Examples:
  %(prog)s                        # Use default config (m130, port 3339)
  %(prog)s -t cm3 -p 3335        # Connect to CM3 on port 3335
  %(prog)s -f custom.elf -r      # Load specific file and reset
  %(prog)s --gdb-path arm-none-eabi-gdb  # Use specific GDB
            '''
        )
        
        parser.add_argument('-t', '--target',
                          default='m130',
                          choices=self.supported_targets,
                          help='Target device (default: m130)')
        
        parser.add_argument('-p', '--port',
                          type=int,
                          default=3339,
                          help='GDB server port (default: 3339 for JTAG, 3335 for SWD)')
        
        parser.add_argument('-f', '--file',
                          type=str,
                          help='Specify file to load (default: .elf file in build directory)')
        
        parser.add_argument('-r', '--reset',
                          action='store_true',
                          help='Reset device after loading')
        
        parser.add_argument('-c', '--continue',
                          dest='continue_exec',
                          action='store_true',
                          help='Continue execution after loading')
        
        parser.add_argument('--gdb-path',
                          type=str,
                          default=None,
                          help='GDB executable path')
        
        parser.add_argument('--verbose',
                          action='store_true',
                          help='Show verbose output')
        
        return parser.parse_args()
    
    def find_elf_file(self, custom_file: Optional[str] = None) -> Path:
        """Find ELF file to load"""
        if custom_file:
            file_path = Path(custom_file)
            if not file_path.exists():
                raise FileNotFoundError(f"Specified file not found: {custom_file}")
            return file_path.absolute()
        
        # Find ELF files in build directory
        elf_files = list(self.build_dir.glob('*.elf'))
        
        if not elf_files:
            raise FileNotFoundError(f"No .elf files found in {self.build_dir}")
        
        # If multiple ELF files exist, choose the newest one
        if len(elf_files) > 1:
            elf_files.sort(key=lambda x: x.stat().st_mtime, reverse=True)
            print(f"{Colors.YELLOW}Multiple ELF files found, using newest: {elf_files[0].name}{Colors.NC}")
        
        return elf_files[0].absolute()
    
    def create_gdb_script(self, elf_file: Path, port: int, reset: bool, continue_exec: bool) -> str:
        """Create GDB command script"""
        commands = []
        
        # Connect to OpenOCD GDB server
        commands.append(f"target remote localhost:{port}")
        commands.append("")
        
        # Load ELF file symbols and data
        commands.append(f"file {str(elf_file).replace(chr(92), '/')}")
        commands.append("")
        
        # Reset and halt
        commands.append("monitor reset halt")
        commands.append("")
        
        # Load program to target
        commands.append("load")
        commands.append("")
        
        # Set up command history
        commands.append("set history save on")
        commands.append("set history filename .gdb_history")
        commands.append("")
        
        if reset:
            # Reset the target
            commands.append("monitor reset")
            commands.append("")
        
        if continue_exec:
            # Continue execution
            commands.append("continue")
        else:
            # Just resume without continuing in GDB
            commands.append("monitor resume")
            commands.append("quit")
        
        return '\n'.join(commands)
    
    def run_gdb(self, gdb_path: str, script_content: str, verbose: bool) -> Tuple[bool, str]:
        """Run GDB with script"""
        import tempfile
        
        # Create temporary GDB script file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.gdb', delete=False) as tmp_file:
            tmp_file.write(script_content)
            tmp_script = tmp_file.name
        
        try:
            # Build GDB command
            cmd = [gdb_path, '-batch', '-x', tmp_script]
            
            if verbose:
                print(f"{Colors.BLUE}Executing command: {' '.join(cmd)}{Colors.NC}")
                print(f"{Colors.BLUE}GDB script content:{Colors.NC}")
                print(script_content)
            
            # Execute GDB
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace',
                timeout=30
            )
            
            # Display output
            if verbose or result.returncode != 0:
                if result.stdout:
                    print(result.stdout)
                if result.stderr:
                    print(result.stderr, file=sys.stderr)
            
            return result.returncode == 0, result.stderr
            
        except subprocess.TimeoutExpired:
            return False, "GDB execution timeout"
        except Exception as e:
            return False, str(e)
        finally:
            # Clean up temporary file
            try:
                os.unlink(tmp_script)
            except:
                pass
    
    def run(self):
        """Main function"""
        try:
            # Parse arguments
            args = self.parse_arguments()
            
            # Determine GDB path
            if args.gdb_path is None:
                args.gdb_path = self.gdb_executable
            
            # Check if GDB is available
            try:
                subprocess.run([args.gdb_path, '--version'], 
                             capture_output=True, timeout=2, check=True)
            except:
                print(f"{Colors.RED}Error: GDB not found at: {args.gdb_path}{Colors.NC}")
                print(f"Please install GDB or specify path with --gdb-path")
                return 1
            
            # Find file to load
            load_file = self.find_elf_file(args.file)
            
            # Display configuration info
            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print(f"{Colors.GREEN}    GDB Flash Loader{Colors.NC}")
            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print(f"{Colors.YELLOW}Target Device:{Colors.NC} {args.target}")
            print(f"{Colors.YELLOW}GDB Server Port:{Colors.NC} {args.port}")
            print(f"{Colors.YELLOW}Load File:{Colors.NC} {load_file}")
            print(f"{Colors.YELLOW}GDB Path:{Colors.NC} {args.gdb_path}")
            
            if args.reset:
                print(f"{Colors.YELLOW}Reset:{Colors.NC} Enabled")
            if args.continue_exec:
                print(f"{Colors.YELLOW}Continue:{Colors.NC} Enabled")
            
            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print()
            
            # Create GDB script
            script_content = self.create_gdb_script(
                load_file, args.port, args.reset, args.continue_exec
            )
            
            # Run GDB
            print(f"{Colors.YELLOW}Connecting to OpenOCD GDB server on port {args.port}...{Colors.NC}")
            success, error_msg = self.run_gdb(
                args.gdb_path, script_content, args.verbose
            )
            
            if success:
                print()
                print(f"{Colors.GREEN}[OK] Program loaded successfully via GDB!{Colors.NC}")
                return 0
            else:
                print()
                print(f"{Colors.RED}[FAIL] GDB loading failed{Colors.NC}")
                if error_msg:
                    print(f"{Colors.RED}Error message: {error_msg}{Colors.NC}")
                return 1
                
        except FileNotFoundError as e:
            print(f"{Colors.RED}Error: {e}{Colors.NC}")
            return 1
        except KeyboardInterrupt:
            print(f"\n{Colors.YELLOW}Operation interrupted by user{Colors.NC}")
            return 130
        except Exception as e:
            print(f"{Colors.RED}An error occurred: {e}{Colors.NC}")
            if args.verbose:
                import traceback
                traceback.print_exc()
            return 1

def main():
    """Program entry point"""
    loader = GDBLoader()
    sys.exit(loader.run())

if __name__ == '__main__':
    main()