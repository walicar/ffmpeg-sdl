#!/bin/bash

if [ ! -f ".clangd" ]; then
    echo "Creating .clangd"
    ffmpeg_cmd="-I$FFMPEG_DEV/include"
    sdl2_cmd="-I$SDL2_DEV/include"
    echo -e "CompileFlags:\r  Add: [$ffmpeg_cmd,$sdl2_cmd]" > ".clangd"
fi
