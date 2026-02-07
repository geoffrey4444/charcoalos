# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(arch_arm64 OBJECT
  Halt.c)
target_compile_options(arch_arm64 PRIVATE
  -ffreestanding
  -fno-builtin)
target_include_directories(arch_arm64 PRIVATE
  ${CMAKE_SOURCE_DIR})
