#!/bin/bash

mkdir -p ~/space-survivors/bin
pushd ~/space-survivors/bin > /dev/null

CompilerFlags="-Wall -Wextra -O0 -g -I/opt/homebrew/opt/raylib/include"
LinkerFlags="-L/opt/homebrew/opt/raylib/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo"

gcc $CompilerFlags ~/space-survivors/src/main.c -o main $LinkerFlags

popd > /dev/null
