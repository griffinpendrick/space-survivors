@echo off

if NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (call "shell.bat")

if not exist C:\ss\bin mkdir C:\ss\bin
pushd C:\ss\bin

set CompilerFlags= -nologo -Od -MDd -Zi -WX -W4 -Oi -IC:\raylib\src
set LinkerFlags= -incremental:no -opt:ref -LIBPATH:C:\raylib\src raylib.lib winmm.lib gdi32.lib user32.lib shell32.lib

del *.pdb > NUL 2> NUL
cl %CompilerFlags% C:\ss\src\ss.c -link %LinkerFlags%

popd