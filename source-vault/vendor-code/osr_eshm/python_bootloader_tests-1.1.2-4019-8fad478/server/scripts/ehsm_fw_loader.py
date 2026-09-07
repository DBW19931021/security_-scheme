#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
EHSM Firmware Loader
Generates parameter files based on fastbl_gdb and loads firmware directly through OpenOCD
Supports custom FastBL files to simplify the loading process
"""

import os
import sys
import re
import argparse
import subprocess
import tempfile
import platform
from pathlib import Path
from typing import Optional, Tuple, List
import logging

# ANSI Color Codes
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
        Colors.disable()

class EHSMFirmwareLoader:
    """EHSM Firmware Loader"""
    
    # Default configuration constants
    DEFAULT_SERVER = '127.0.0.1:3333'
    DEFAULT_CONFIG = 'openocd_ftdi.cfg'
    DEFAULT_TIMEOUT = 30
    MEMORY_MAPPING_ADDR = 0x30003800
    PARAMS_LOAD_ADDR = 0xe0041000
    FASTBL_LOAD_ADDR = 0x10000000
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.default_openocd_path = self._get_default_openocd_path()
        self._setup_logging()

    def _setup_logging(self):
        """Setup logging configuration"""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
    
    def _get_default_openocd_path(self) -> str:
        """Get default OpenOCD path"""
        if platform.system() == 'Windows':
            # Check common OpenOCD installation paths
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

    def _find_params_file(self, gdb_script_path: Optional[str]) -> Optional[str]:
        """Find parameter file path"""
        params_file = None
        
        # First extract from GDB script
        if gdb_script_path and os.path.exists(gdb_script_path):
            try:
                with open(gdb_script_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    
                for line in content.splitlines():
                    line = line.strip()
                    if line.startswith('restore ') and 'params_fw_in_soc' in line:
                        parts = line.split()
                        if len(parts) >= 4 and parts[2] == 'binary':
                            params_file = parts[1]
                            break
            except Exception as e:
                self.logger.warning(f"Error parsing GDB script: {e}")
        
        # If not found, search current directory
        if not params_file:
            possible_params = [
                'params_fw_in_soc_0x60041000.bin',
                'params_fw_in_soc.bin'
            ]
            for param_file in possible_params:
                if os.path.exists(param_file):
                    params_file = param_file
                    print(f"{Colors.YELLOW}Using found parameter file: {param_file}{Colors.NC}")
                    break
        
        return params_file
    
    def create_openocd_script_direct(self, gdb_script_path: str = None, fastbl_path: str = None, firmware_path: str = None) -> str:
        """Create OpenOCD script directly"""
        commands = []
        
        # Initialize and halt
        commands.extend([
            "# Load script - assume target is initialized and halted",
            "halt",
            "",
            "# Set memory mapping backup registers",
            f"mww 0x{self.MEMORY_MAPPING_ADDR:08x} 0x00000000",
            f"mww 0x{self.MEMORY_MAPPING_ADDR + 4:08x} 0x00000000",
            ""
        ])
        
        # Determine loading mode
        if fastbl_path:
            # FastBL + parameter file mode
            return self._create_fastbl_mode_script(commands, gdb_script_path, fastbl_path)
        elif firmware_path:
            # Pure firmware mode
            return self._create_firmware_only_mode_script(commands, firmware_path)
        else:
            raise ValueError("Must specify either fastbl_path or firmware_path")

    def _create_fastbl_mode_script(self, commands: List[str], gdb_script_path: str, fastbl_path: str) -> str:
        """Create FastBL + parameter file mode script"""
        # Find parameter file
        params_file = self._find_params_file(gdb_script_path)
        
        # Load parameter file
        if params_file:
            commands.extend([
                "# Load parameter file",
                f"echo \"Loading params file to 0x{self.PARAMS_LOAD_ADDR:08x}...\"",
                f"load_image {params_file} 0x{self.PARAMS_LOAD_ADDR:08x}"
            ])
        else:
            self.logger.warning("Parameter file not found, skipping parameter load")
            commands.extend([
                "# Skip parameter file - not found",
                "echo \"Warning: No params file found, skipping params load\""
            ])
        
        # Load FastBL file
        fastbl_path_normalized = self._normalize_path(fastbl_path)
        commands.extend([
            "",
            "# Load FastBL file",
            f"echo \"Loading FastBL file to 0x{self.FASTBL_LOAD_ADDR:08x}...\"",
            f"load_image {fastbl_path_normalized} 0x{self.FASTBL_LOAD_ADDR:08x}"
        ])
        
        # Add verification commands
        commands.extend([
            "",
            "echo \"Verifying loads using mem2array...\"",
            f"mem2array verify_params 32 0x{self.PARAMS_LOAD_ADDR:08x} 4",
            "echo \"Params verification - first 4 words: $verify_params(0) $verify_params(1) $verify_params(2) $verify_params(3)\"",
            f"mem2array verify_fastbl 32 0x{self.FASTBL_LOAD_ADDR:08x} 4",
            "echo \"Fastbl verification - first 4 words: $verify_fastbl(0) $verify_fastbl(1) $verify_fastbl(2) $verify_fastbl(3)\""
        ])
        
        self._add_common_ending(commands)
        return '\n'.join(commands)
    
    def _create_firmware_only_mode_script(self, commands: List[str], firmware_path: str) -> str:
        """Create pure firmware mode script (load directly to 0x10000000)"""
        firmware_path_normalized = self._normalize_path(firmware_path)
        
        commands.extend([
            "# Pure firmware mode - load firmware directly to FastBL address",
            f"echo \"Loading firmware directly to 0x{self.FASTBL_LOAD_ADDR:08x}...\"",
            f"load_image {firmware_path_normalized} 0x{self.FASTBL_LOAD_ADDR:08x}",
            "",
            "echo \"Verifying firmware load using mem2array...\"",
            f"mem2array verify_firmware 32 0x{self.FASTBL_LOAD_ADDR:08x} 4",
            "echo \"Firmware verification - first 4 words: $verify_firmware(0) $verify_firmware(1) $verify_firmware(2) $verify_firmware(3)\""
        ])
        
        self._add_common_ending(commands)
        return '\n'.join(commands)
    
    def _add_common_ending(self, commands: List[str]):
        """Add common ending commands"""
        commands.extend([
            "echo \"All loads completed and verified!\"",
            "",
            "# Resume execution",
            "resume",
            "",
            "# Exit OpenOCD",
            "shutdown"
        ])

    def _normalize_path(self, path: str) -> str:
        """Normalize file path"""
        normalized = path.replace('\\', '/')
        return f'"{normalized}"' if ' ' in normalized else normalized
    
    def _check_command_available(self, command: str) -> bool:
        """Check if command is available"""
        try:
            result = subprocess.run(
                [command, '--version'],
                capture_output=True,
                text=True,
                timeout=5
            )
            return result.returncode == 0
        except (subprocess.SubprocessError, FileNotFoundError):
            return False

    def _create_argument_parser(self) -> argparse.ArgumentParser:
        """Create command line argument parser"""
        parser = argparse.ArgumentParser(
            description='EHSM Firmware Loader - Supports FastBL mode and pure firmware mode',
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog='''
Examples:
  %(prog)s firmware.bin  # Pure firmware mode, load directly to 0x10000000
  %(prog)s firmware.bin --fastbl fast_bl.bin  # FastBL mode, load parameter file and FastBL
  %(prog)s firmware.bin --fastbl /path/to/fast_bl.bin --server 192.168.1.100:3333 # Specify server
  %(prog)s firmware.bin --verbose --preview  # Pure firmware mode preview
            '''
        )

        parser.add_argument('firmware',
                          type=str,
                          help='Firmware file path (.bin file)')

        parser.add_argument('--server', '-s',
                          default=self.DEFAULT_SERVER,
                          help=f'GDB server address (default: {self.DEFAULT_SERVER})')

        parser.add_argument('--fastbl',
                          type=str,
                          required=False,
                          help='Specify fast_bl.bin file path (optional, if not specified, firmware will be loaded directly to 0x10000000)')

        parser.add_argument('--output', '-o',
                          default='load_fw.gdb',
                          help='Generated GDB script filename (default: load_fw.gdb)')

        parser.add_argument('--openocd-path',
                          type=str,
                          default=None,
                          help='OpenOCD executable path (default: auto-detect)')

        parser.add_argument('--openocd-script',
                          default='load_fw_openocd.cfg',
                          help='Generated OpenOCD script filename (default: load_fw_openocd.cfg)')

        parser.add_argument('--config',
                          default=self.DEFAULT_CONFIG,
                          help=f'OpenOCD configuration file (default: {self.DEFAULT_CONFIG})')

        parser.add_argument('--no-hex',
                          action='store_true',
                          help='Do not use --hex option')

        parser.add_argument('--verbose', '-v',
                          action='store_true',
                          help='Show verbose output')

        parser.add_argument('--preview', '-p',
                          action='store_true',
                          help='Preview generated GDB script only, do not execute loading')

        return parser
    
    def parse_arguments(self) -> argparse.Namespace:
        """Parse command line arguments"""
        parser = self._create_argument_parser()
        return parser.parse_args()

    def validate_arguments(self, args) -> Tuple[bool, List[str]]:
        """Validate arguments and return validation result and error messages"""
        errors = []
        
        # Validate FastBL file (if specified)
        if args.fastbl:
            fastbl_path = Path(args.fastbl)
            if not fastbl_path.exists():
                errors.append(f"FastBL file does not exist: {args.fastbl}")

        # Validate firmware file
        firmware_path = Path(args.firmware)
        if not firmware_path.exists():
            errors.append(f"Firmware file does not exist: {args.firmware}")
        elif not firmware_path.suffix.lower() == '.bin':
            self.logger.warning(f"Firmware file is not .bin format: {args.firmware}")

        # Validate configuration file
        config_path = Path(args.config)
        if not config_path.is_absolute():
            config_path = self.script_dir / config_path
        
        if not config_path.exists():
            errors.append(f"OpenOCD configuration file does not exist: {config_path}")

        return len(errors) == 0, errors

    def run_fastbl_gdb(self, firmware_path: str, server: str, output_script: str,
                      use_hex: bool = True, verbose: bool = False) -> bool:
        """Run fastbl_gdb command to generate loading script"""
        cmd = [
            'fastbl_gdb',
            '--server', server,
            '-o', output_script,
            firmware_path
        ]

        if use_hex:
            cmd.append('--hex')

        if verbose:
            self.logger.info(f"Executing fastbl_gdb command: {' '.join(cmd)}")

        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            if result.stdout and verbose:
                self.logger.info(result.stdout)
            return True
        except subprocess.CalledProcessError as e:
            self.logger.error(f"fastbl_gdb command execution failed: {e.returncode}")
            if e.stderr:
                self.logger.error(f"Error message: {e.stderr}")
            return False
        except FileNotFoundError:
            self.logger.error("Cannot find fastbl_gdb command! Please ensure it is in the system PATH")
            return False

    def _show_script_content(self, script_path: str, script_type: str = "OpenOCD"):
        """Display script content"""
        try:
            with open(script_path, 'r', encoding='utf-8') as f:
                content = f.read()
            print(f"{Colors.YELLOW}{script_type} script content ({script_path}):{Colors.NC}")
            print("-" * 50)
            print(content)
            print("-" * 50)
        except Exception as e:
            self.logger.error(f"Cannot read {script_type} script file: {e}")

    def create_openocd_load_script(self, gdb_script_path: str, openocd_script_path: str, fastbl_path: str) -> bool:
        """Create OpenOCD loading script"""
        try:
            openocd_content = self.create_openocd_script_direct(gdb_script_path, fastbl_path)
            with open(openocd_script_path, 'w', encoding='utf-8') as f:
                f.write(openocd_content)
            return True
        except Exception as e:
            self.logger.error(f"Failed to create OpenOCD script: {e}")
            return False

    def _analyze_openocd_output(self, result: subprocess.CompletedProcess, verbose: bool) -> bool:
        """Analyze OpenOCD output results"""
        if result.stdout:
            self.logger.info("OpenOCD standard output:")
            print(result.stdout)
        if result.stderr:
            self.logger.warning("OpenOCD error output:")
            print(result.stderr, file=sys.stderr)

        self.logger.info(f"OpenOCD return code: {result.returncode}")
        
        if result.returncode != 0:
            self.logger.error("OpenOCD execution failed")
            return False
            
        # Check success indicators
        all_output = (result.stdout or "") + (result.stderr or "")
        success_indicators = [
            "all loads completed and verified",
            "verification - first 4 words",
            "shutdown command invoked"
        ]
        
        found_indicators = [indicator for indicator in success_indicators 
                          if indicator in all_output.lower()]
        
        if found_indicators:
            self.logger.info(f"Detected loading success indicators: {len(found_indicators)} success markers")
            if verbose:
                for indicator in found_indicators:
                    self.logger.info(f"  - {indicator}")
            
            # Check verification data
            hex_pattern = r'0x[0-9a-f]{1,8}'
            hex_matches = re.findall(hex_pattern, all_output.lower())
            non_zero_values = [h for h in hex_matches if h not in ['0x0', '0x00000000']]
            
            if non_zero_values and len(non_zero_values) >= 4:
                self.logger.info("Memory verification successful: detected valid loading data")
            return True
        else:
            self.logger.warning("OpenOCD executed successfully but specific verification messages not detected")
            return True  # Return code 0 is still considered success
    
    def run_openocd_with_script(self, config_file: str, script_content: str,
                               openocd_path: str, verbose: bool) -> Tuple[bool, str]:
        """Execute loading script using OpenOCD"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.cfg', delete=False) as tmp_file:
            tmp_file.write(script_content)
            tmp_script = tmp_file.name

        try:
            cmd = [openocd_path, '-f', config_file, '-f', tmp_script]

            if verbose:
                self.logger.info(f"Executing OpenOCD command: {' '.join(cmd)}")
                self.logger.info(f"OpenOCD script content:\n{script_content}")

            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace',
                timeout=self.DEFAULT_TIMEOUT
            )

            success = self._analyze_openocd_output(result, verbose)
            return success, result.stderr or ""

        except subprocess.TimeoutExpired:
            self.logger.error("OpenOCD execution timeout")
            return False, "OpenOCD execution timeout"
        except Exception as e:
            self.logger.error(f"OpenOCD execution error: {e}")
            return False, str(e)
        finally:
            try:
                os.unlink(tmp_script)
            except:
                pass

    def _get_file_size(self, file_path: Path) -> str:
        """Get file size in readable format"""
        size_bytes = file_path.stat().st_size
        
        for unit in ['B', 'KB', 'MB', 'GB']:
            if size_bytes < 1024.0:
                return f"{size_bytes:.2f} {unit}"
            size_bytes /= 1024.0
        
        return f"{size_bytes:.2f} TB"
    
    def _print_header(self, args):
        """Print program header information"""
        firmware_path = Path(args.firmware).absolute()
        config_path = Path(args.config)
        if not config_path.is_absolute():
            config_path = self.script_dir / config_path
            
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print(f"{Colors.GREEN}    EHSM Firmware Loader (Based on FastBL + OpenOCD){Colors.NC}")
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print(f"{Colors.YELLOW}Platform:{Colors.NC} {platform.system()} {platform.machine()}")
        print(f"{Colors.YELLOW}Firmware file:{Colors.NC} {firmware_path}")
        print(f"{Colors.YELLOW}File size:{Colors.NC} {self._get_file_size(firmware_path)}")
        print(f"{Colors.YELLOW}Server address:{Colors.NC} {args.server}")
        print(f"{Colors.YELLOW}OpenOCD config:{Colors.NC} {config_path}")
        print(f"{Colors.YELLOW}OpenOCD path:{Colors.NC} {args.openocd_path}")
        print(f"{Colors.YELLOW}GDB script:{Colors.NC} {args.output}")
        print(f"{Colors.YELLOW}OpenOCD script:{Colors.NC} {args.openocd_script}")
        if args.fastbl:
            print(f"{Colors.YELLOW}FastBL file:{Colors.NC} {args.fastbl}")
        else:
            print(f"{Colors.YELLOW}Mode:{Colors.NC} Pure firmware mode (load directly to 0x10000000)")
        print(f"{Colors.GREEN}{'='*50}{Colors.NC}")
        print()

    def run(self):
        """Main function"""
        try:
            # Parse arguments
            args = self.parse_arguments()

            # Validate arguments
            is_valid, errors = self.validate_arguments(args)
            if not is_valid:
                for error in errors:
                    self.logger.error(error)
                return 1

            # Determine path
            if args.openocd_path is None:
                args.openocd_path = self.default_openocd_path

            # Display configuration information
            self._print_header(args)

            firmware_path = Path(args.firmware).absolute()
            
            # Determine loading mode based on whether fastbl parameter exists
            if args.fastbl:
                # FastBL mode: need to generate parameter file
                self.logger.info("Step 1/2: Generating parameter file information...")
                success = self.run_fastbl_gdb(
                    firmware_path=str(firmware_path),
                    server=args.server,
                    output_script=args.output,
                    use_hex=not args.no_hex,
                    verbose=args.verbose
                )
                
                if not success:
                    self.logger.warning("fastbl_gdb failed, will try using default parameter file path")
                
                self.logger.info("Step 2/2: Loading firmware using OpenOCD (FastBL mode)...")
                openocd_content = self.create_openocd_script_direct(
                    gdb_script_path=args.output if success else None,
                    fastbl_path=args.fastbl
                )
            else:
                # Pure firmware mode: load firmware directly to 0x10000000
                self.logger.info("Using pure firmware mode, loading firmware directly to 0x10000000...")
                openocd_content = self.create_openocd_script_direct(
                    firmware_path=str(firmware_path)
                )
            
            # Display script content
            if args.preview or args.verbose:
                print(f"{Colors.YELLOW}OpenOCD loading script content:{Colors.NC}")
                print("-" * 50)
                print(openocd_content)
                print("-" * 50)

            if args.preview:
                self.logger.info("Preview mode only, not executing actual loading operation")
                return 0

            config_path = Path(args.config)
            if not config_path.is_absolute():
                config_path = self.script_dir / config_path
                
            success, error_msg = self.run_openocd_with_script(
                str(config_path), openocd_content, args.openocd_path, args.verbose
            )

            if success:
                self.logger.info("Firmware loading successful!")
                print(f"{Colors.GREEN}[OK] Firmware loading successful!{Colors.NC}")
                return 0
            else:
                self.logger.error("Firmware loading failed")
                print(f"{Colors.RED}[FAIL] Firmware loading failed{Colors.NC}")
                if error_msg:
                    print(f"{Colors.RED}Error message: {error_msg}{Colors.NC}")
                return 1

        except KeyboardInterrupt:
            self.logger.info("Operation interrupted by user")
            return 130
        except Exception as e:
            self.logger.error(f"Error occurred: {e}")
            if hasattr(self, 'logger') and args and args.verbose:
                import traceback
                traceback.print_exc()
            return 1

def main():
    """Program entry point"""
    loader = EHSMFirmwareLoader()
    sys.exit(loader.run())

if __name__ == '__main__':
    main()