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
find_program(QEMU_SYSTEM_AARCH64 NAMES qemu-system-aarch64)

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

set(VIRT_QEMU_MACHINE "virt,accel=hvf" CACHE STRING
    "QEMU machine argument for running minimal-virt")
set(VIRT_QEMU_CPU "host" CACHE STRING
    "QEMU cpu argument for running minimal-virt")
set(VIRT_QEMU_MEMORY "512M" CACHE STRING
    "QEMU memory argument for running minimal-virt")
set(VIRT_QEMU_NOGRAPHIC ON CACHE BOOL
    "Use -nographic when running minimal-virt")
set(VIRT_QEMU_SERIAL_MON_STDIO ON CACHE BOOL
    "Use -serial mon:stdio when running minimal-virt")

if(QEMU_SYSTEM_AARCH64)
  set(VIRT_QEMU_RUN_ARGS
      -machine ${VIRT_QEMU_MACHINE}
      -cpu ${VIRT_QEMU_CPU}
      -m ${VIRT_QEMU_MEMORY}
      -kernel ${VIRT_ELF})
  if(VIRT_QEMU_NOGRAPHIC)
    list(APPEND VIRT_QEMU_RUN_ARGS -nographic)
  endif()
  if(VIRT_QEMU_SERIAL_MON_STDIO)
    list(APPEND VIRT_QEMU_RUN_ARGS -serial mon:stdio)
  endif()

  add_custom_target(
    run-minimal-virt
    COMMAND ${QEMU_SYSTEM_AARCH64} ${VIRT_QEMU_RUN_ARGS}
    DEPENDS minimal-virt
    USES_TERMINAL
    COMMENT "Run minimal-virt in QEMU")
else()
  message(STATUS
          "qemu-system-aarch64 not found; run-minimal-virt target will not be available")
endif()

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
  option(CHARCOALOS_FETCH_UNITY
         "Automatically fetch Unity from official sources when not found locally"
         ON)
  set(UNITY_SOURCE_DIR "${CMAKE_SOURCE_DIR}/third_party/unity" CACHE PATH
      "Path to a Unity checkout (expects src/unity.h and src/unity.c)")

  set(UNITY_RESOLVED_SOURCE_DIR "")
  if(EXISTS "${UNITY_SOURCE_DIR}/src/unity.h" AND EXISTS "${UNITY_SOURCE_DIR}/src/unity.c")
    set(UNITY_RESOLVED_SOURCE_DIR "${UNITY_SOURCE_DIR}")
  elseif(CHARCOALOS_FETCH_UNITY)
    include(FetchContent)
    FetchContent_Declare(
      unity_upstream
      GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
      GIT_TAG master
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(unity_upstream)
    set(UNITY_RESOLVED_SOURCE_DIR "${unity_upstream_SOURCE_DIR}")
  endif()

  if(UNITY_RESOLVED_SOURCE_DIR)
    if(TARGET unity)
      message(STATUS "Using Unity target provided by upstream CMake")
    else()
      add_library(unity INTERFACE)
      target_include_directories(unity INTERFACE "${UNITY_RESOLVED_SOURCE_DIR}/src")
      target_sources(unity INTERFACE "${UNITY_RESOLVED_SOURCE_DIR}/src/unity.c")
    endif()
  else()
    message(STATUS
            "Unity not found at ${UNITY_SOURCE_DIR}; set CHARCOALOS_FETCH_UNITY=ON to auto-fetch it")
  endif()

  add_subdirectory(tests)
endif()
