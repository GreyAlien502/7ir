#!/bin/bash
iconv -f shift-jis -t utf-8 "$1" > "$2"
dos2unix "$2"
