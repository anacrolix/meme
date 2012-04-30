#!/usr/bin/env python 

import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', choices=['clang', 'gcc'], default='gcc')
parser.add_argument('--no-optimize', dest='optimize', default=False, action='store_false')
args = parser.parse_args()

build_args = [
    args.compiler,
    '-o meme *.c',
    '`pkg-config --cflags --libs glib-2.0`',
    '-std=gnu1x -fplan9-extensions -fdiagnostics-show-option',
    '-Werror-implicit-function-declaration -Wno-unused-parameter',]
if args.compiler != 'gcc':
    build_args.append('-Wno-incompatible-pointer-types')
build_args.append('-g')
if args.optimize:
    build_args += ['-DNDEBUG', '-Ofast']

raise SystemExit(subprocess.Popen(' '.join(build_args), shell=True).wait())

