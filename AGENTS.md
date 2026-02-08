<!-- Distributed under the MIT license. -->
<!-- See LICENSE.txt for details. -->

# AGENTS.md

## Purpose
This repository is for learning kernel development on AArch64 by building small, incremental examples.

## Current Targets
- Legacy minimal examples:
  - `minimal/virt`: minimal kernel for QEMU `virt` machine.
  - `minimal/rpi`: minimal kernel for Raspberry Pi 4B.
- Refactored targets (current main path):
  - `virt`: links `platform/virt` + `kernel` + `arch/arm64` into `build/virt/kernel.elf`.
  - `rpi`: links `platform/rpi` + `kernel` + `arch/arm64` and emits `build/rpi/kernel8.img`.

## Language And Build Direction
- Architecture focus: AArch64 only.
- Programming language: C for kernel code.
- Build system: top-level CMake is the primary build path.
- Minimal examples still keep manual toolchain commands in source comments for learning/reference.

## Style Conventions
- C code style target: Google style.
- Formatting tool: `clang-format`.
- Keep comments concise and technical; explain hardware assumptions and boot-stage constraints.

## Toolchain Assumptions
- Cross toolchain prefix: `aarch64-elf-` (assembler, compiler, linker, objcopy, objdump).
- No hosted runtime assumptions in kernel code (`-ffreestanding`, no libc dependence unless explicitly introduced).

## Repository Practices
- Prefer out-of-source builds in root directories named `build*`.
- Keep examples small and focused on one concept at a time.
- Keep architecture-independent kernel code in `kernel/`, AArch64 arch code in `arch/arm64/`, and platform code in `platform/<target>/`.
- Keep legacy minimal examples isolated under `minimal/<target>/`.

## Testing Layout
- `tests/Unit`: unit tests.
- `tests/Helpers`: CTest helper scripts for symbol checks and smoke tests.
- Current automated coverage includes:
  - symbol checks for `minimal/*` and refactored `virt`/`rpi` outputs,
  - QEMU smoke test for refactored `virt`,
  - host-side Unity tests for console I/O behavior.

## Near-Term Plan
- Keep both legacy `minimal/*` and refactored `virt`/`rpi` targets building in CMake.
- Continue expanding host-side unit tests in `tests/Unit`.
- Keep run/debug commands documented per target in source comments or target-specific docs.

## Change Guidance For Agents
- Preserve AArch64-only scope unless explicitly requested otherwise.
- Prefer edits that improve the refactored `kernel` + `arch` + `platform` structure while keeping minimal examples working.
- Do not remove manual build/run instructions from minimal examples unless equivalent CMake targets remain available and verified.
