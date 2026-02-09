# Distributed under the MIT license.
# See LICENSE.txt for details.

if(COMMAND add_host_unity_test)
  add_host_unity_test(
    NAME unit-console-newline
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/ConsoleNewlineTest.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.c
      ${CMAKE_SOURCE_DIR}/kernel/String/String.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.h
      ${CMAKE_SOURCE_DIR}/kernel/String/String.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h)

  add_host_unity_test(
    NAME unit-console-io
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/ConsoleIOTest.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.c
      ${CMAKE_SOURCE_DIR}/kernel/String/String.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.h
      ${CMAKE_SOURCE_DIR}/kernel/String/String.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h)

  add_host_unity_test(
    NAME unit-shell
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/ShellTest.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/Shell.c
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.c
      ${CMAKE_SOURCE_DIR}/kernel/String/String.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/kernel/Console/Shell.h
      ${CMAKE_SOURCE_DIR}/kernel/Console/IO.h
      ${CMAKE_SOURCE_DIR}/kernel/String/String.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h)

  add_host_unity_test(
    NAME unit-arch-info
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/ArchInfoTest.c
      ${CMAKE_SOURCE_DIR}/arch/arm64/Info.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/arch/Info.h)

  add_host_unity_test(
    NAME unit-platform-virt-info
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/PlatformVirtInfoTest.c
      ${CMAKE_SOURCE_DIR}/platform/virt/Info.c
      ${CMAKE_SOURCE_DIR}/platform/virt/IO.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/platform/Info.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h
      ${CMAKE_SOURCE_DIR}/platform/virt/IO.h)

  add_host_unity_test(
    NAME unit-platform-rpi-info
    SOURCES
      ${CMAKE_SOURCE_DIR}/tests/Unit/Console/PlatformRpiInfoTest.c
      ${CMAKE_SOURCE_DIR}/platform/rpi/Info.c
      ${CMAKE_SOURCE_DIR}/platform/rpi/IO.c
    DEPENDS
      ${CMAKE_SOURCE_DIR}/platform/Info.h
      ${CMAKE_SOURCE_DIR}/platform/IO.h
      ${CMAKE_SOURCE_DIR}/platform/rpi/IO.h)
endif()
