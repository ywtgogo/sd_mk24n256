set PATH=%PATH%;%~dp0\..\..\..\tools;%~dp0\..\..\..\toolchain\bin

make clean

make all -j4
