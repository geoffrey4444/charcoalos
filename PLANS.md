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

Longer-term goal: support true multiprocessing (SMP) plus multithreading in both kernel and user space, not only single-core multitasking.

Follow-up goal: support persistent files with a minimal file system.

## Roadmap

## ✅ Phase 1: Pre-EL0 Contract Fixes (short)

1. ✅ Exception vector path must support resume:
   - Replace panic-only flow in `arch/arm64/ExceptionVectors.s` so handled exceptions can restore context and `eret`.
2. ✅ Exception handler API must express action and clear state:
   - Update `handle_exception` contract to return an action (for example, panic vs resume), and clear in-progress state on resumable exits.
3. ✅ Decouple kernel control from shell:
   - Move panic/reboot control into kernel APIs (for example, `kernel/Panic`), and keep shell as a client.
   - Move the shell command table out of `Shell.h` into `Shell.c` (or a registration API) to avoid header-level coupling.
4. ✅ Fix command tokenization bounds:
   - Enforce `max_tokens` in `tokenize_command` to prevent writes past `tokens[MAX_SHELL_ARGS]`.
   - Add unit coverage for over-limit argument counts.
5. ✅ Remove console layering leak:
   - Remove platform-header exposure from `kernel/Console/IO.h`; keep platform dependency inside `kernel/Console/IO.c`.
6. ✅ Split kernel lifecycle from shell loop:
   - Refactor `kmain` into explicit kernel init/run phases so scheduler/process work does not depend on an always-foreground shell loop.

Exit criteria:
- ✅ IRQ and `svc` paths can return safely from exception entry.
- ✅ Shell no longer defines kernel-control APIs or command table at header scope.
- ✅ Command parsing is bounds-safe under unit test.
- ✅ Console interface no longer exposes platform headers.
- ✅ Kernel lifecycle allows shell as one client, not the kernel control loop.

## ✅ Phase 2: Interrupts + Timer Tick

Goal:
- ✅ Deliver a reliable periodic timer IRQ on CPU0, first on `virt`, then on `rpi`, with kernel-visible ticks and a shell command to inspect uptime ticks.

2.1. Device tree discovery + interrupt map notes
1. ✅ Dump/decompile device tree for each target:
   - `virt`: use QEMU `dumpdtb`, then `dtc` to DTS.
   - `rpi`: decompile board DTB (and/or live `/sys/firmware/devicetree/base` when booted under Linux).
2. ✅ Record in-source or docs notes for:
   - interrupt controller compatible/version and MMIO base(s),
   - timer node interrupt tuples (which are PPIs, trigger flags, and IDs),
   - chosen timer source (`CNTP_*` or `CNTV_*`) and why. — CNTP, physical timer is simplest for a bare-metal EL1 kernel
3. ✅ Keep these notes target-specific so later SMP work can extend them without rediscovery.

2.2. ✅ Common AArch64 IRQ/timer scaffolding
1. Add architecture-level helpers for:
   - ✅ reading `CNTFRQ_EL0`,
   - ✅ programming timer interval (`*_TVAL` or `*_CVAL`),
   - ✅ enabling/disabling timer (`*_CTL`),
   - ✅ unmasking/remasking IRQ bit in `DAIF`.
2. Define a small IRQ dispatch contract:
   - ✅ platform code can identify/acknowledge active IRQ,
   - ✅ architecture/kernel code handles timer IRQ and returns resume.
3. Keep exception default behavior panic-first for unknown IRQ causes, but make timer IRQ explicitly resumable.

2.3. ✅ `virt` first bring-up (authoritative path)
1. ✅ Implement `platform/virt` GIC init (minimum required for CPU0):
   - init distributor + CPU interface path for the emulated GIC version,
   - enable the timer PPI line for CPU0.
2. ✅ Program generic timer periodic interrupt:
   - pick a fixed interval (for example 10ms or 100Hz),
   - arm timer before unmasking IRQs.
3. ✅ Unmask IRQs only after vector + GIC + timer are ready.
4. ✅ In IRQ handler:
   - detect timer interrupt source,
   - increment global monotonic `tick_count`,
   - re-arm timer,
   - send EOI/deactivate as required,
   - return `EXCEPTION_ACTION_RESUME`.
