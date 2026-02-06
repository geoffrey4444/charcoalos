<!-- Distributed under the MIT license. -->
<!-- See LICENSE.txt for details. -->

# AGENTS.md

## Purpose
This repository is for learning kernel development on AArch64 by building small, incremental examples.

## Current Targets
- `minimal/virt`: minimal kernel for QEMU `virt` machine.
- `minimal/rpi`: minimal kernel for Raspberry Pi 4B.

## Language And Build Direction
- Architecture focus: AArch64 only.
- Programming language: C for kernel code.
- Build system direction: CMake for project builds.
- Current minimal examples may still use manual toolchain commands documented in source comments.

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
- When adding new experiments, isolate by target directory and keep boot/linker/kernel files close together.

## Testing Layout
- `tests/Unit`: unit tests.
- `tests/Helpers`: shared test utilities (for example, fake UART helpers and test scripts).

## Planned Layout Refactor
- Current state: two separate minimal kernels in `minimal/virt` and `minimal/rpi`.
- Planned direction (not implemented yet):
- Place all AArch64-dependent code in `/arch/arm64`.
- Place all platform-dependent code in `/platform/rpi` and `/platform/virt`.
- Place all platform-independent kernel code in `/kernel`.
- Do not start this refactor until explicitly requested.

## Near-Term Plan
- Introduce a top-level CMake build that can configure and build both current minimal targets.
- Add a shared clang-format configuration aligned with Google style.
- Keep run/debug commands documented per target in their source or target-specific docs.

## Change Guidance For Agents
- Preserve AArch64-only scope unless explicitly requested otherwise.
- Prefer edits that move the repo toward CMake-based builds while keeping working minimal examples.
- Do not remove manual build/run instructions until equivalent CMake targets exist and are verified.
