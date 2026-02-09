# Distributed under the MIT license.
# See LICENSE.txt for details.

add_library(platform_virt OBJECT
  IO.c
  Info.c)
target_compile_options(platform_virt PRIVATE
  -ffreestanding
  -fno-builtin)
target_compile_definitions(platform_virt PRIVATE TARGET_VIRT=1)
target_include_directories(platform_virt PRIVATE
  ${CMAKE_SOURCE_DIR})
