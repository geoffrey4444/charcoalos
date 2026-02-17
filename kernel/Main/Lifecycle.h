// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

#include <stdint.h>

/*!
 * \brief Foreground client function type run by the kernel main loop.
 */
typedef void (*kernel_foreground_client_t)(void);

/*!
 * \brief Initializes the kernel.
 * \details Initialization includes the following steps:
 * 1. Initialize the (IRQ exception-based) timer
 * 2. Read and parse the device table blob (dtb) for hardware info
 * 3. Set the kernel's foreground client to a default (shell) client, if 
 * not already set by the caller (typically kmain).
 * \param dtb The device tree blob pointer passed from kmain
 */
void kernel_init(uintptr_t dtb);

/*!
 * \brief Sets the foreground client to run in kernel_run.
 * \param client The foreground client callback. If NULL, kernel_run halts.
 */
void kernel_set_foreground_client(kernel_foreground_client_t client);

/*!
 * \brief Runs the kernel.
 */
void kernel_run(void);

/*!
 * \brief Halts the kernel.
 */
void kernel_halt(void);
