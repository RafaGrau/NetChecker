#!/usr/bin/env python3
"""
increment_build.py
Pre-build script for NetChecker.
Reads src/version.h, increments VER_BUILD by 1, writes it back.

Usage (from VS pre-build event):
    python "$(ProjectDir)scripts\increment_build.py" "$(ProjectDir)src\version.h"
"""
import sys
import re
import os

def main():
    if len(sys.argv) < 2:
        print("Usage: increment_build.py <path-to-version.h>")
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.isfile(path):
        print(f"ERROR: {path} not found")
        sys.exit(1)

    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Match:  #define VER_BUILD   <number>
    pattern = r'(#define\s+VER_BUILD\s+)(\d+)'
    m = re.search(pattern, content)
    if not m:
        print("ERROR: VER_BUILD not found in version.h")
        sys.exit(1)

    old_build = int(m.group(2))
    new_build = old_build + 1
    new_content = re.sub(pattern, rf'\g<1>{new_build}', content)

    with open(path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"VER_BUILD: {old_build} -> {new_build}")

if __name__ == '__main__':
    main()
