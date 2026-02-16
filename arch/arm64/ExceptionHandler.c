// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Halt.h"
#include "arch/Interrupt.h"
#include "arch/arm64/ExceptionTypes.h"
#include "kernel/Console/IO.h"

#include <stddef.h>
#include <stdint.h>

static const char *exception_type_string(uint64_t kind_of_exception) {
  switch (kind_of_exception) {
    case EXCEPTION_TYPE_SYNC_EL1_SP0:
      return "SYNC_EL1_SP0";
    case EXCEPTION_TYPE_IRQ_EL1_SP0:
      return "IRQ_EL1_SP0";
    case EXCEPTION_TYPE_FIQ_EL1_SP0:
      return "FIQ_EL1_SP0";
    case EXCEPTION_TYPE_SERROR_EL1_SP0:
      return "SERROR_EL1_SP0";
    case EXCEPTION_TYPE_SYNC_EL1_SPX:
      return "SYNC_EL1_SPX";
    case EXCEPTION_TYPE_IRQ_EL1_SPX:
      return "IRQ_EL1_SPX";
    case EXCEPTION_TYPE_FIQ_EL1_SPX:
      return "FIQ_EL1_SPX";
    case EXCEPTION_TYPE_SERROR_EL1_SPX:
      return "SERROR_EL1_SPX";
    case EXCEPTION_TYPE_SYNC_EL0_64:
      return "SYNC_EL0_64";
    case EXCEPTION_TYPE_IRQ_EL0_64:
      return "IRQ_EL0_64";
    case EXCEPTION_TYPE_FIQ_EL0_64:
      return "FIQ_EL0_64";
    case EXCEPTION_TYPE_SERROR_EL0_64:
      return "SERROR_EL0_64";
    case EXCEPTION_TYPE_SYNC_EL0_32:
      return "SYNC_EL0_32";
    case EXCEPTION_TYPE_IRQ_EL0_32:
      return "IRQ_EL0_32";
    case EXCEPTION_TYPE_FIQ_EL0_32:
      return "FIQ_EL0_32";
    case EXCEPTION_TYPE_SERROR_EL0_32:
      return "SERROR_EL0_32";
    default:
      break;
  }
  return "UNKNOWN_EXCEPTION_TYPE";
}

// Read system registers esr_el1, elr_el1, spsr_el1, far_el1
// ESR_EL1: exception syndrome register
//          Bits [31:26] = exception class
// ELR_EL1: exception link register (return address for exception)
// SPSR_EL1: saved program status register ... bits have info about exception
// FAR_EL1: Fault address register
//
static inline uint64_t read_esr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  uint64_t register_value;
  __asm__ volatile("mrs %0, ESR_EL1" : "=r"(register_value));
  return register_value;
#else
  return 0;
#endif
}

static inline uint64_t read_elr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  uint64_t register_value;
  __asm__ volatile("mrs %0, ELR_EL1" : "=r"(register_value));
  return register_value;
#else
  return 0;
#endif
}

static inline uint64_t read_spsr_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  uint64_t register_value;
  __asm__ volatile("mrs %0, SPSR_EL1" : "=r"(register_value));
  return register_value;
#else
  return 0;
#endif
}

static inline uint64_t read_far_el1(void) {
#if defined(__aarch64__) && !__STDC_HOSTED__
  uint64_t register_value;
  __asm__ volatile("mrs %0, FAR_EL1" : "=r"(register_value));
  return register_value;
#else
  return 0;
#endif
}

static inline uint64_t exception_class_from_esr_el1(uint64_t esr_el1_value) {
  // Get bits [31:26] (6 bits): shift right 26 places, use bitwise & to zero
  // bits higher than (after the shift) bit 5
  // 0x3FULL = 64-bit unsigned (unsigned long long) integer, value 0x3F=0b111111
  uint64_t exception_class = (esr_el1_value >> 26) & 0x3FULL;
  return exception_class;
}

static inline uint64_t iss_from_esr_el1(uint64_t esr_el1_value) {
  return esr_el1_value & 0x01FFFFFFULL;  // 0x01FFFFFFULL = 25 1's, masks [24:0]
}

