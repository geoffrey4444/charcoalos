// Distributed under the MIT license.
// See LICENSE.txt for details.

#include "arch/Interrupt.h"
#include "kernel/Console/IO.h"
#include "kernel/Time/Uptime.h"
#include "platform/InterruptMap.h"

#include <stdbool.h>
#include <stdint.h>

// The timer we are using is ID 30 = 0x1e (unsigned)
#define TIMER_INTERRUPT_ID 0x1eU

uint64_t read_timer_frequency_in_hz(void) {
  uint64_t freq_in_hz;
  __asm__ volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq_in_hz));
  return freq_in_hz;
}

uint64_t interrupt_frequency_in_hz(void) { return INTERRUPT_FREQUENCY_HZ; }

void initialize_timer(void) {
  // Get the system clock frequency
  const uint64_t freq_in_hz = read_timer_frequency_in_hz();
  const uint64_t clock_ticks_per_interrupt =
      freq_in_hz / interrupt_frequency_in_hz();

  // Set system register: clock ticks per interrupt
  __asm__ volatile("msr CNTP_TVAL_EL0, %0" ::"r"(clock_ticks_per_interrupt));

  // Set system register: physical timer controller
  uint64_t physical_timer_control_value;
  __asm__ volatile("mrs %0, CNTP_CTL_EL0" : "=r"(physical_timer_control_value));
  // CNTP_CTL_EL0 Bit 0: 1 == enabled, 0 == disabled
  //              Bit 1: 1 == masked, 0 == not masked
  // Set last 2 bits [1:0] to 01
  physical_timer_control_value = (physical_timer_control_value & ~0x3) | 0x1;
  // Write back to the register
  __asm__ volatile("msr CNTP_CTL_EL0, %0" ::"r"(physical_timer_control_value));

  // Enable interrupts in timer Generic Interrupt Controller (GIC)
  // for physical timer PPI interrupt ID 30.
  *(volatile uint32_t*)GICD_ISENABLER0 |= (1u << TIMER_INTERRUPT_ID);

  // Set the priority by writing one byte at address GICD_IPRIORITYR + 30.
  // Lower value = higher priority. 0x80 is lower than default 0xFF, but leaves
  // room for more urgent interrupts later.
  *(volatile uint8_t*)(GICD_IPRIORITYR + TIMER_INTERRUPT_ID) = 0x80;

  // Set the prioity filter...for now, allow all priorities through (0xff)
  *(volatile uint32_t*)GICC_PMR = 0xff;

  // Enable signaling and forwarding interrupts.
#if TARGET_RPI
  // On Pi, route timer PPI to Group 1 (non-secure EL1) and enable Group 1.
  *(volatile uint32_t*)GICD_IGROUPR0 |= (1u << TIMER_INTERRUPT_ID);
  *(volatile uint32_t*)GICD_CTLR |= 0x2;
  *(volatile uint32_t*)GICC_CTLR |= 0x2;
#else
  // On QEMU virt, keep Group 0 behavior used by current smoke tests.
  *(volatile uint32_t*)GICD_CTLR |= 0x1;
  *(volatile uint32_t*)GICC_CTLR |= 0x1;
#endif

  // Data and instruction synchronization barriers: make sure the previous
  // instructions take effect before continuing
  // sy means "entire system"
  __asm__ volatile("dsb sy");
  __asm__ volatile("isb");

  // Enable timer in SPSR_EL1 DAIF
  // This clears the D,A,I,F bits in SPSR_EL1
  __asm__ volatile("msr daifclr, #0b0010");
}

bool handle_interrupt_exception(void) {
  // Check if the exception is a timer tick
  uint32_t iar = *(uint32_t*)GICC_IAR;
  // GICC_IAR[9:0] hold the interrupt id, so mask with 2^10-1=0x3ff.
  uint32_t id = iar & 0x3ff;
  bool handled = false;
  if (id == TIMER_INTERRUPT_ID) {
    handled = true;
    increment_uptime_by_one_tick();

    uint64_t freq_in_hz = read_timer_frequency_in_hz();
    uint64_t clock_ticks_per_interrupt =
        freq_in_hz / interrupt_frequency_in_hz();
    __asm__ volatile("msr CNTP_TVAL_EL0, %0" ::"r"(clock_ticks_per_interrupt));
  }

  // Acknowledge finishing handling interrupt exception
  *(uint32_t*)GICC_EOIR = iar;
  return handled;
}

void print_timer_diagnostics(void) {
  uint32_t gicd_typer = *(volatile uint32_t*)GICD_TYPER;
  uint32_t gicd_iidr = *(volatile uint32_t*)GICD_IIDR;
  uint32_t gicc_iidr = *(volatile uint32_t*)GICC_IIDR;
  console_print("Timer diagnostics\n  GICD_TYPER: 0x");
  console_print_hex((void*)&gicd_typer, 4);
  console_print("\n  GICD_IIDR:  0x");
  console_print_hex((void*)&gicd_iidr, 4);
  console_print("\n  GICC_IIDR:  0x");
  console_print_hex((void*)&gicc_iidr, 4);
  console_print("\n");
}
