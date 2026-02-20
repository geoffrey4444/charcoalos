# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run Commands

```bash
# Configure
cmake -S . -B build

# Build all targets
cmake --build build -j

# Run the OS in QEMU
cmake --build build --target run-virt

# Run all tests (unit + symbol checks + QEMU smoke tests)
ctest --test-dir build -j

# Generate Doxygen docs
cmake --build build --target docs
# Output: build/docs/html/index.html
```

Unity is fetched automatically during `cmake -S . -B build` if not present locally (`CHARCOALOS_FETCH_UNITY=ON` by default).

## Architecture Overview

CharcoalOS is a bare-metal AArch64 kernel structured in three independent layers:

```
kernel/          Architecture-independent kernel code
arch/arm64/      AArch64-specific code (exception vectors, interrupts, halt)
platform/virt/   QEMU virt machine target
platform/rpi/    Raspberry Pi 4B target
```

**Boot sequence:** `platform/<target>/boot.s` → parks secondary cores, zeros BSS, sets exception vector base, captures DTB pointer → calls `kmain(dtb)` in [kernel/Main/Main.c](kernel/Main/Main.c) → `kernel_init()` / `kernel_run()` / `kernel_halt()` in [kernel/Main/Lifecycle.c](kernel/Main/Lifecycle.c).

**Exception handling:** [arch/arm64/ExceptionVectors.s](arch/arm64/ExceptionVectors.s) holds a 2048-byte vector table (16 handlers × 128 bytes each). Each handler saves context and dispatches to C in [arch/arm64/ExceptionHandler.c](arch/arm64/ExceptionHandler.c). Synchronous exceptions (SVC, data/instruction aborts) are resumable; IRQ fires the periodic timer tick.

**Console/Shell:** [kernel/Console/IO.c](kernel/Console/IO.c) provides a UART abstraction used by [kernel/Console/Shell.c](kernel/Console/Shell.c). Platform UART drivers live in `platform/<target>/IO.c`. The shell runs in `kernel_run()` and dispatches built-in commands (`help`, `info`, `memread`, `ticks`, `panic`, `reboot`).

**Legacy minimal examples** under `minimal/{virt,rpi}/` are kept as learning references. The primary development path is the refactored `virt`/`rpi` targets.

## Testing Layout

- `tests/Unit/` — Host-side Unity tests (compile/run on host, not target). Cover console I/O, shell parsing, exception dispatch, panic lifecycle.
- `tests/Helpers/` — CTest scripts for symbol-presence checks and QEMU smoke tests (boot output, timer IRQ, exception recovery, SVC resumption).

To run a specific test by name:
```bash
ctest --test-dir build -R <test-name>
```

## Style & Toolchain

- **Language:** C (kernel), AArch64 assembly (boot + exception vectors)
- **Cross-toolchain prefix:** `aarch64-elf-` (`gcc`, `as`, `ld`, `objcopy`)
- **Flags:** `-ffreestanding`, no libc in kernel code
- **Style:** Google C style; format with `clang-format`
- **Comments:** keep concise and technical; explain hardware assumptions and boot-stage constraints

## Key Architectural Constraints

- Kernel runs at EL1; user space will run at EL0 (future phase).
- Platform-specific MMIO addresses (UART, GIC bases) are in `platform/<target>/InterruptMap.h` and `platform/<target>/IO.c` — never hardcode them in `kernel/` or `arch/`.
- Keep architecture-independent code strictly in `kernel/`, AArch64-specific code in `arch/arm64/`, and machine-specific code in `platform/<target>/`.
- Out-of-source builds only; build directories are named `build*`.
