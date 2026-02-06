# Distributed under the MIT license.
# See LICENSE.txt for details.

cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
          "In-source builds are not supported. Configure with: cmake -S . -B build")
endif()

project(charcoalos LANGUAGES NONE)
include(CTest)

set(AARCH64_TOOLCHAIN_PREFIX "aarch64-elf-" CACHE STRING
    "Prefix for the AArch64 bare-metal cross toolchain")

find_program(AARCH64_AS NAMES ${AARCH64_TOOLCHAIN_PREFIX}as REQUIRED)
find_program(AARCH64_GCC NAMES ${AARCH64_TOOLCHAIN_PREFIX}gcc REQUIRED)
find_program(AARCH64_LD NAMES ${AARCH64_TOOLCHAIN_PREFIX}ld REQUIRED)
find_program(AARCH64_OBJCOPY NAMES ${AARCH64_TOOLCHAIN_PREFIX}objcopy REQUIRED)

set(VIRT_SRC_DIR ${CMAKE_SOURCE_DIR}/minimal/virt)
set(VIRT_BUILD_DIR ${CMAKE_BINARY_DIR}/minimal/virt)
set(VIRT_BOOT_O ${VIRT_BUILD_DIR}/boot.o)
set(VIRT_KERNEL_O ${VIRT_BUILD_DIR}/kernel.o)
set(VIRT_ELF ${VIRT_BUILD_DIR}/kernel.elf)

add_custom_command(
  OUTPUT ${VIRT_ELF}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${VIRT_BUILD_DIR}
  COMMAND ${AARCH64_AS} ${VIRT_SRC_DIR}/boot.s -o ${VIRT_BOOT_O}
  COMMAND ${AARCH64_GCC} -ffreestanding -fno-builtin -nostdlib -c ${VIRT_SRC_DIR}/kernel.c -o ${VIRT_KERNEL_O}
  COMMAND ${CMAKE_COMMAND} -E chdir ${VIRT_BUILD_DIR} ${AARCH64_LD} -nostdlib -T ${VIRT_SRC_DIR}/linker.ld boot.o kernel.o -o kernel.elf
  DEPENDS ${VIRT_SRC_DIR}/boot.s ${VIRT_SRC_DIR}/kernel.c ${VIRT_SRC_DIR}/linker.ld
  VERBATIM)

add_custom_target(minimal-virt ALL DEPENDS ${VIRT_ELF})

set(RPI_SRC_DIR ${CMAKE_SOURCE_DIR}/minimal/rpi)
set(RPI_BUILD_DIR ${CMAKE_BINARY_DIR}/minimal/rpi)
set(RPI_BOOT_O ${RPI_BUILD_DIR}/boot.o)
set(RPI_KERNEL_O ${RPI_BUILD_DIR}/kernel.o)
set(RPI_ELF ${RPI_BUILD_DIR}/kernel.elf)
set(RPI_IMG ${RPI_BUILD_DIR}/kernel8.img)

add_custom_command(
  OUTPUT ${RPI_IMG}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${RPI_BUILD_DIR}
  COMMAND ${AARCH64_AS} ${RPI_SRC_DIR}/boot.s -o ${RPI_BOOT_O}
  COMMAND ${AARCH64_GCC} -ffreestanding -fno-builtin -nostdlib -c ${RPI_SRC_DIR}/kernel.c -o ${RPI_KERNEL_O}
  COMMAND ${AARCH64_LD} -nostdlib -T ${RPI_SRC_DIR}/linker.ld ${RPI_BOOT_O} ${RPI_KERNEL_O} -o ${RPI_ELF}
  COMMAND ${AARCH64_OBJCOPY} -O binary ${RPI_ELF} ${RPI_IMG}
  DEPENDS ${RPI_SRC_DIR}/boot.s ${RPI_SRC_DIR}/kernel.c ${RPI_SRC_DIR}/linker.ld
  VERBATIM)

add_custom_target(minimal-rpi ALL DEPENDS ${RPI_IMG})

add_custom_target(minimal DEPENDS minimal-virt minimal-rpi)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
