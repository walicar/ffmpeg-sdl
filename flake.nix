{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      ffmpegPath = pkgs.ffmpeg.dev;
      sdl3Path = pkgs.sdl3.dev;
    in {
      devShells.default = pkgs.mkShell {
        packages = with pkgs; [
          pkg-config
          ffmpeg
          sdl3
        ];
        shellHook = ''
          export SRC=$(pwd)

          export FFMPEG_DEV=${ffmpegPath}
          export SDL3_DEV=${sdl3Path}

          bash scripts/setup_clangd.sh
          bash scripts/setup_vscode.sh
        '';
      };
    }
  );
}
