ffmpeg -f s16le -ar 44.1k -ac 1 -i "$1" -ar 48000 "$2"
