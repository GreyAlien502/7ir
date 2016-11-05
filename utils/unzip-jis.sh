#!/bin/bash
mkdir "$1".dir
cd "$1".dir
bsdtar -xf ../"$1"
convmv -f shift-jis -t utf-8 -r . --notest
