# OpenOCD Flash Loader

## Overview

Automated flash loading scripts for embedded targets via OpenOCD, supporting both JTAG and SWD modes.

## Files

| File | Description |
|------|-------------|
| `flash_loader.py` | Main flash loader script (cross-platform) |
| `flash_loader_config.ini` | Configuration file for OpenOCD paths |
| `gdb_loader.py` | GDB-based loader for debugging |
| `openocd_server.py` | OpenOCD server launcher |
| `openocd_jlink_jtag.cfg` | JTAG mode configuration (M130/RISC-V) |
| `openocd_jlink_swd.cfg` | SWD mode configuration (CM3/Cortex-M3) |

## Prerequisites

- **OpenOCD**: Version 0.12.0 or later (Path configured in `flash_loader_config.ini`)
- **J-Link**: SEGGER J-Link drivers installed
- **Python 3**: Version 3.6 or later

## Quick Start

### Basic Usage

```bash
# Load to M130 (RISC-V) via JTAG (default)
python flash_loader.py

# Load to CM3 (Cortex-M3) via SWD
python flash_loader.py -t cm3 -m swd

# Load with reset and verify
python flash_loader.py -r -v

# Load specific file
python flash_loader.py -f path/to/firmware.elf -r
```

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-t, --target` | Target device (cm3, m130) | m130 |
| `-m, --mode` | Connection mode (jtag, swd) | jtag |
| `-f, --file` | ELF file to load | auto-detect |
| `-r, --reset` | Reset after loading | disabled |
| `-v, --verify` | Verify after loading | disabled |
| `--verbose` | Show detailed output | disabled |
| `-h, --help` | Show help message | - |

## GDB Debug Mode

For debugging with breakpoints and single-stepping:

```bash
# Step 1: Start OpenOCD server
python openocd_server.py --daemon

# Step 2: Load via GDB
python gdb_loader.py

# Step 3: Stop server
python openocd_server.py --kill
```

## Platform Configuration

### M130 (RISC-V)
- Connection: JTAG
- Entry point: 0x10000000
- GDB port: 3339
- Config: `openocd_jlink_jtag.cfg`

### CM3 (Cortex-M3)
- Connection: SWD
- Entry point: Reset vector
- GDB port: 3335
- Config: `openocd_jlink_swd.cfg`

## Configuration

Edit `flash_loader_config.ini` to set OpenOCD path:

```ini
[OpenOCD]
windows_path = D:\A04_Works\07_ai_event\04_openocd\openocd-0.12.0\bin\openocd.exe
linux_path = /usr/bin/openocd

[Default]
target = m130
mode = jtag
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| J-Link not found | Check USB connection and drivers |
| Program doesn't run | Use `-r` flag to reset after loading |
| OpenOCD not found | Update path in `flash_loader_config.ini` |
| JTAG chain error | Check target power and connections |

## License

Part of the Python Embedded Tests project.