#!/bin/bash

mkdir -p ~/space-survivors/bin
pushd ~/space-survivors/bin > /dev/null

CompilerFlags="-Wall -Wextra -O0 -g -I/opt/homebrew/opt/raylib/include"
LinkerFlags="-L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"

gcc $CompilerFlags ~/space-survivors/src/ss.c -o ss $LinkerFlags

popd > /dev/null
