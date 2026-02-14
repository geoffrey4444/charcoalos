# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(arch_arm64 OBJECT
  ExceptionHandler.c
  ExceptionVectors.s
  Halt.c
  Info.c)
target_compile_options(arch_arm64 PRIVATE
  -mno-outline-atomics
  -ffreestanding
  -fno-builtin)
target_include_directories(arch_arm64 PRIVATE
  ${CMAKE_SOURCE_DIR})
