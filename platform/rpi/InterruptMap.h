// Distributed under the MIT license.
// See LICENSE.txt for details.

#pragma once

// Define base addresses for the interrupt controller for timer, IRQs.
// Addresses in the registers entry in the device table are
// 0x40041000 for GICD and 0x40042000 for GICC. However! Codex suggests these
// are bus addresses and must be ranges translated to physical addresses, so
// that the final addresses are 0xFF841000 and 0xFF842000.
// For now, no MMU, so just hard-code the correct physical address.
#define GICD_BASE 0xFF841000
#define GICC_BASE 0xFF842000

#define GICD_CTLR (GICD_BASE + 0x000)
#define GICD_TYPER (GICD_BASE + 0x004)
#define GICD_IIDR (GICD_BASE + 0x008)
#define GICD_IGROUPR0 (GICD_BASE + 0x080)
#define GICD_ISENABLER0 (GICD_BASE + 0x100)
#define GICD_IPRIORITYR (GICD_BASE + 0x400)
#define GICD_ITARGETSR (GICD_BASE + 0x800)

#define GICC_CTLR (GICC_BASE + 0x000)
#define GICC_PMR (GICC_BASE + 0x004)
#define GICC_IAR (GICC_BASE + 0x00C)
#define GICC_EOIR (GICC_BASE + 0x010)
#define GICC_IIDR (GICC_BASE + 0x0FC)