5. ✅ Add shell command `uptime` to print ticks since boot.

2.4. ✅ `rpi` follow-up bring-up
1. ✅ Reuse architecture timer code and IRQ flow from `virt`.
2. ✅ Implement `platform/rpi` interrupt-controller init + timer PPI enable based on DT/TRM mapping.
3. ✅ Verify EL2->EL1 timer access configuration remains correct and compatible with selected timer source.
4. ✅ Enable same `ticks` command behavior on `rpi`.

2.5. ✅ Tests and validation
1. ✅ Add host/unit coverage for:
   - tick counter API behavior (monotonic increments, reset/init semantics),
   - shell command formatting/parsing for `ticks`.
2. ✅ Add integration smoke for `virt`:
   - boot under QEMU,
   - observe tick count increasing over wall-clock delay,
   - assert IRQ handler returns and system remains interactive.
3. ✅ Keep panic path independent from timer/IRQ availability (consistent with Phase 2.5).

Exit criteria:
- ✅ `virt`: periodic generic timer IRQ is firing on CPU0, tick count increases monotonically, and `ticks` command reports non-zero increasing values.
- ✅ `rpi`: same externally visible behavior (`ticks` increases) using platform-specific interrupt-controller setup.
- ✅ IRQ unknown-source behavior is diagnosable and does not silently continue.
- ✅ QEMU smoke test proves periodic interrupts are actually firing and kernel resumes from IRQ repeatedly.

## ✅ Phase 2.5: Panic UART Drain Reliability

1. ✅ Add a platform console TX drain hook (for example, `platform_console_flush_tx()`).
2. ✅ Implement drain per target (`virt` + `rpi`) using polling on UART TX-idle status bits.
3. ✅ Call TX drain from panic/fatal paths before halt/restart.
4. ✅ Add/update unit tests for panic lifecycle to verify flush is invoked on panic paths.
5. ✅ Remove __atomic_test_and_set() in exception handler, so diagnostic text prints on rpi on unhandled exception

Exit criteria:
- ✅ Panic diagnostics are consistently visible on `rpi` and `virt` before halt/restart.
- ✅ Panic path remains polling-based and does not depend on IRQ/timer availability.

## Phase 3: Memory Management Foundation

Goal:
- Build a physical-memory allocator that can use all available RAM and is ready for MMU/page-table allocation.

Decision:
- Keep Phase 3 and Phase 4 separate.
- Phase 3 owns physical memory discovery + page-frame allocation.
- Phase 4 consumes that allocator to build page tables and turn on virtual memory.
- This separation avoids circular bring-up problems and makes debugging much easier.

3.1. RAM discovery strategy (runtime first, hard-coded fallback)
1. Capture the boot DTB (device tree blob) pointer from boot assembly and pass/store it for C init.
2. Parse RAM from DTB `memory` node `reg` entries on both `virt` and `rpi`.
3. Also parse reserved regions (`/memreserve/` and `reserved-memory`) so allocator never hands those out.
4. If DTB parsing fails, use a conservative fallback RAM region per platform only to boot; keep fallback documented as temporary.
5. Normalize all discovered RAM ranges:
   - align start up to 4 KiB page size,
   - align end down to 4 KiB,
   - discard empty ranges.

3.2. Reserve regions before allocator init
1. Build a simple "physical memory map" table with entries marked `USABLE` or `RESERVED`.
2. Reserve:
   - kernel image range (`.text/.rodata/.data/.bss`) via linker symbols,
   - boot/kernel stack range via linker symbols,
   - MMIO windows (from platform constants and/or DTB ranges),
   - DTB blob itself,
   - allocator metadata memory (bitmap + page descriptors).
3. Keep this reserve API explicit (for example, `pmm_reserve_range(pa_start, pa_end, reason)`), because later phases will reuse it for ELF/user mappings and DMA-safe reservations.

3.3. Allocator choice and data structures
1. Use a bitmap page-frame allocator now (best minimal base for performance + total-RAM scalability):
   - 1 bit per 4 KiB page frame (`0=free`, `1=used`),
   - allocation scan with "next-fit cursor" to avoid always scanning from page 0,
   - deterministic O(n/word) worst-case scan, very small metadata.
