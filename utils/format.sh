#!/bin/bash
iconv -f shift-jis -t utf-8 "$1" > "$1".utf-8
mv -f "$1".utf-8 "$1"
dos2unix "$1"
