#!/usr/bin/env python3
"""
OpenOCD Flash Loader Script (Cross-platform)
Supports JTAG/JLink and SWD connection modes
Compatible with Windows and Linux systems
"""

import os
import sys
import argparse
import subprocess
import tempfile
import platform
import configparser
from pathlib import Path
from typing import Optional, Tuple

# ANSI 颜色代码
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

class OpenOCDLoader:
    """OpenOCD Program Loader"""

    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.build_dir = self.script_dir.parent / 'build'
        self.supported_targets = ['cm3', 'm130']
        self.supported_modes = ['jtag', 'swd']
        # Default OpenOCD path for Windows
        self.default_openocd_path = self.get_default_openocd_path()
        self.target_type = None

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
            elif system == 'Linux' and config.has_option('OpenOCD', 'linux_path'):
                path = config.get('OpenOCD', 'linux_path')
                if Path(path).exists():
                    return path
            elif system == 'Darwin' and config.has_option('OpenOCD', 'macos_path'):
                path = config.get('OpenOCD', 'macos_path')
                if Path(path).exists():
                    return path

        # Fallback to hardcoded paths
        if platform.system() == 'Windows':
            # Check common OpenOCD installation paths on Windows
            openocd_paths = [
                r'D:\A04_Works\07_ai_event\04_openocd\openocd-0.12.0\bin\openocd.exe',
                r'C:\openocd\bin\openocd.exe',
                r'C:\Program Files\OpenOCD\bin\openocd.exe',
                'openocd.exe'  # Try system PATH
            ]
            for path in openocd_paths:
                if Path(path).exists():
                    return path
        return 'openocd'  # Default to system PATH

    def parse_arguments(self) -> argparse.Namespace:
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description='OpenOCD Flash Loader - Supports JTAG/JLink and SWD modes',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog='''
Examples:
  %(prog)s                        # Use default config (m130, jtag)
  %(prog)s -t cm3 -m swd         # Load to CM3 using SWD mode
  %(prog)s -f custom.elf -r      # Load specific file and reset
  %(prog)s --verbose             # Show verbose debug output
            '''
        )

        parser.add_argument('-t', '--target',
                          default='m130',
                          choices=self.supported_targets,
                          help='Target device (default: m130)')

        parser.add_argument('-m', '--mode',
                          default='jtag',
                          choices=self.supported_modes,
                          help='Connection mode (default: jtag)')

        parser.add_argument('-f', '--file',
                          type=str,
                          help='Specify file to load (default: .elf file in build directory)')

        parser.add_argument('-r', '--reset',
                          action='store_true',
                          help='Reset device after loading')

        parser.add_argument('-v', '--verify',
                          action='store_true',
                          help='Verify loaded program')

        parser.add_argument('--verbose',
                          action='store_true',
                          help='Show verbose output')

        parser.add_argument('--openocd-path',
                          type=str,
                          default=None,
                          help='OpenOCD executable path (default: auto-detect)')

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

    def get_file_size(self, file_path: Path) -> str:
        """Get file size in readable format"""
        size_bytes = file_path.stat().st_size

        for unit in ['B', 'KB', 'MB', 'GB']:
            if size_bytes < 1024.0:
                return f"{size_bytes:.2f} {unit}"
            size_bytes /= 1024.0

        return f"{size_bytes:.2f} TB"

    def get_elf_entry_point(self, elf_file: Path) -> int:
        """Get entry point from ELF file"""
        try:
            import struct
            with open(elf_file, 'rb') as f:
                f.seek(24)  # e_entry offset in ELF header
                entry = struct.unpack('<I', f.read(4))[0]
                return entry
        except:
            # Default entry points
            return 0x00000000 if self.target_type == 'cm3' else 0x10000000
    
    def create_openocd_script(self, load_file: Path, verify: bool, reset: bool) -> str:
        """Create OpenOCD command script based on .gdbinit reference"""
        commands = []

        # Initialize if not already done (needed for CM3/SWD)
        commands.append("# Initialize target")
        commands.append("init")
        commands.append("")
        
        # The config file already does init and halt, so we just need to ensure halt state
        commands.append("# Ensure target is halted")
        commands.append("halt")
        commands.append("wait_halt 5000")  # Wait up to 5 seconds for halt
        commands.append("")
        
        # Reset the target to clean state
        commands.append("# Reset to clean state")
        commands.append("reset init")  # Reset and halt at reset vector
        commands.append("wait_halt 5000")
        commands.append("")
        
        # Load program using load_image
        commands.append("# Load program")
        # Convert backslashes to forward slashes for Windows paths
        load_path = str(load_file).replace('\\', '/')
        # Use load_image without base address - let ELF headers determine placement
        commands.append(f'load_image "{load_path}"')
        commands.append("")
        
        # Verify program
        if verify:
            commands.append("# Verify loaded program")
            commands.append(f'verify_image "{load_path}"')
            commands.append("")
        
        # Set PC to entry point based on target type
        entry_point = self.get_elf_entry_point(load_file)
        commands.append("# Set PC to entry point")
        
        if self.target_type == 'cm3':
            # For Cortex-M3, don't set PC directly - let reset vector handle it
            commands.append("# CM3 uses reset vector from 0x00000004")
        else:
            # For RISC-V, set PC directly
            commands.append(f"reg pc 0x{entry_point:08x}")  # Entry point from ELF header
        commands.append("")
        
        # Reset or resume execution
        if reset:
            commands.append("# Reset and run")
            commands.append("reset run")
        else:
            commands.append("# Resume execution")
            commands.append("resume")
        
        # Wait a bit before exit to ensure commands complete
        commands.append("")
        commands.append("# Wait before exit")
        commands.append("sleep 1000")  # Wait 1 second
        
        # Exit
        commands.append("")
        commands.append("# Exit OpenOCD")
        commands.append("shutdown")
        
        return '\n'.join(commands)

    def check_openocd(self, openocd_path: str) -> bool:
        """Check if OpenOCD is available"""
        try:
            result = subprocess.run(
                [openocd_path, '--version'],
                capture_output=True,
                text=True,
                timeout=5
            )
            return result.returncode == 0
        except (subprocess.SubprocessError, FileNotFoundError):
            return False

    def run_openocd(self, config_file: Path, script_content: str,
                   openocd_path: str, verbose: bool) -> Tuple[bool, str]:
        """Run OpenOCD"""
        # Create temporary script file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.cfg', delete=False) as tmp_file:
            tmp_file.write(script_content)
            tmp_script = tmp_file.name

        try:
            # Build OpenOCD command
            cmd = [openocd_path, '-f', str(config_file), '-f', tmp_script]

            if verbose:
                print(f"{Colors.BLUE}Executing command: {' '.join(cmd)}{Colors.NC}")
                print(f"{Colors.BLUE}Script content:{Colors.NC}")
                print(script_content)

            # Execute OpenOCD with UTF-8 encoding
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
            return False, "OpenOCD execution timeout"
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
            
            # Store target type for later use
            self.target_type = args.target

            # Determine OpenOCD path
            if args.openocd_path is None:
                args.openocd_path = self.default_openocd_path

            # Check OpenOCD
            if not self.check_openocd(args.openocd_path):
                print(f"{Colors.RED}Error: OpenOCD is not available at: {args.openocd_path}{Colors.NC}")
                print(f"Please check if OpenOCD is installed at:")
                print(f"  D:\\A04_Works\\07_ai_event\\04_openocd\\openocd-0.12.0\\bin\\openocd.exe")
                print(f"Or use --openocd-path parameter to specify OpenOCD path")
                return 1

            # Find configuration file
            config_file = self.find_config_file(args.mode)

            # Find file to load
            load_file = self.find_elf_file(args.file)

            # Display configuration info
            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print(f"{Colors.GREEN}    OpenOCD Flash Loader (Cross-platform){Colors.NC}")
            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print(f"{Colors.YELLOW}Platform:{Colors.NC} {platform.system()} {platform.machine()}")
            print(f"{Colors.YELLOW}Target Device:{Colors.NC} {args.target}")
            print(f"{Colors.YELLOW}Connection Mode:{Colors.NC} {args.mode}")
            print(f"{Colors.YELLOW}Config File:{Colors.NC} {config_file}")
            print(f"{Colors.YELLOW}Load File:{Colors.NC} {load_file}")
            print(f"{Colors.YELLOW}File Size:{Colors.NC} {self.get_file_size(load_file)}")
            print(f"{Colors.YELLOW}OpenOCD Path:{Colors.NC} {args.openocd_path}")

            if args.verify:
                print(f"{Colors.YELLOW}Verify Mode:{Colors.NC} Enabled")
            if args.reset:
                print(f"{Colors.YELLOW}Reset Mode:{Colors.NC} Enabled")

            print(f"{Colors.GREEN}{'='*40}{Colors.NC}")
            print()

            # Create OpenOCD script
            script_content = self.create_openocd_script(
                load_file, args.verify, args.reset
            )

            # Run OpenOCD
            print(f"{Colors.YELLOW}Starting OpenOCD...{Colors.NC}")
            success, error_msg = self.run_openocd(
                config_file, script_content, args.openocd_path, args.verbose
            )

            if success:
                print()
                print(f"{Colors.GREEN}[OK] Program loaded successfully!{Colors.NC}")
                return 0
            else:
                print()
                print(f"{Colors.RED}[FAIL] Program loading failed{Colors.NC}")
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
    loader = OpenOCDLoader()
    sys.exit(loader.run())

if __name__ == '__main__':
    main()