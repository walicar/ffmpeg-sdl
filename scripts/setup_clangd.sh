#!/bin/bash

if [ ! -f ".clangd" ]; then
    echo "Creating .clangd"
    ffmpeg_cmd="-I$FFMPEG_DEV/include"
    sdl3_cmd="-I$SDL3_DEV/include"
    echo -e "CompileFlags:\r  Add: [$ffmpeg_cmd,$sdl3_cmd]" > ".clangd"
fi
