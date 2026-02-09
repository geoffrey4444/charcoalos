# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(platform_rpi OBJECT
  IO.c
  Info.c)
target_compile_options(platform_rpi PRIVATE
  -ffreestanding
  -fno-builtin)
target_compile_definitions(platform_rpi PRIVATE TARGET_RPI=1)
target_include_directories(platform_rpi PRIVATE
  ${CMAKE_SOURCE_DIR})
