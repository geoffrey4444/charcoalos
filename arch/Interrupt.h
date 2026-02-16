// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

#define INTERRUPT_FREQUENCY_HZ 100

/*!
 * \brief Read the system timer frequency in hertz.
 * \return The timer frequency in hertz as a uint64_t.
 */
uint64_t read_timer_frequency_in_hz(void);

/*!
 * \brief Return the interrupt frequency in hertz.
 * \return The interrupt frequency in hertz as a uint64_t.
 */
uint64_t interrupt_frequency_in_hz(void);

/*!
 * \brief Initialize the timer for interrupt exceptions (IRQs).
 * \details Call this
 * during kernel initialization to set up the timer to trigger interrupts.
 */
void initialize_timer(void);

/*!
 * \brief Handles an interrupt request exception (IRQ).
 * \details The exception handler calls this when it receives an IRQ exception.
 * For now, this function only handles timer interrupts by incrementing the
 * uptime timer.
 */
void handle_interrupt_exception(void);
