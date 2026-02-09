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
endif()
