// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

/*!
 * \brief Foreground client function type run by the kernel main loop.
 */
typedef void (*kernel_foreground_client_t)(void);

/*!
 * \brief Initializes the kernel.
 */
void kernel_init(void);

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
