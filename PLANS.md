# CharcoalOS Plan: Kernel Hello World -> Userland Hello World

This plan is based on what is already working in this repo, and on your background building nand2tetris (hardware + full language/toolchain stack). Codex created the draft plan, but the plan is living and will adjust as seems best to me or as codex advises, as we go along.

## Already Done (Verified)

- ✅ Cross-build + out-of-tree CMake flow for `minimal/*` and refactored `virt`/`rpi` targets.
- ✅ Boot assembly sets up stack, parks secondary cores, zeros `.bss`.
- ✅ UART-backed console I/O abstraction and line input.
- ✅ Kernel main loop with interactive shell and built-in commands (`help`, `info`, `memread`, `add`, `panic`, `reboot`).
- ✅ AArch64 EL1 exception vector table and exception diagnostics in C.
- ✅ Raspberry Pi EL2 -> EL1 drop path and early FP enable.
- ✅ Automated checks: symbol tests, QEMU smoke tests, host-side Unity unit tests.

## Goal

`hello` is no longer a kernel function. It is a user program running at EL0, launched from the kernel shell (for example: `run hello`).

## Roadmap

## Phase 1: Pre-EL0 Contract Fixes (short)

1. Exception vector path must support resume:
   - Replace panic-only flow in `arch/arm64/ExceptionVectors.s` so handled exceptions can restore context and `eret`.
2. Exception handler API must express action and clear state:
   - Update `handle_exception` contract to return an action (for example, panic vs resume), and clear in-progress state on resumable exits.
3. Decouple kernel control from shell:
   - Move panic/reboot control into kernel APIs (for example, `kernel/Panic`), and keep shell as a client.
   - Move the shell command table out of `Shell.h` into `Shell.c` (or a registration API) to avoid header-level coupling.
4. Fix command tokenization bounds:
   - Enforce `max_tokens` in `tokenize_command` to prevent writes past `tokens[MAX_SHELL_ARGS]`.
   - Add unit coverage for over-limit argument counts.
5. Remove console layering leak:
   - Remove platform-header exposure from `kernel/Console/IO.h`; keep platform dependency inside `kernel/Console/IO.c`.
6. Split kernel lifecycle from shell loop:
   - Refactor `kmain` into explicit kernel init/run phases so scheduler/process work does not depend on an always-foreground shell loop.

Exit criteria:
- IRQ and `svc` paths can return safely from exception entry.
- Shell no longer defines kernel-control APIs or command table at header scope.
- Command parsing is bounds-safe under unit test.
- Console interface no longer exposes platform headers.
- Kernel lifecycle allows shell as one client, not the kernel control loop.

## Phase 2: Interrupts + Timer Tick

1. Bring up periodic timer interrupts on `virt` first, then `rpi`.
2. Route IRQ through vector table and acknowledge in platform code.
3. Add a monotonic tick counter and simple sleep primitive.

Exit criteria:
- Kernel prints/records ticks; IRQ path is reliable under QEMU smoke tests.

## Phase 3: Memory Management Foundation

1. Parse/define usable RAM region(s) per platform.
2. Implement page-frame allocator (bitmap or freelist).
3. Reserve kernel image, stacks, MMIO, and allocator metadata regions.

Exit criteria:
- Kernel can allocate/free page frames deterministically with tests.

## Phase 4: Virtual Memory (MMU) for EL1 + EL0

1. Build initial page tables.
2. Enable MMU at EL1 with identity/high-half strategy (pick one and document).
3. Map user address space region separately from kernel mappings.
4. Enforce permissions (user non-exec where needed, kernel inaccessible from EL0).

Exit criteria:
- EL1 runs stable with MMU enabled; deliberate bad access produces expected fault diagnostics.

## Phase 5: EL0 Entry + Minimal Syscall ABI

1. Implement a trap-based syscall entry (`svc`) and syscall dispatcher.
2. Define first syscalls:
   - `write(fd, buf, len)` (stdout only initially)
   - `exit(code)`
3. Implement context structure to enter/return from EL0 cleanly.

Exit criteria:
- A handcrafted EL0 payload can print via syscall and exit back to kernel.

## Phase 6: Process Model + Scheduler (Minimal)

1. Create `task/process` struct (register context, page table root, state, pid).
2. Implement single-core round-robin scheduler.
3. Add context switch path driven by timer tick.
4. Add `wait`/reap path for exited processes.

Exit criteria:
- Kernel can run multiple EL0 tasks and switch between them without corruption.

## Phase 7: User Program Loading

1. Pick initial user binary format:
   - Start simple: flat binary with fixed entry + stack.
   - Then upgrade to ELF64 loader (recommended milestone).
2. Implement loader that maps code/data/stack into a new user address space.
3. Add process creation API: `spawn(program_name, args)`.

Exit criteria:
- Kernel can load `hello` user binary into its own address space and start it.

## Phase 8: Shell-Selectable Userland Hello

1. Add shell command: `run <program>`.
2. Add built-in program table (initially static, e.g. `hello`).
3. Launch user process from shell; return to shell after `exit`.
4. Print clear provenance in output, e.g. `[user:hello] hello world`.

Exit criteria:
- User chooses and runs `hello` as EL0 process from shell.

## Phase 9: Hardening + Learning Extensions

1. Fault containment checks (user invalid memory, bad syscall args).
2. Add `kill`, process list, and basic status reporting.
3. Add regression tests for syscall, scheduler, and loader paths.
4. Optional: move from static program table to initramfs-backed loading.

Exit criteria:
- Reproducible userland execution path with strong diagnostics and tests.

## Suggested Milestone Sequence

1. `M1`: Timer IRQ + tick
2. `M2`: Page allocator
3. `M3`: MMU enabled
4. `M4`: EL0 + `svc write/exit`
5. `M5`: Single user task hello
6. `M6`: `run hello` from shell
7. `M7`: Multi-task + wait/reap

## Why This Fits Your Background

- Your nand2tetris toolchain/compiler experience maps directly to:
  - syscall ABI design,
  - executable format/loading,
  - process abstraction boundaries.
- You can treat each phase as a "vertical slice": architecture + kernel + test, similar to how you likely iterated hardware -> VM -> compiler -> OS in nand2tetris.

## Immediate Next Step

Implement Phase 2 on `virt` first: timer IRQ path + tick counter + smoke test that proves periodic interrupts are actually firing.
