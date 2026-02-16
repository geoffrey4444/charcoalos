# Distributed under the MIT license.
# See LICENSE.txt for details.

function(add_platform_rpi_target target_name)
  set(options)
  set(one_value_args PLATFORM_INCLUDE_DIR)
  set(multi_value_args)
  cmake_parse_arguments(PLATFORM_RPI_TARGET "${options}" "${one_value_args}"
                        "${multi_value_args}" ${ARGN})

  set(platform_rpi_dir ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(platform_rpi_sources
    ${platform_rpi_dir}/IO.c
    ${platform_rpi_dir}/Info.c
    ${platform_rpi_dir}/Reboot.c)

  add_library(${target_name} OBJECT ${platform_rpi_sources})
  target_compile_options(${target_name} PRIVATE
    -ffreestanding
    -fno-builtin)
  target_compile_definitions(${target_name} PRIVATE TARGET_RPI=1)
  target_include_directories(${target_name} PRIVATE
    ${PLATFORM_RPI_TARGET_PLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR})
endfunction()
