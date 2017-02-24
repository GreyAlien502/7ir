#!/bin/bash
aplay -f S16_LE -c1 -r44100 "$@"
