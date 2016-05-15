#!/bin/bash
ffmpeg -i "$1" -f s16le -ar 44100 -ac 1 "$1".wav
