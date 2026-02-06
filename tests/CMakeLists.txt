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
