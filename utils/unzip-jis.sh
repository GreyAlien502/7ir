#!/bin/bash
mkdir "$1".dir
bsdtar -xf "$1" -C "$1".dir
convmv -f shift-jis -t utf-8 -r "$1".dir --notest
format="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/format.sh
echo $format
"$format" "$1".dir
