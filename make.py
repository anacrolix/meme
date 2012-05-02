#!/usr/bin/env python3

import argparse
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', choices=['clang', 'gcc'], default='gcc')
parser.add_argument('--no-optimize', dest='optimize', default=True, action='store_false')
parser.add_argument('--tracing', default=False, action='store_true')
parser.add_argument('--flags', default='', type=str)
args = parser.parse_args()

build_args = [
    args.compiler,
    '-o meme *.c',
    '`pkg-config --cflags --libs glib-2.0`',
    '-std=gnu1x -fplan9-extensions -fdiagnostics-show-option',
    '-Wall -Wextra -Werror-implicit-function-declaration -Wno-unused-parameter',]
if args.compiler != 'gcc':
    build_args.append('-Wno-incompatible-pointer-types')
build_args.append('-g')
if args.optimize:
    build_args += ['-DNDEBUG', '-Ofast', '-flto', '-fwhole-program']
if args.tracing:
    build_args.append('-DTRACE')
build_args.append(args.flags)

build_str = ' '.join(build_args)
print(build_str, file=sys.stderr)
raise SystemExit(subprocess.Popen(' '.join(build_args), shell=True).wait())