2. Add a minimal per-page metadata array (`struct page`) now so later phases do not require a rewrite:
   - `flags` (reserved/kernel/table/etc.),
   - `refcount` (for shared mappings/COW groundwork),
   - optional `order_or_next` field reserved for future buddy/slab integration.
3. Keep allocator API page-granular:
   - `pmm_alloc_page()`,
   - `pmm_free_page(pa)`,
   - `pmm_alloc_pages(count)` (contiguous, first-fit; can be slow initially),
   - `pmm_mark_used(pa, size)` for reservation plumbing.

3.4. Tests for Phase 3
1. Unit-test range normalize/reserve overlap behavior.
2. Unit-test allocate/free determinism and double-free detection.
3. Unit-test "never allocate reserved frame" invariants.
4. Add simple stress test: allocate-until-full, then free-all, verify exact frame count.

Exit criteria:
- Kernel discovers RAM at runtime (DTB path), falls back safely if needed, and can allocate/free page frames deterministically with tests.
- Reserved regions are never returned by allocator.
- API is stable enough to back page-table allocation in Phase 4.

## Phase 4: Virtual Memory (MMU) for EL1 + EL0

Goal:
- Enable MMU at EL1 with kernel in high-half virtual addresses, while deferring full user allocation policy to late Phase 4 / early Phase 5.

4.0. Concepts, acronyms, and registers used in this phase
1. `MMU`: Memory Management Unit; translates virtual addresses (VA) to physical addresses (PA) and enforces permissions.
2. `VA` / `PA`: virtual vs physical address.
3. `EL1` / `EL0`: kernel privilege level vs user privilege level.
4. `TTBR0_EL1`: Translation Table Base Register 0 (typically user-space page-table root).
5. `TTBR1_EL1`: Translation Table Base Register 1 (typically kernel high-half page-table root).
6. `TCR_EL1`: Translation Control Register; chooses page size/granule, VA size, cacheability/shareability defaults, and TTBR split behavior.
7. `MAIR_EL1`: Memory Attribute Indirection Register; defines attribute encodings (normal cacheable RAM vs device memory).
8. `SCTLR_EL1`: System Control Register; enabling MMU/caches uses bits `M` (MMU), `C` (data cache), `I` (instruction cache).
9. `ESR_EL1`: Exception Syndrome Register; tells why a fault happened.
10. `FAR_EL1`: Fault Address Register; VA that faulted.
11. `VBAR_EL1`: exception vector base address; must remain valid after MMU switch.
12. `PXN` / `UXN`: Privileged/User Execute Never page permission bits.
13. `AF`: Access Flag bit in page descriptors; should be set for mapped pages.

4.1. Virtual address layout (high-half kernel design)
1. Use 4 KiB granule and 48-bit VA scheme.
2. Put kernel mappings in high-half via `TTBR1_EL1` (kernel-only).
3. Keep `TTBR0_EL1` disabled at first (`EPD0=1` in `TCR_EL1`) until user-space mappings are introduced.
4. During initial bring-up, map both:
   - identity map for current execution transition safety,
   - high-half kernel map (final steady-state kernel execution).
5. After high-half jump is stable, remove identity map in a cleanup step (can be end of Phase 4).

4.2. Page-table build plan
1. Allocate page-table pages from Phase 3 allocator only.
2. Implement a reusable `map_range(va, pa, size, attrs)` helper.
3. Use block mappings where valid (to reduce table count and keep setup fast), and page mappings for fine-grained kernel section permissions.
4. Map at least:
   - kernel `.text` as read-only + executable (EL1 only),
   - kernel `.rodata` as read-only + non-executable,
   - kernel `.data/.bss/.stack` as read-write + non-executable,
   - MMIO as Device-nGnRE + non-executable + strongly ordered device semantics.
5. Keep an early direct-map (physmap) window in high-half so kernel can access physical frames predictably by VA.

4.3. MMU enable sequence (EL1)
1. Build and zero all needed translation tables in physical memory.
2. Program `MAIR_EL1` (at minimum):
   - AttrIdx 0: normal WB/WA cacheable memory,
   - AttrIdx 1: device nGnRE memory for MMIO.
