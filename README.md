# ffmpeg-sdl
GOAL: Using ffmpeg to decode a frame and render it using SDL

Current program will use an h264 codec to decode 6 frames of big buck bunny and save it as a .pgm file.

## Required Assets
Uses `bbb_sunflower_1080p_30fps_normal.mp4` from [this website](https://peach.blender.org/download/), renamed to `bbb.mp4`. `bbb.mp4` is already encoded with encoded in H264.

## Usage
```sh
make && ./main assets/bbb.mp4
```

## Setup Your Dev Environment
The recommended way to start coding on this project is to use:
- nix
- direnv
- VSCode, and download the clangd extension

```sh
$ git clone ffmpeg-sdl
$ code ffmpeg-sdl
$ direnv allow
```