static const char *exception_class_to_string(uint64_t exception_class) {
  switch (exception_class) {
    case EXCEPTION_CLASS_UNKNOWN:
      return "EXCEPTION_CLASS_UNKNOWN";
    case EXCEPTION_CLASS_TRAPPED_WFI_WFE:
      return "EXCEPTION_CLASS_TRAPPED_WFI_WFE";

    case EXCEPTION_CLASS_TRAPPED_MCR_MRC_CP15:
      return "EXCEPTION_CLASS_TRAPPED_MCR_MRC_CP15";
    case EXCEPTION_CLASS_TRAPPED_MCRR_MRRC_CP15:
      return "EXCEPTION_CLASS_TRAPPED_MCRR_MRRC_CP15";
    case EXCEPTION_CLASS_TRAPPED_MCR_MRC_CP14:
      return "EXCEPTION_CLASS_TRAPPED_MCR_MRC_CP14";
    case EXCEPTION_CLASS_TRAPPED_LDC_STC:
      return "EXCEPTION_CLASS_TRAPPED_LDC_STC";

    case EXCEPTION_CLASS_TRAPPED_SIMD_FP_SVE_SME:
      return "EXCEPTION_CLASS_TRAPPED_SIMD_FP_SVE_SME";
    case EXCEPTION_CLASS_TRAPPED_POINTER_AUTHENTICATION:
      return "EXCEPTION_CLASS_TRAPPED_POINTER_AUTHENTICATION";
    case EXCEPTION_CLASS_TRAPPED_OTHER_INSTRUCTION:
      return "EXCEPTION_CLASS_TRAPPED_OTHER_INSTRUCTION";
    case EXCEPTION_CLASS_TRAPPED_MRRC_CP14:
      return "EXCEPTION_CLASS_TRAPPED_MRRC_CP14";

    case EXCEPTION_CLASS_BRANCH_TARGET:
      return "EXCEPTION_CLASS_BRANCH_TARGET";
    case EXCEPTION_CLASS_ILLEGAL_EXECUTION_STATE:
      return "EXCEPTION_CLASS_ILLEGAL_EXECUTION_STATE";

    case EXCEPTION_CLASS_SVC_AARCH32:
      return "EXCEPTION_CLASS_SVC_AARCH32";

    case EXCEPTION_CLASS_TRAPPED_SYSREG128_SYSINSTR128:
      return "EXCEPTION_CLASS_TRAPPED_SYSREG128_SYSINSTR128";
    case EXCEPTION_CLASS_SVC_AARCH64:
      return "EXCEPTION_CLASS_SVC_AARCH64";

    case EXCEPTION_CLASS_TRAPPED_MSR_MRS_SYSTEM_AARCH64:
      return "EXCEPTION_CLASS_TRAPPED_MSR_MRS_SYSTEM_AARCH64";
    case EXCEPTION_CLASS_TRAPPED_SVE:
      return "EXCEPTION_CLASS_TRAPPED_SVE";
    case EXCEPTION_CLASS_TRAPPED_ERET_ERETAA_ERETAB:
      return "EXCEPTION_CLASS_TRAPPED_ERET_ERETAA_ERETAB";
    case EXCEPTION_CLASS_PAC_FAIL:
      return "EXCEPTION_CLASS_PAC_FAIL";
    case EXCEPTION_CLASS_TRAPPED_SME:
      return "EXCEPTION_CLASS_TRAPPED_SME";

    case EXCEPTION_CLASS_INSTRUCTION_ABORT_LOWER_EL:
      return "EXCEPTION_CLASS_INSTRUCTION_ABORT_LOWER_EL";
    case EXCEPTION_CLASS_INSTRUCTION_ABORT_SAME_EL:
      return "EXCEPTION_CLASS_INSTRUCTION_ABORT_SAME_EL";
    case EXCEPTION_CLASS_PC_ALIGNMENT_FAULT:
      return "EXCEPTION_CLASS_PC_ALIGNMENT_FAULT";

    case EXCEPTION_CLASS_DATA_ABORT_LOWER_EL:
      return "EXCEPTION_CLASS_DATA_ABORT_LOWER_EL";
    case EXCEPTION_CLASS_DATA_ABORT_SAME_EL:
      return "EXCEPTION_CLASS_DATA_ABORT_SAME_EL";
    case EXCEPTION_CLASS_SP_ALIGNMENT_FAULT:
      return "EXCEPTION_CLASS_SP_ALIGNMENT_FAULT";

    case EXCEPTION_CLASS_MEMORY_OPERATION:
      return "EXCEPTION_CLASS_MEMORY_OPERATION";
    case EXCEPTION_CLASS_TRAPPED_FP_EXCEPTION_AARCH32:
      return "EXCEPTION_CLASS_TRAPPED_FP_EXCEPTION_AARCH32";
    case EXCEPTION_CLASS_TRAPPED_FP_EXCEPTION_AARCH64:
      return "EXCEPTION_CLASS_TRAPPED_FP_EXCEPTION_AARCH64";
    case EXCEPTION_CLASS_GCS:
      return "EXCEPTION_CLASS_GCS";
    case EXCEPTION_CLASS_SERROR:
      return "EXCEPTION_CLASS_SERROR";

    case EXCEPTION_CLASS_BREAKPOINT_LOWER_EL:
      return "EXCEPTION_CLASS_BREAKPOINT_LOWER_EL";
    case EXCEPTION_CLASS_BREAKPOINT_SAME_EL:
      return "EXCEPTION_CLASS_BREAKPOINT_SAME_EL";
    case EXCEPTION_CLASS_SOFTWARE_STEP_LOWER_EL:
      return "EXCEPTION_CLASS_SOFTWARE_STEP_LOWER_EL";
    case EXCEPTION_CLASS_SOFTWARE_STEP_SAME_EL:
      return "EXCEPTION_CLASS_SOFTWARE_STEP_SAME_EL";
    case EXCEPTION_CLASS_WATCHPOINT_LOWER_EL:
      return "EXCEPTION_CLASS_WATCHPOINT_LOWER_EL";
    case EXCEPTION_CLASS_WATCHPOINT_SAME_EL:
      return "EXCEPTION_CLASS_WATCHPOINT_SAME_EL";

    case EXCEPTION_CLASS_BKPT_AARCH32:
      return "EXCEPTION_CLASS_BKPT_AARCH32";
    case EXCEPTION_CLASS_BRK_AARCH64:
      return "EXCEPTION_CLASS_BRK_AARCH64";
    case EXCEPTION_CLASS_PROFILING:
      return "EXCEPTION_CLASS_PROFILING";

    default:
      return "EXCEPTION_CLASS_<unknown>";
  }
}

