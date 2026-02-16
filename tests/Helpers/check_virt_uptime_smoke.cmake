# Distributed under the MIT license.
# See LICENSE.txt for details.

if(NOT DEFINED QEMU_BIN)
  message(FATAL_ERROR "QEMU_BIN is required.")
endif()

if(NOT DEFINED INPUT_ELF)
  message(FATAL_ERROR "INPUT_ELF is required.")
endif()

if(NOT DEFINED QEMU_MACHINE)
  message(FATAL_ERROR "QEMU_MACHINE is required.")
endif()

if(NOT DEFINED QEMU_CPU)
  message(FATAL_ERROR "QEMU_CPU is required.")
endif()

if(NOT DEFINED QEMU_MEMORY)
  message(FATAL_ERROR "QEMU_MEMORY is required.")
endif()

set(smoke_timeout_sec 5)
if(DEFINED SMOKE_TIMEOUT_SEC)
  set(smoke_timeout_sec ${SMOKE_TIMEOUT_SEC})
endif()

set(script_path "${CMAKE_CURRENT_BINARY_DIR}/qemu_uptime_smoke.sh")
file(WRITE "${script_path}" "#!/bin/sh\n")
file(APPEND "${script_path}" "{\n")
file(APPEND "${script_path}" "  printf 'uptime\\n'\n")
file(APPEND "${script_path}" "  sleep 1\n")
file(APPEND "${script_path}" "  printf 'uptime\\n'\n")
file(APPEND "${script_path}" "} | \"${QEMU_BIN}\"")
file(APPEND "${script_path}" " -machine \"${QEMU_MACHINE}\"")
file(APPEND "${script_path}" " -cpu \"${QEMU_CPU}\"")
file(APPEND "${script_path}" " -m \"${QEMU_MEMORY}\"")
file(APPEND "${script_path}" " -nographic -monitor none -serial stdio")
file(APPEND "${script_path}" " -kernel \"${INPUT_ELF}\"\n")

execute_process(COMMAND sh "${script_path}"
                RESULT_VARIABLE qemu_rc
                OUTPUT_VARIABLE qemu_out
                ERROR_VARIABLE qemu_err
                TIMEOUT ${smoke_timeout_sec})

set(qemu_text "${qemu_out}\n${qemu_err}")
string(REPLACE "\r\n" "\n" qemu_text "${qemu_text}")
string(REPLACE "\r" "\n" qemu_text "${qemu_text}")

if(qemu_text STREQUAL "")
  message(FATAL_ERROR
          "QEMU uptime smoke test produced no output.\nstdout/stderr:\n${qemu_text}")
endif()

string(
  REGEX MATCHALL
        "0x[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f]"
        uptime_hex_values "${qemu_text}")
list(LENGTH uptime_hex_values uptime_value_count)
if(uptime_value_count LESS 2)
  message(FATAL_ERROR
          "Expected at least two fixed-width uptime values, found ${uptime_value_count}.\n"
          "Full output:\n${qemu_text}")
endif()

list(GET uptime_hex_values 0 first_uptime_hex)
list(GET uptime_hex_values 1 second_uptime_hex)
string(REGEX REPLACE "^0x" "" first_uptime_hex_noprefix "${first_uptime_hex}")
string(REGEX REPLACE "^0x" "" second_uptime_hex_noprefix "${second_uptime_hex}")
math(EXPR first_uptime_dec "0x${first_uptime_hex_noprefix}")
math(EXPR second_uptime_dec "0x${second_uptime_hex_noprefix}")

if(second_uptime_dec LESS_EQUAL first_uptime_dec)
  message(FATAL_ERROR
          "Expected second uptime to be greater than first.\n"
          "First uptime: ${first_uptime_hex}\n"
          "Second uptime: ${second_uptime_hex}\n"
          "Full output:\n${qemu_text}")
endif()
