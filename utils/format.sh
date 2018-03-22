#!/bin/bash
format="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/format.sh
if [ -d "$1" ]; then
	find "$1" \
		-type f \
		-regex '.*\(oto\.ini\|prefix\.map\|\.ust\|\.txt\)' \
		-exec "$format" {} \;
else
	iconv -f shift-jis -t utf-8 "$1" > "$1".utf-8
	mv -f "$1".utf-8 "$1"
	dos2unix "$1"
fi
