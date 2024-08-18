# ffmpeg-sdl
Decode and display the first few frames of Big Buck Bunny using SDL and ffmpeg

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
git clone https://github.com/walicar/ffmpeg-sdl
code ffmpeg-sdl
direnv allow
```
