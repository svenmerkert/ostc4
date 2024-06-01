
#
# uncomment and set the arm embedded compiler path here!!!
# for example: SET(ARMTOOLPATH "C:/projects/arm_embedded")
#
# This should be set to the top-level dir with the following structure
# - arm-none-eabi
# - bin
# - lib
# - share
#

SET(ARMTOOLPATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/gcc-arm-none-eabi") 

if (NOT ARMTOOLPATH)
  message(FATAL_ERROR "Please set the toolchain path in arm-base-metal.cmake" )
endif()

set(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR "armv7")

#CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
#CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)
#CMAKE_FORCE_ASM_COMPILER(arm-none-eabi-as GNU)

set( CMAKE_ASM_COMPILER_ID_RUN TRUE )
set( CMAKE_ASM_COMPILER_ID GNU )
set( CMAKE_ASM_COMPILER_WORKS TRUE )
set( CMAKE_ASM_COMPILER_FORCED TRUE )
set( CMAKE_COMPILER_IS_GNUASM 1)

SET(CMAKE_C_COMPILER   ${ARMTOOLPATH}/bin/arm-none-eabi-gcc.exe)
SET(CMAKE_RC_COMPILER  ${ARMTOOLPATH}/bin/arm-none-eabi-gcc.exe)
SET(CMAKE_CXX_COMPILER ${ARMTOOLPATH}/bin/arm-none-eabi-g++.exe)
SET(CMAKE_ASM_COMPILER ${ARMTOOLPATH}/bin/arm-none-eabi-gcc.exe)

set(CMAKE_C_COMPILER_WORKS TRUE )
set(CMAKE_CXX_COMPILER_WORKS TRUE )
set( CMAKE_C_COMPILER_FORCED TRUE )
set( CMAKE_CXX_COMPILER_FORCED TRUE )

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  ${ARMTOOLPATH})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

