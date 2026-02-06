# Distributed under the MIT license.
# See LICENSE.txt for details.

if(NOT DEFINED OBJDUMP)
  message(FATAL_ERROR "OBJDUMP is required.")
endif()

if(NOT DEFINED OBJCOPY)
  message(FATAL_ERROR "OBJCOPY is required.")
endif()

if(NOT DEFINED INPUT_IMG)
  message(FATAL_ERROR "INPUT_IMG is required.")
endif()

if(NOT DEFINED WRAPPED_ELF)
  message(FATAL_ERROR "WRAPPED_ELF is required.")
endif()

get_filename_component(img_dir ${INPUT_IMG} DIRECTORY)
get_filename_component(img_name ${INPUT_IMG} NAME)

execute_process(
  COMMAND ${OBJCOPY} -I binary -O elf64-littleaarch64 -B aarch64 ${img_name}
          ${WRAPPED_ELF}
  WORKING_DIRECTORY ${img_dir}
  RESULT_VARIABLE copy_rc
  OUTPUT_VARIABLE copy_out
  ERROR_VARIABLE copy_err)
if(NOT copy_rc EQUAL 0)
  message(FATAL_ERROR
          "Failed to wrap ${INPUT_IMG} as ELF.\n${copy_out}\n${copy_err}")
endif()

execute_process(
  COMMAND ${OBJDUMP} -t ${WRAPPED_ELF}
  RESULT_VARIABLE dump_rc
  OUTPUT_VARIABLE dump_out
  ERROR_VARIABLE dump_err)
if(NOT dump_rc EQUAL 0)
  message(FATAL_ERROR
          "Failed to inspect wrapped image ${WRAPPED_ELF}.\n${dump_err}")
endif()

set(required_img_symbols
    "_binary_.*kernel8_img_start"
    "_binary_.*kernel8_img_end"
    "_binary_.*kernel8_img_size")

foreach(sym_regex IN LISTS required_img_symbols)
  if(NOT dump_out MATCHES "${sym_regex}")
    message(FATAL_ERROR
            "Missing expected symbol pattern '${sym_regex}' in wrapped image.")
  endif()
endforeach()
