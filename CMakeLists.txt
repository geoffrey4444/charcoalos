# Distributed under the MIT license.
# See LICENSE.txt for details.

cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
          "In-source builds are not supported. Configure with: cmake -S . -B build")
endif()

set(AARCH64_TOOLCHAIN_PREFIX "aarch64-elf-" CACHE STRING
    "Prefix for the AArch64 bare-metal cross toolchain")
# Bare-metal cross setup: prevent Darwin host defaults (for example, '-arch arm64')
# from being injected into compiler checks and build rules.
set(CMAKE_SYSTEM_NAME Generic CACHE STRING "Target system" FORCE)
set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE STRING "Target processor" FORCE)
set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "Disable host OSX arch flags for cross build" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT CMAKE_C_COMPILER)
  find_program(CHARCOALOS_AARCH64_GCC NAMES ${AARCH64_TOOLCHAIN_PREFIX}gcc REQUIRED)
  set(CMAKE_C_COMPILER "${CHARCOALOS_AARCH64_GCC}" CACHE FILEPATH
      "C compiler for CharcoalOS" FORCE)
endif()

project(charcoalos LANGUAGES C ASM)
enable_language(ASM)
include(CTest)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Doxygen QUIET)
if(Doxygen_FOUND)
  set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
  set(DOXYGEN_HTML_OUTPUT_DIR "${DOXYGEN_OUTPUT_DIR}/html")
  set(DOXYGEN_INPUT_DIRS
      "${CMAKE_SOURCE_DIR}/kernel"
      "${CMAKE_SOURCE_DIR}/arch"
      "${CMAKE_SOURCE_DIR}/platform"
      "${CMAKE_SOURCE_DIR}/minimal")
  set(DOXYGEN_EXCLUDE_PATTERNS
      "*/build/*"
      "*/build-*/*")
  string(REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT_DIRS}")
  string(REPLACE ";" " " DOXYGEN_EXCLUDES "${DOXYGEN_EXCLUDE_PATTERNS}")
  set(DOXYFILE_IN "${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in")
  set(DOXYFILE_OUT "${CMAKE_BINARY_DIR}/Doxyfile")

  configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)
  add_custom_target(
    docs
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIR}"
    COMMAND Doxygen::doxygen "${DOXYFILE_OUT}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Generate HTML documentation with Doxygen"
    VERBATIM)
else()
  message(STATUS "Doxygen not found; docs target is unavailable")
endif()

find_program(AARCH64_AS NAMES ${AARCH64_TOOLCHAIN_PREFIX}as REQUIRED)
find_program(AARCH64_LD NAMES ${AARCH64_TOOLCHAIN_PREFIX}ld REQUIRED)
find_program(AARCH64_OBJCOPY NAMES ${AARCH64_TOOLCHAIN_PREFIX}objcopy REQUIRED)
find_program(QEMU_SYSTEM_AARCH64 NAMES qemu-system-aarch64)
set(AARCH64_GCC "${CMAKE_C_COMPILER}")

add_subdirectory(kernel)
add_subdirectory(arch/arm64)
add_subdirectory(platform/virt)
add_subdirectory(platform/rpi)

function(configure_platform_interrupt_map platform_name out_include_dir)
  set(platform_include_dir
      "${CMAKE_BINARY_DIR}/generated/${platform_name}/include")
  set(platform_include_parent
      "${platform_include_dir}/platform")
  file(MAKE_DIRECTORY "${platform_include_parent}")
  configure_file(
    "${CMAKE_SOURCE_DIR}/platform/${platform_name}/InterruptMap.h"
    "${platform_include_parent}/InterruptMap.h"
    COPYONLY)
  set(${out_include_dir} "${platform_include_dir}" PARENT_SCOPE)
endfunction()

configure_platform_interrupt_map(virt VIRT_PLATFORM_INCLUDE_DIR)
configure_platform_interrupt_map(rpi RPI_PLATFORM_INCLUDE_DIR)

add_kernel_common_target(kernel_common_virt
  PLATFORM_INCLUDE_DIR "${VIRT_PLATFORM_INCLUDE_DIR}"
  PLATFORM_DEFINE TARGET_VIRT=1)
add_kernel_common_target(kernel_common_rpi
  PLATFORM_INCLUDE_DIR "${RPI_PLATFORM_INCLUDE_DIR}"
  PLATFORM_DEFINE TARGET_RPI=1)
add_arch_arm64_target(arch_arm64_virt
  PLATFORM_INCLUDE_DIR "${VIRT_PLATFORM_INCLUDE_DIR}"
  PLATFORM_DEFINE TARGET_VIRT=1)
add_arch_arm64_target(arch_arm64_rpi
  PLATFORM_INCLUDE_DIR "${RPI_PLATFORM_INCLUDE_DIR}"
  PLATFORM_DEFINE TARGET_RPI=1)
add_platform_virt_target(platform_virt
  PLATFORM_INCLUDE_DIR "${VIRT_PLATFORM_INCLUDE_DIR}")
