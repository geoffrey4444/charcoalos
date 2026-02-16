# Distributed under the MIT license.
# See LICENSE.txt for details.

function(add_platform_virt_target target_name)
  set(options)
  set(one_value_args PLATFORM_INCLUDE_DIR)
  set(multi_value_args)
  cmake_parse_arguments(PLATFORM_VIRT_TARGET "${options}" "${one_value_args}"
                        "${multi_value_args}" ${ARGN})

  set(platform_virt_dir ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(platform_virt_sources
    ${platform_virt_dir}/IO.c
    ${platform_virt_dir}/Info.c
    ${platform_virt_dir}/Reboot.c)

  add_library(${target_name} OBJECT ${platform_virt_sources})
  target_compile_options(${target_name} PRIVATE
    -ffreestanding
    -fno-builtin)
  target_compile_definitions(${target_name} PRIVATE TARGET_VIRT=1)
  target_include_directories(${target_name} PRIVATE
    ${PLATFORM_VIRT_TARGET_PLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR})
endfunction()
