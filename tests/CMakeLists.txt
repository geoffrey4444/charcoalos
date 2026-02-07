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

find_program(HOST_CC NAMES clang cc gcc)
if(HOST_CC AND UNITY_RESOLVED_SOURCE_DIR)
  set(CONSOLE_NEWLINE_TEST_BIN ${CMAKE_CURRENT_BINARY_DIR}/console_newline_test)
  add_custom_command(
    OUTPUT ${CONSOLE_NEWLINE_TEST_BIN}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${HOST_CC}
            -std=c11
            -Wall
            -Wextra
            -I${CMAKE_SOURCE_DIR}
            -I${UNITY_RESOLVED_SOURCE_DIR}/src
            ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.c
            ${CMAKE_SOURCE_DIR}/tests/Unit/ConsoleNewlineTest.c
            ${CMAKE_SOURCE_DIR}/kernel/Console/IO.c
            -o ${CONSOLE_NEWLINE_TEST_BIN}
    DEPENDS
      ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.c
      ${UNITY_RESOLVED_SOURCE_DIR}/src/unity.h
      ${CMAKE_SOURCE_DIR}/tests/Unit/ConsoleNewlineTest.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h
    VERBATIM)
  add_custom_target(unit-console-newline-build ALL DEPENDS ${CONSOLE_NEWLINE_TEST_BIN})
  add_test(
    NAME unit-console-newline
    COMMAND ${CONSOLE_NEWLINE_TEST_BIN})
  set_tests_properties(unit-console-newline PROPERTIES DEPENDS unit-console-newline-build)
elseif(NOT HOST_CC)
  message(STATUS "No host C compiler found; unit-console-newline test will not be available")
else()
  message(STATUS "Unity source not available; unit-console-newline test will not be available")
endif()
