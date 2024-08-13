#!/bin/bash

clang_path=$(which clangd)
settings=$(cat <<EOF
{
  "clang.path": "$clang_path"
}
EOF
)

root_dir=${SRC:-""}
vs_code_dir="$root_dir/.vscode"
vs_code_settings="$vs_code_dir/settings.json"

if [ ! -f "$vs_code_settings" ]; then
    echo "Creating .vscode/settings.json"
    mkdir -p "$vs_code_dir"
    echo "$settings" > "$vs_code_settings"
fi
