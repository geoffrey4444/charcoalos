// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/ExceptionHandler.h"
#include "arch/arm64/ExceptionTypes.h"
#include "unity.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static char g_tx_buffer[8192];
static size_t g_tx_index;
static size_t g_halt_calls;
static size_t g_interrupt_handler_calls;
static bool g_interrupt_handler_result;
static bool g_exit_on_halt;
static jmp_buf g_halt_jmp;
static uint64_t g_mock_esr_el1;
static uint64_t g_mock_elr_el1;
static uint64_t g_mock_spsr_el1;
static uint64_t g_mock_far_el1;

void arch_exception_set_in_progress_for_test(uint64_t value);

uint64_t arch_exception_read_esr_el1(void) { return g_mock_esr_el1; }
uint64_t arch_exception_read_elr_el1(void) { return g_mock_elr_el1; }
uint64_t arch_exception_read_spsr_el1(void) { return g_mock_spsr_el1; }
uint64_t arch_exception_read_far_el1(void) { return g_mock_far_el1; }

void platform_console_putc(char c) {
  if (g_tx_index < sizeof(g_tx_buffer)) {
    g_tx_buffer[g_tx_index++] = c;
  }
}

char platform_console_getc(void) { return '\0'; }

void halt(void) {
  ++g_halt_calls;
  if (g_exit_on_halt) {
    longjmp(g_halt_jmp, 1);
  }
}

bool handle_interrupt_exception(void) {
  ++g_interrupt_handler_calls;
  return g_interrupt_handler_result;
}

static void assert_tx_contains(const char *needle) {
  TEST_ASSERT_NOT_NULL(strstr(g_tx_buffer, needle));
}

void setUp(void) {
  memset(g_tx_buffer, 0, sizeof(g_tx_buffer));
  g_tx_index = 0;
  g_halt_calls = 0;
  g_interrupt_handler_calls = 0;
  g_interrupt_handler_result = true;
  g_exit_on_halt = false;
  g_mock_esr_el1 = 0;
  g_mock_elr_el1 = 0;
  g_mock_spsr_el1 = 0;
  g_mock_far_el1 = 0;
  arch_exception_set_in_progress_for_test(0);
}

void tearDown(void) {}

void test_handle_exception_prints_diagnostics_and_panics(void) {
  uint64_t saved_registers[32] = {0};
  for (uint64_t i = 0; i < 32; ++i) {
    saved_registers[i] = 0x1000ULL + i;
  }

  uint64_t action =
      handle_exception(saved_registers, EXCEPTION_TYPE_SYNC_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(action, EXCEPTION_ACTION_PANIC);
  assert_tx_contains("Sorry, a system error has occurred.");
  assert_tx_contains("Type =  0x0000000000000004 = SYNC_EL1_SPX");
  assert_tx_contains("Class = 0x0000000000000000 = EXCEPTION_CLASS_UNKNOWN");
  assert_tx_contains("Saved registers (Register Value):");
  assert_tx_contains("0x0000000000000000 0x0000000000001000");
  assert_tx_contains("0x000000000000001F 0x000000000000101F");
  assert_tx_contains("Kernel panic: your computer must be restarted.");
}

void test_handle_exception_irq_calls_interrupt_handler_and_resumes(void) {
  uint64_t saved_registers[32] = {0};
  g_interrupt_handler_result = true;

  uint64_t action = handle_exception(saved_registers, EXCEPTION_TYPE_IRQ_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(EXCEPTION_ACTION_RESUME, action);
  TEST_ASSERT_EQUAL_size_t(1, g_interrupt_handler_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_halt_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_handle_exception_irq_unhandled_source_panics(void) {
  uint64_t saved_registers[32] = {0};
  g_interrupt_handler_result = false;

  uint64_t action = handle_exception(saved_registers, EXCEPTION_TYPE_IRQ_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(EXCEPTION_ACTION_PANIC, action);
  TEST_ASSERT_EQUAL_size_t(1, g_interrupt_handler_calls);
  assert_tx_contains("Sorry, a system error has occurred.");
  assert_tx_contains("Type =  0x0000000000000005 = IRQ_EL1_SPX");
  assert_tx_contains("Kernel panic: your computer must be restarted.");
}

void test_handle_exception_irq_ignores_stale_svc_esr_class(void) {
  uint64_t saved_registers[32] = {0};
  saved_registers[8] = 0x44;
  g_interrupt_handler_result = true;
  g_mock_esr_el1 = ((uint64_t)EXCEPTION_CLASS_SVC_AARCH64 << 26);

  uint64_t action = handle_exception(saved_registers, EXCEPTION_TYPE_IRQ_EL1_SPX);

  TEST_ASSERT_EQUAL_size_t(EXCEPTION_ACTION_RESUME, action);
  TEST_ASSERT_EQUAL_size_t(1, g_interrupt_handler_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_halt_calls);
  TEST_ASSERT_EQUAL_size_t(0, g_tx_index);
}

void test_handle_exception_nested_fatal_exception_halts_without_extra_output(void) {
  uint64_t saved_registers[32] = {0};
  arch_exception_set_in_progress_for_test(1);
  const size_t tx_before_nested_exception = g_tx_index;

  g_exit_on_halt = true;
  if (setjmp(g_halt_jmp) == 0) {
    (void)handle_exception(saved_registers, EXCEPTION_TYPE_SYNC_EL1_SPX);
    TEST_FAIL_MESSAGE("Expected nested exception path to call halt()");
  }
  g_exit_on_halt = false;

  TEST_ASSERT_EQUAL_size_t(1, g_halt_calls);
  TEST_ASSERT_EQUAL_size_t(tx_before_nested_exception, g_tx_index);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_handle_exception_prints_diagnostics_and_panics);
  RUN_TEST(test_handle_exception_irq_calls_interrupt_handler_and_resumes);
  RUN_TEST(test_handle_exception_irq_unhandled_source_panics);
  RUN_TEST(test_handle_exception_irq_ignores_stale_svc_esr_class);
  RUN_TEST(test_handle_exception_nested_fatal_exception_halts_without_extra_output);
  return UNITY_END();
}