// Flag to stop a cascade of recursive exceptions
static volatile uint64_t g_exception_in_progress = 0;

uint64_t handle_exception(uint64_t *saved_registers,
                          uint64_t kind_of_exception) {
  // First, save the state. Registers already saved; save system registers
  // and basic info about the exception here.
  const uint64_t esr_el1_value = read_esr_el1();
  const uint64_t elr_el1_value = read_elr_el1();
  const uint64_t spsr_value = read_spsr_el1();
  const uint64_t far_value = read_far_el1();
  const uint64_t exception_class = exception_class_from_esr_el1(esr_el1_value);
  const uint64_t iss = iss_from_esr_el1(esr_el1_value);

  // Check if exception can be handled. Handled exceptions:
  //   - system calls via svc trap
  //   - IRQs from the timer (type IRQ_EL1_SPX, since EL1 uses SP_EL1)
  if (exception_class == EXCEPTION_CLASS_SVC_AARCH64) {
    // By convention, x8 has the system call number
    const uint64_t system_call_number_requested = saved_registers[8];

    // Eventually, flesh out system call handler. For now, all calls echo
    // except 0x4444, which reboots, and 0x7777, which panics.
    if (system_call_number_requested == 0x4444) {
      console_print("System call received: 0x4444 ... restart\n");
      return EXCEPTION_ACTION_RESTART;      
    } else if (system_call_number_requested != 0x7777) {
      console_print("System call received: 0x");
      console_print_hex((void *)&system_call_number_requested, 8);
      console_print("\n");
      g_exception_in_progress = 0;
      return EXCEPTION_ACTION_RESUME;
    }
  } else if (kind_of_exception == EXCEPTION_TYPE_IRQ_EL1_SPX) {
    if (handle_interrupt_exception()) {
      return EXCEPTION_ACTION_RESUME;
    }
  }

  // The exception cannot be handled. So now print diagnostics and panic.
  // But first, check if already in an exception.
  // If this is an exception that triggered after another exception, just halt
  // Uses a compiler built-in to check if there's already an exception in
  // progress. atomic relaxed means no cross-thread ordering guarantees
  // beyond this single variable.
  if (__atomic_test_and_set((volatile void *)&g_exception_in_progress,
                            __ATOMIC_RELAXED)) {
    // Nested exception: do not print, just stop immediately.
    halt();
  }

  g_exception_in_progress = 1;

  // Later on, some errors could be handled and recovered from, but for now,
  // just print error information and panic, regardless of the type
  console_print("\n\nSorry, a system error has occurred.\n");
  console_print("Type =  0x");
  console_print_hex((void *)&kind_of_exception, 8);
  console_print(" = ");
  console_print(exception_type_string(kind_of_exception));
  console_print("\n");

  console_print("Class = 0x");
  console_print_hex((void *)&exception_class, 8);
  console_print(" = ");
  console_print(exception_class_to_string(exception_class));
  console_print("\nISS   = 0x");
  console_print_hex((void *)&iss, 8);
  console_print("\n\n");

  console_print("ESR_EL1:  0x");
  console_print_hex((void *)&esr_el1_value, 8);
  console_print("\nELR_EL1:  0x");
  console_print_hex((void *)&elr_el1_value, 8);
  console_print("\nSPSR_EL1: 0x");
  console_print_hex((void *)&spsr_value, 8);
  console_print("\nFAR_EL1:  0x");
  console_print_hex((void *)&far_value, 8);
  console_print("\n\n");

  // Now let's print the saved registers (on arm64, save 32 registers)
  console_print("Saved registers (Register Value): \n");
  for (uint64_t i = 0; i < 32; ++i) {
    console_print("0x");
    console_print_hex((void *)&i, 8);
    console_print(" 0x");
    const uint64_t register_value = saved_registers[i];
    console_print_hex((void *)&register_value, 8);
    console_print("\n");
  }

  // For now, all exceptions are unhandled, so panic
  console_print("Kernel panic: your computer must be restarted.\n");
  return EXCEPTION_ACTION_PANIC;
}
