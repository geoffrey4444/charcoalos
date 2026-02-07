# Distributed under the MIT license.
# See LICENSE.txt for details.

add_test(
  NAME symbols-minimal-virt
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DINPUT_ELF=${VIRT_ELF}
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_virt_elf_symbols.cmake)
set_tests_properties(symbols-minimal-virt PROPERTIES DEPENDS minimal-virt)

add_test(
  NAME symbols-minimal-rpi-img
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DOBJCOPY=${AARCH64_TOOLCHAIN_PREFIX}objcopy
          -DINPUT_IMG=${RPI_IMG}
          -DWRAPPED_ELF=${CMAKE_CURRENT_BINARY_DIR}/kernel8_img_wrapped.elf
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_rpi_img_symbols.cmake)
set_tests_properties(symbols-minimal-rpi-img PROPERTIES DEPENDS minimal-rpi)

add_test(
  NAME symbols-minimal-rpi-elf
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DINPUT_ELF=${RPI_ELF}
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_rpi_elf_symbols.cmake)
set_tests_properties(symbols-minimal-rpi-elf PROPERTIES DEPENDS minimal-rpi)

add_test(
  NAME symbols-refactored-virt
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DINPUT_ELF=${REF_VIRT_ELF}
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_refactored_virt_elf_symbols.cmake)
set_tests_properties(symbols-refactored-virt PROPERTIES DEPENDS refactored-virt)

add_test(
  NAME symbols-refactored-rpi-img
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DOBJCOPY=${AARCH64_TOOLCHAIN_PREFIX}objcopy
          -DINPUT_IMG=${REF_RPI_IMG}
          -DWRAPPED_ELF=${CMAKE_CURRENT_BINARY_DIR}/refactored_kernel8_img_wrapped.elf
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_rpi_img_symbols.cmake)
set_tests_properties(symbols-refactored-rpi-img PROPERTIES DEPENDS refactored-rpi)

add_test(
  NAME symbols-refactored-rpi-elf
  COMMAND ${CMAKE_COMMAND}
          -DOBJDUMP=${AARCH64_TOOLCHAIN_PREFIX}objdump
          -DINPUT_ELF=${REF_RPI_ELF}
          -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_refactored_rpi_elf_symbols.cmake)
set_tests_properties(symbols-refactored-rpi-elf PROPERTIES DEPENDS refactored-rpi-elf)

if(QEMU_SYSTEM_AARCH64)
  add_test(
    NAME smoke-refactored-virt-console
    COMMAND ${CMAKE_COMMAND}
            -DQEMU_BIN=${QEMU_SYSTEM_AARCH64}
            -DINPUT_ELF=${REF_VIRT_ELF}
            -DQEMU_MACHINE=virt
            -DQEMU_CPU=max
            -DQEMU_MEMORY=${VIRT_QEMU_MEMORY}
            -DEXPECTED_SUBSTRING=Welcome\ to\ CharcoalOS.
            -DSMOKE_TIMEOUT_SEC=2
            -P ${CMAKE_SOURCE_DIR}/tests/Helpers/check_virt_console_smoke.cmake)
  set_tests_properties(smoke-refactored-virt-console PROPERTIES DEPENDS refactored-virt)
endif()

find_program(HOST_CC NAMES clang cc gcc)
if(HOST_CC AND UNITY_RESOLVED_SOURCE_DIR)
  function(add_host_unity_test)
    set(options)
    set(one_value_args NAME)
    set(multi_value_args SOURCES DEPENDS INCLUDE_DIRS)
    cmake_parse_arguments(HOST_TEST "${options}" "${one_value_args}"
                          "${multi_value_args}" ${ARGN})

    if(NOT HOST_TEST_NAME)
      message(FATAL_ERROR "add_host_unity_test requires NAME")
    endif()
    if(NOT HOST_TEST_SOURCES)
      message(FATAL_ERROR "add_host_unity_test requires SOURCES")
    endif()

    set(test_bin ${CMAKE_CURRENT_BINARY_DIR}/${HOST_TEST_NAME})
    set(include_flags -I${CMAKE_SOURCE_DIR} -I${UNITY_RESOLVED_SOURCE_DIR}/src)
    foreach(include_dir IN LISTS HOST_TEST_INCLUDE_DIRS)
      list(APPEND include_flags -I${include_dir})
    endforeach()

    add_custom_command(
      OUTPUT ${test_bin}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
      COMMAND ${HOST_CC}
              -std=c11
              -Wall
              -Wextra
              ${include_flags}
              ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.c
              ${HOST_TEST_SOURCES}
              -o ${test_bin}
      DEPENDS
        ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.c
        ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.h
        ${HOST_TEST_SOURCES}
        ${HOST_TEST_DEPENDS}
      COMMAND_EXPAND_LISTS
      VERBATIM)

    set(test_build_target ${HOST_TEST_NAME}-build)
    add_custom_target(${test_build_target} ALL DEPENDS ${test_bin})
    add_test(NAME ${HOST_TEST_NAME} COMMAND ${test_bin})
    set_tests_properties(${HOST_TEST_NAME} PROPERTIES DEPENDS ${test_build_target})
  endfunction()
elseif(NOT HOST_CC)
  message(STATUS "No host C compiler found; host Unity unit tests will not be available")
else()
  message(STATUS "Unity source not available; host Unity unit tests will not be available")
endif()

add_subdirectory(Unit)
