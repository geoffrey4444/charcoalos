# Distributed under the MIT license.
# See LICENSE.txt for details.

function(add_arch_arm64_target target_name)
  set(options)
  set(one_value_args PLATFORM_INCLUDE_DIR PLATFORM_DEFINE)
  set(multi_value_args)
  cmake_parse_arguments(ARCH_TARGET "${options}" "${one_value_args}"
                        "${multi_value_args}" ${ARGN})

  set(arch_arm64_dir ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(arch_arm64_sources
    ${arch_arm64_dir}/ExceptionHandler.c
    ${arch_arm64_dir}/ExceptionVectors.s
    ${arch_arm64_dir}/Halt.c
    ${arch_arm64_dir}/Info.c
    ${arch_arm64_dir}/Interrupt.c)

  add_library(${target_name} OBJECT ${arch_arm64_sources})
  target_compile_options(${target_name} PRIVATE
    -mno-outline-atomics
    -ffreestanding
    -fno-builtin)
  target_include_directories(${target_name} PRIVATE
    ${ARCH_TARGET_PLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR})
  if(ARCH_TARGET_PLATFORM_DEFINE)
    target_compile_definitions(${target_name} PRIVATE
      ${ARCH_TARGET_PLATFORM_DEFINE})
  endif()
endfunction()