add_platform_rpi_target(platform_rpi
  PLATFORM_INCLUDE_DIR "${RPI_PLATFORM_INCLUDE_DIR}")

set(REF_VIRT_BUILD_DIR ${CMAKE_BINARY_DIR}/virt)
set(REF_VIRT_BOOT_O ${REF_VIRT_BUILD_DIR}/boot.o)
set(REF_VIRT_ELF ${REF_VIRT_BUILD_DIR}/kernel.elf)

add_custom_command(
  OUTPUT ${REF_VIRT_BOOT_O}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${REF_VIRT_BUILD_DIR}
  COMMAND ${AARCH64_AS} ${CMAKE_SOURCE_DIR}/platform/virt/boot.s -o ${REF_VIRT_BOOT_O}
  DEPENDS ${CMAKE_SOURCE_DIR}/platform/virt/boot.s
  VERBATIM)

add_custom_command(
  OUTPUT ${REF_VIRT_ELF}
  COMMAND ${CMAKE_COMMAND} -E chdir ${REF_VIRT_BUILD_DIR}
          ${AARCH64_LD} -nostdlib -T ${CMAKE_SOURCE_DIR}/platform/virt/linker.ld
          boot.o
          $<TARGET_OBJECTS:kernel_common_virt>
          $<TARGET_OBJECTS:arch_arm64_virt>
          $<TARGET_OBJECTS:platform_virt>
          -o kernel.elf
  DEPENDS ${REF_VIRT_BOOT_O}
          ${CMAKE_SOURCE_DIR}/platform/virt/linker.ld
          kernel_common_virt
          arch_arm64_virt
          platform_virt
          $<TARGET_OBJECTS:kernel_common_virt>
          $<TARGET_OBJECTS:arch_arm64_virt>
          $<TARGET_OBJECTS:platform_virt>
  COMMAND_EXPAND_LISTS
  VERBATIM)
add_custom_target(refactored-virt DEPENDS ${REF_VIRT_ELF})
add_custom_target(virt ALL DEPENDS refactored-virt)

set(REF_RPI_BUILD_DIR ${CMAKE_BINARY_DIR}/rpi)
set(REF_RPI_BOOT_O ${REF_RPI_BUILD_DIR}/boot.o)
set(REF_RPI_ELF ${REF_RPI_BUILD_DIR}/kernel.elf)
set(REF_RPI_IMG ${REF_RPI_BUILD_DIR}/kernel8.img)

add_custom_command(
  OUTPUT ${REF_RPI_BOOT_O}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${REF_RPI_BUILD_DIR}
  COMMAND ${AARCH64_AS} ${CMAKE_SOURCE_DIR}/platform/rpi/boot.s -o ${REF_RPI_BOOT_O}
  DEPENDS ${CMAKE_SOURCE_DIR}/platform/rpi/boot.s
  VERBATIM)

add_custom_command(
  OUTPUT ${REF_RPI_ELF}
  COMMAND ${AARCH64_LD} -nostdlib -T ${CMAKE_SOURCE_DIR}/platform/rpi/linker.ld
          ${REF_RPI_BOOT_O}
          $<TARGET_OBJECTS:kernel_common_rpi>
          $<TARGET_OBJECTS:arch_arm64_rpi>
          $<TARGET_OBJECTS:platform_rpi>
          -o ${REF_RPI_ELF}
  DEPENDS ${REF_RPI_BOOT_O}
          ${CMAKE_SOURCE_DIR}/platform/rpi/linker.ld
          kernel_common_rpi
          arch_arm64_rpi
          platform_rpi
          $<TARGET_OBJECTS:kernel_common_rpi>
          $<TARGET_OBJECTS:arch_arm64_rpi>
          $<TARGET_OBJECTS:platform_rpi>
  COMMAND_EXPAND_LISTS
  VERBATIM)
add_custom_target(refactored-rpi-elf DEPENDS ${REF_RPI_ELF})

add_custom_command(
  OUTPUT ${REF_RPI_IMG}
  COMMAND ${AARCH64_OBJCOPY} -O binary ${REF_RPI_ELF} ${REF_RPI_IMG}
  DEPENDS ${REF_RPI_ELF}
  VERBATIM)
add_custom_target(refactored-rpi DEPENDS ${REF_RPI_IMG})
add_custom_target(rpi ALL DEPENDS refactored-rpi)
add_custom_target(refactored DEPENDS virt rpi)

set(VIRT_SRC_DIR ${CMAKE_SOURCE_DIR}/minimal/virt)
set(VIRT_BUILD_DIR ${CMAKE_BINARY_DIR}/minimal/virt)
set(VIRT_BOOT_O ${VIRT_BUILD_DIR}/boot.o)
set(VIRT_KERNEL_O ${VIRT_BUILD_DIR}/kernel.o)
set(VIRT_ELF ${VIRT_BUILD_DIR}/kernel.elf)

