# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(kernel_common OBJECT
  Console/IO.c
  Console/Shell.c
  Main/Main.c
  String/String.c)
target_compile_options(kernel_common PRIVATE
  -ffreestanding
  -fno-builtin)
target_include_directories(kernel_common PRIVATE
  ${CMAKE_SOURCE_DIR})
