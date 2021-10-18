#!/bin/sh

TMPDIR='/tmp/cc'

cd "$(dirname $0)"
mkdir "$TMPDIR" >/dev/null 2>&1

gcc -g cc.c -o "$TMPDIR/cc.elf"
