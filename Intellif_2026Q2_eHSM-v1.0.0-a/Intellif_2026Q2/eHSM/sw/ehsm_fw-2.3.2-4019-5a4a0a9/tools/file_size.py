#!/usr/bin/env python3
# -*- coding:utf-8-*-
"""

"""

import os
import sys
from pathlib import Path

file = sys.argv[1]
size = os.stat(file).st_size

print(f"{Path(file).name} size: {size/1024.0:.2f} KB")