3. Program `TCR_EL1` for:
   - 4 KiB granule (`TG0/TG1`),
   - desired VA size (`T0SZ/T1SZ`),
   - shareability/cache policy (`SHx/IRGNx/ORGNx`),
   - `EPD0=1` initially (disable TTBR0 walks until user-space ready).
4. Write `TTBR1_EL1` with kernel root table PA; set `TTBR0_EL1` to known-safe value.
5. Barrier sequence before/after control changes (`DSB ISH`, `ISB`).
6. Set `SCTLR_EL1.M=1`, `SCTLR_EL1.C=1`, `SCTLR_EL1.I=1`; preserve required RES1 bits.
7. `ISB`, then branch to a known high-half kernel VA label.
8. Re-load `VBAR_EL1` with a valid virtual address (if not already high-half-safe).

4.4. Fault diagnostics and validation
1. Extend exception diagnostics to decode and print `ESR_EL1` class/reason plus `FAR_EL1`.
2. Add deliberate fault tests:
   - execute from NX page,
   - write to read-only page,
   - touch unmapped VA.
3. Confirm kernel remains stable with MMU/caches on under timer IRQ load.

4.5. User-space boundary for Phase 4
1. Keep full user allocator/paging policy out of early Phase 4.
2. Late Phase 4 (or start Phase 5):
   - define user VA window under `TTBR0_EL1`,
   - re-enable walks for TTBR0 (`EPD0=0`) only when first EL0 program path is ready.
3. Ensure kernel pages are inaccessible to EL0 (`AP`/`PXN`/`UXN` settings) from day one.

Exit criteria:
- EL1 runs stably in high-half virtual space with MMU enabled.
- Kernel permissions are enforced (`.text` RO+X, writable regions NX, MMIO device attributes).
- Fault diagnostics show actionable `ESR_EL1` + `FAR_EL1` output.
- TTBR0/user mappings are structurally prepared but user allocation policy remains deferred to Phase 5 entry.

## Phase 5: EL0 Entry + Minimal Syscall ABI

1. Implement a trap-based syscall entry (`svc`) and syscall dispatcher.
2. Define first syscalls:
   - `write(fd, buf, len)` (stdout only initially)
   - `exit(code)`
3. Implement context structure to enter/return from EL0 cleanly.

Exit criteria:
- A handcrafted EL0 payload can print via syscall and exit back to kernel.

## Phase 6: Task/Thread Model + Scheduler (UP Baseline)

1. Replace process-only model with task model:
   - `process` (address space + resources),
   - `thread` (CPU context + kernel stack + state), with one thread as initial process main thread.
2. Add kernel thread support (`kthread_create`, run function + arg) sharing kernel address space.
3. Implement single-core round-robin scheduler over runnable threads.
4. Add context switch path driven by timer tick.
5. Add `wait`/reap for process exit and thread lifecycle bookkeeping.

Exit criteria:
- Kernel can run and switch multiple kernel/user threads on one core without corruption.
- Process and thread objects are distinct and ready for SMP scaling.

## Phase 7: Concurrency Primitives (SMP-Ready APIs)

1. Add spinlock API and IRQ-safe lock variants for kernel internal use.
2. Add blocking wait queues and wakeup primitives used by scheduler and syscalls.
3. Introduce per-CPU data abstractions (`current_cpu`, `current_thread`, per-CPU run queue hook points), even if only CPU0 is active.
4. Convert global mutable scheduler/task paths to use the new primitives.

Exit criteria:
- No scheduler-critical shared state is protected only by "single-core assumption".
- Core kernel paths compile cleanly against lock/wait queue interfaces.

## Phase 8: SMP Bring-Up (Kernel)

1. Bring up secondary cores on `virt` first, then `rpi`:
   - per-core boot entry, per-core stack, per-core exception vectors/state.
2. Add inter-processor interrupt (IPI) support for scheduler reschedule/wakeup.
3. Move from global run queue to per-CPU run queues (or global + locking as first step, then per-CPU).
4. Enable timer tick and preemption on all active cores.
5. Add SMP-safe shutdown/restart/panic behavior (stop other cores, ordered panic output).

