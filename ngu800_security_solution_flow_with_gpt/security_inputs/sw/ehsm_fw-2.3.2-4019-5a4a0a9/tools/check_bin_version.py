#!/usr/bin/env python3
# -*- coding:utf-8  -*-
"""
check version between bin file and config.h
"""

import sys
import struct
import re
from typing import Tuple
from pathlib import Path
import argparse as ap

VER_OFFSET_IN_BINARY = 0x80  # see this offset in .ld file
VER_LEN_IN_BINARY = 12


def get_version_from_header(config_h: str) -> Tuple[int, int, int, int, str]:
    content: str = Path(config_h).read_text(encoding="utf-8")
    major = re.search(r"""#define\s+EHSM_FW_VER_MAJOR\b\s+(\d+)""", content).group(1)
    minor = re.search(r"""#define\s+EHSM_FW_VER_MINOR\b\s+(\d+)""", content).group(1)
    patch = re.search(r"""#define\s+EHSM_FW_VER_PATCH\b\s+(\d+)""", content).group(1)
    pre_release = re.search(
        r'''#define\s+EHSM_FW_VER_PRE_RELEASE\b\s+"(.*)"''', content
    ).group(1)

    # the first 1 is type of firmware
    return (1, int(major), int(minor), int(patch), pre_release)


def get_version_from_bin(bin_file: str) -> Tuple[int, int, int, int, str]:
    content: bytes = Path(bin_file).read_bytes()
    ret: Tuple[int, int, int, int] = struct.unpack(
        ">BBBB",
        content[VER_OFFSET_IN_BINARY : VER_OFFSET_IN_BINARY + 4],
    )
    pre_release = (
        content[VER_OFFSET_IN_BINARY + 4 : VER_OFFSET_IN_BINARY + VER_LEN_IN_BINARY]
        .rstrip(b"\x00")
        .decode("utf-8")
    )
    return (*ret, pre_release)


def cli() -> ap.Namespace:
    parser = ap.ArgumentParser()
    parser.add_argument(
        "--header-version", "-V", required=True, help="version.h file path"
    )
    parser.add_argument("--bin", "-b", required=True, help="binary file path")
    return parser.parse_args(sys.argv[1:])


def main():
    args = cli()
    ver1 = get_version_from_header(args.header_version)
    ver2 = get_version_from_bin(args.bin)
    if ver1 != ver2:
        print("Different version between version.h and binary file:", file=sys.stderr)
        print(f"in {args.header_version}:", ver1, file=sys.stderr)
        print(f"in {args.bin}:", ver2, file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
