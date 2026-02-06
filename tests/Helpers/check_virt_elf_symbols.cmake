# Distributed under the MIT license.
# See LICENSE.txt for details.

if(NOT DEFINED OBJDUMP)
  message(FATAL_ERROR "OBJDUMP is required.")
endif()

if(NOT DEFINED INPUT_ELF)
  message(FATAL_ERROR "INPUT_ELF is required.")
endif()

execute_process(
  COMMAND ${OBJDUMP} -t ${INPUT_ELF}
  RESULT_VARIABLE dump_rc
  OUTPUT_VARIABLE dump_out
  ERROR_VARIABLE dump_err)
if(NOT dump_rc EQUAL 0)
  message(FATAL_ERROR
          "Failed to inspect ${INPUT_ELF} with objdump.\n${dump_err}")
endif()

set(required_symbols
    "_start"
    "kmain"
    "uart_putchar"
    "stack_top")

foreach(sym IN LISTS required_symbols)
  if(NOT dump_out MATCHES "(^|[^A-Za-z0-9_])${sym}([^A-Za-z0-9_]|$)")
    message(FATAL_ERROR
            "Missing expected symbol '${sym}' in ${INPUT_ELF}.")
  endif()
endforeach()
