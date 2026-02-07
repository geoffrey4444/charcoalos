# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(platform_rpi OBJECT
  IO.c)
target_compile_options(platform_rpi PRIVATE
  -ffreestanding
  -fno-builtin)
target_include_directories(platform_rpi PRIVATE
  ${CMAKE_SOURCE_DIR})
