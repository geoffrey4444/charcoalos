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

if(NOT DEFINED EXPECTED_SUBSTRING)
  message(FATAL_ERROR "EXPECTED_SUBSTRING is required.")
endif()

if(NOT DEFINED COMMAND_TEXT)
  message(FATAL_ERROR "COMMAND_TEXT is required.")
endif()

set(smoke_timeout_sec 2)
if(DEFINED SMOKE_TIMEOUT_SEC)
  set(smoke_timeout_sec ${SMOKE_TIMEOUT_SEC})
endif()

set(command_text "${COMMAND_TEXT}")
string(REPLACE "\\n" "\n" command_text "${command_text}")

string(MD5 command_hash "${COMMAND_TEXT}|${EXPECTED_SUBSTRING}")
set(qemu_input_file
    "${CMAKE_CURRENT_BINARY_DIR}/qemu_exception_smoke_input_${command_hash}.txt")
file(WRITE "${qemu_input_file}" "${command_text}")

execute_process(
  COMMAND ${QEMU_BIN}
          -machine ${QEMU_MACHINE}
          -cpu ${QEMU_CPU}
          -m ${QEMU_MEMORY}
          -nographic
          -monitor none
          -serial stdio
          -kernel ${INPUT_ELF}
  INPUT_FILE "${qemu_input_file}"
  RESULT_VARIABLE qemu_rc
  OUTPUT_VARIABLE qemu_out
  ERROR_VARIABLE qemu_err
  TIMEOUT ${smoke_timeout_sec})

set(qemu_text "${qemu_out}\n${qemu_err}")
string(REPLACE "\r\n" "\n" qemu_text "${qemu_text}")
string(REPLACE "\r" "\n" qemu_text "${qemu_text}")

if(qemu_text STREQUAL "")
  message(FATAL_ERROR
          "QEMU exception smoke test produced no output.\nstdout/stderr:\n${qemu_text}")
endif()

string(FIND "${qemu_text}" "${EXPECTED_SUBSTRING}" expected_pos)
if(expected_pos EQUAL -1)
  message(FATAL_ERROR
          "QEMU output did not contain expected text.\n"
          "Expected substring: '${EXPECTED_SUBSTRING}'\n"
          "Full output:\n${qemu_text}")
endif()
