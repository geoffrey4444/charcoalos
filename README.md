<!-- Distributed under the MIT license. -->
<!-- See LICENSE.txt for details. -->

## `charcoalos`
This repo will hold some code to help me learn about kernels, inspired by tutorials at [OSDev wiki](https://wiki.osdev.org/Expanded_Main_Page) and [NAND2Testris](https://www.nand2tetris.org).

Starting from a minimal "hello world" kernel that runs either in [QEMU](https://www.qemu.org) virtually or on the [Raspberry Pi](https://www.raspberrypi.com), I plan to gradually build up code to do kernel-type things (memory, files, hardware, ...).

## Dependencies
- [Unity](https://github.com/ThrowTheSwitch/Unity) (for host-side unit testing)
  - If missing at `third_party/unity`, CMake can fetch it from the official source during configure (`CHARCOALOS_FETCH_UNITY=ON` by default).
- [CMake](https://cmake.org)
- AArch64 cross compiler and linker (for example, `aarch64-elf-gcc` and `aarch64-elf-ld`)
- [QEMU](https://www.qemu.org) (to run/test the `minimal/virt` target)

## Running `minimal/virt` with CMake
- Configure/build:
  - `cmake -S . -B build`
  - `cmake --build build --target run-minimal-virt`
- The `run-minimal-virt` target builds `minimal-virt` first, then runs QEMU with defaults equivalent to:
  - `qemu-system-aarch64 -machine virt,accel=hvf -cpu host -m 512M -nographic -serial mon:stdio -kernel <build>/minimal/virt/kernel.elf`
