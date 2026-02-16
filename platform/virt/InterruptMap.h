// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

// Define base addresses for the interrupt controller for timer, IRQs.
#define GICD_BASE 0x08000000
#define GICC_BASE 0x08010000

#define GICD_CTLR (GICD_BASE + 0x000)
#define GICD_ISENABLER0 (GICD_BASE + 0x100)
#define GICD_IPRIORITYR (GICD_BASE + 0x400)
#define GICD_ITARGETSR (GICD_BASE + 0x800)


#define GICC_CTLR (GICC_BASE + 0x000)
#define GICC_PMR (GICC_BASE + 0x004)
#define GICC_IAR (GICC_BASE + 0x00C)
#define GICC_EOIR (GICC_BASE + 0x010)
