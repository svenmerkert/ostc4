echo off
SET PATH=%PATH%;%CD%\tools\ninja

cmake -G "Eclipse CDT4 - Ninja"^
 -B %CD%/../OSTC4_Build/Debug ^
 -DCMAKE_BUILD_TYPE=Debug^
 -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j8"^
 -DCMAKE_TOOLCHAIN_FILE=cmake/arm-bare-metal.cmake^
 -S .
 
cmake -G "Eclipse CDT4 - Ninja"^
 -B %CD%/../OSTC4_Build/Release ^
 -DCMAKE_BUILD_TYPE=Release^
 -DCMAKE_ECLIPSE_MAKE_ARGUMENTS="-j8"^
 -DCMAKE_TOOLCHAIN_FILE=cmake/arm-bare-metal.cmake^
 -S .
pause