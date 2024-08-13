{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      ffmpegPath = pkgs.ffmpeg.dev;
      sdl2Path = pkgs.SDL2.dev;
    in {
      devShells.default = pkgs.mkShell {
        packages = with pkgs; [
          clang-tools
          pkg-config
          ffmpeg
          SDL2
        ];
        shellHook = ''
          export SRC=$(pwd)

          export FFMPEG_DEV=${ffmpegPath}
          export SDL2_DEV=${sdl2Path}

          bash scripts/setup_clangd.sh
          bash scripts/setup_vscode.sh
        '';
      };
    }
  );
}
