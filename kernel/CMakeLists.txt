# Distributed under the MIT license.
# See LICENSE.txt for details.

function(add_kernel_common_target target_name)
  set(options)
  set(one_value_args PLATFORM_INCLUDE_DIR PLATFORM_DEFINE)
  set(multi_value_args)
  cmake_parse_arguments(KERNEL_TARGET "${options}" "${one_value_args}"
                        "${multi_value_args}" ${ARGN})

  set(kernel_dir ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(kernel_common_sources
    ${kernel_dir}/Console/IO.c
    ${kernel_dir}/Console/Shell.c
    ${kernel_dir}/Main/Lifecycle.c
    ${kernel_dir}/Main/Main.c
    ${kernel_dir}/Panic/Panic.c
    ${kernel_dir}/Panic/Restart.c
    ${kernel_dir}/String/String.c
    ${kernel_dir}/Time/Uptime.c)

  add_library(${target_name} OBJECT ${kernel_common_sources})
  target_compile_options(${target_name} PRIVATE
    -ffreestanding
    -fno-builtin)
  target_include_directories(${target_name} PRIVATE
    ${KERNEL_TARGET_PLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR})
  if(KERNEL_TARGET_PLATFORM_DEFINE)
    target_compile_definitions(${target_name} PRIVATE
      ${KERNEL_TARGET_PLATFORM_DEFINE})
  endif()
endfunction()