Exit criteria:
- Multiple cores run kernel threads concurrently on `virt`.
- Scheduler correctness holds under parallel execution and timer preemption.
- `rpi` boots with secondary cores online (even if feature-limited initially).

## Phase 9: User Program Loading

1. Pick initial user binary format:
   - Start simple: flat binary with fixed entry + stack.
   - Then upgrade to ELF64 loader (recommended milestone).
2. Implement loader that maps code/data/stack into a new user address space.
3. Add process creation API: `spawn(program_name, args)`.

Exit criteria:
- Kernel can load `hello` user binary into its own address space and start it.

## Phase 10: Shell-Selectable Userland Hello

1. Add shell command: `run <program>`.
2. Add built-in program table (initially static, e.g. `hello`).
3. Launch user process from shell; return to shell after `exit`.
4. Print clear provenance in output, e.g. `[user:hello] hello world`.

Exit criteria:
- User chooses and runs `hello` as EL0 process from shell.

## Phase 11: User-Space Multithreading

1. Add thread syscalls (for example `thread_create`, `thread_exit`, `thread_join`; design names/ABI explicitly).
2. Define user thread memory model details:
   - user stack allocation policy,
   - thread-local storage bootstrap strategy.
3. Add synchronization primitives exposed to userland (start with kernel-assisted primitive such as futex-like wait/wake or a minimal mutex syscall set).
4. Ensure scheduler can run multiple threads from the same process across different CPUs.
5. Add sample multithreaded user program and shell command to run it.

Exit criteria:
- One user process can create and join multiple threads.
- User threads execute in parallel on multiple CPUs under `virt`.

## Phase 12: Persistence Foundation (Block I/O + VFS)

1. Add a block-device abstraction in kernel (`read_blocks`, `write_blocks`, geometry/capabilities).
2. Implement first block backends:
   - `virt`: virtual disk path (for example QEMU-provided block device),
   - `rpi`: SD/eMMC storage path for Pi.
3. Add a minimal VFS layer:
   - vnode/inode-like object model,
   - mount table and path lookup.
4. Implement an in-memory filesystem first (for VFS validation), then a minimal persistent on-disk filesystem.
5. Add buffer/page cache basics and explicit sync/flush path for durability.
6. Expose first file syscalls (`open`, `close`, `read`, `write`, `lseek`, `mkdir` minimal subset).
7. Add shell commands for filesystem bring-up (`ls`, `cat`, `writefile`, `mount` minimal variants).

Exit criteria:
- Kernel can mount a persistent filesystem on `virt` and `rpi`.
- Files created in one boot are readable after reboot.
- User programs can read/write persistent files via syscalls.

## Phase 13: Hardening + Learning Extensions

1. Fault containment checks (user invalid memory, bad syscall args).
2. Add `kill`, process list, and basic status reporting.
3. Add regression tests for syscall, scheduler, loader, SMP, user-threading, and filesystem paths.
4. Optional: move from static program table to initramfs-backed loading.

Exit criteria:
- Reproducible userland execution path with strong diagnostics and tests.

## Suggested Milestone Sequence

1. `M1`: Timer IRQ + tick
2. `M2`: Page allocator
3. `M3`: MMU enabled
4. `M4`: EL0 + `svc write/exit`
5. `M5`: Unified task/thread model on one core
6. `M6`: SMP kernel scheduling on `virt`
7. `M7`: User program loading + `run hello`
8. `M8`: User multithreading across cores
9. `M9`: Persistent filesystem on `virt` + `rpi`

## Why This Fits Your Background

- Your nand2tetris toolchain/compiler experience maps directly to:
  - syscall ABI design,
  - executable format/loading,
  - process abstraction boundaries.
- You can treat each phase as a "vertical slice": architecture + kernel + test, similar to how you likely iterated hardware -> VM -> compiler -> OS in nand2tetris.

## Immediate Next Step

Implement Phase 3.1 and 3.2 first:
1. Capture/pass DTB pointer into C init.
2. Build RAM/reserved range table and reservation API.
3. Add a minimal bitmap allocator over 4 KiB page frames.