add_custom_command(
  OUTPUT ${VIRT_ELF}
  BYPRODUCTS ${VIRT_BOOT_O} ${VIRT_KERNEL_O}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${VIRT_BUILD_DIR}
  COMMAND ${AARCH64_AS} ${VIRT_SRC_DIR}/boot.s -o ${VIRT_BOOT_O}
  COMMAND ${AARCH64_GCC} -ffreestanding -fno-builtin -nostdlib -c ${VIRT_SRC_DIR}/kernel.c -o ${VIRT_KERNEL_O}
  COMMAND ${CMAKE_COMMAND} -E chdir ${VIRT_BUILD_DIR} ${AARCH64_LD} -nostdlib -T ${VIRT_SRC_DIR}/linker.ld boot.o kernel.o -o kernel.elf
  DEPENDS ${VIRT_SRC_DIR}/boot.s ${VIRT_SRC_DIR}/kernel.c ${VIRT_SRC_DIR}/linker.ld
  VERBATIM)

add_custom_target(minimal-virt ALL DEPENDS ${VIRT_ELF})

set(VIRT_QEMU_MACHINE "virt,accel=tcg" CACHE STRING
    "QEMU machine argument for running minimal-virt")
set(VIRT_QEMU_SMP "4" CACHE STRING
    "QEMU SMP argument for running minimal-virt")
set(VIRT_QEMU_CPU "cortex-a72" CACHE STRING
    "QEMU cpu argument for running minimal-virt")
set(VIRT_QEMU_MEMORY "512M" CACHE STRING
    "QEMU memory argument for running minimal-virt")
set(VIRT_QEMU_NOGRAPHIC ON CACHE BOOL
    "Use -nographic when running minimal-virt")
set(VIRT_QEMU_SERIAL_MON_STDIO ON CACHE BOOL
    "Use -serial mon:stdio when running minimal-virt")

if(QEMU_SYSTEM_AARCH64)
  set(REF_VIRT_QEMU_RUN_ARGS
      -machine ${VIRT_QEMU_MACHINE}
      -cpu ${VIRT_QEMU_CPU}
      -smp ${VIRT_QEMU_SMP}
      -m ${VIRT_QEMU_MEMORY}
      -kernel ${REF_VIRT_ELF})
  if(VIRT_QEMU_NOGRAPHIC)
    list(APPEND REF_VIRT_QEMU_RUN_ARGS -nographic)
  endif()
  if(VIRT_QEMU_SERIAL_MON_STDIO)
    list(APPEND REF_VIRT_QEMU_RUN_ARGS -serial mon:stdio)
  endif()
  add_custom_target(
    run-virt
    COMMAND ${QEMU_SYSTEM_AARCH64} ${REF_VIRT_QEMU_RUN_ARGS}
    DEPENDS virt
    USES_TERMINAL
    COMMENT "Run refactored virt target in QEMU")

  set(VIRT_QEMU_RUN_ARGS
      -machine ${VIRT_QEMU_MACHINE}
      -cpu ${VIRT_QEMU_CPU}
      -m ${VIRT_QEMU_MEMORY}
      -smp ${VIRT_QEMU_SMP}
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
          "qemu-system-aarch64 not found; run-virt and run-minimal-virt targets will not be available")
endif()

set(RPI_SRC_DIR ${CMAKE_SOURCE_DIR}/minimal/rpi)
set(RPI_BUILD_DIR ${CMAKE_BINARY_DIR}/minimal/rpi)
set(RPI_BOOT_O ${RPI_BUILD_DIR}/boot.o)
set(RPI_KERNEL_O ${RPI_BUILD_DIR}/kernel.o)
set(RPI_ELF ${RPI_BUILD_DIR}/kernel.elf)
set(RPI_IMG ${RPI_BUILD_DIR}/kernel8.img)

add_custom_command(
  OUTPUT ${RPI_IMG}
  BYPRODUCTS ${RPI_BOOT_O} ${RPI_KERNEL_O} ${RPI_ELF}
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
  set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL
      "Avoid network updates for already-populated FetchContent dependencies" FORCE)
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
      GIT_TAG v2.6.1
      GIT_SHALLOW TRUE)
    cmake_policy(PUSH)
    if(POLICY CMP0169)
      cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_GetProperties(unity_upstream)
    if(NOT unity_upstream_POPULATED)
      FetchContent_Populate(unity_upstream)
    endif()
    cmake_policy(POP)
    set(UNITY_RESOLVED_SOURCE_DIR "${unity_upstream_SOURCE_DIR}")
  endif()

  if(UNITY_RESOLVED_SOURCE_DIR)
    message(STATUS "Unity source available at ${UNITY_RESOLVED_SOURCE_DIR}")
  else()
    message(STATUS
            "Unity not found at ${UNITY_SOURCE_DIR}; set CHARCOALOS_FETCH_UNITY=ON to auto-fetch it")
  endif()

  add_subdirectory(tests)
endif()
