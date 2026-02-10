<!-- Distributed under the MIT license. -->
<!-- See LICENSE.txt for details. -->

## `charcoalos`
This repo holds some code to help me learn about kernels, inspired by tutorials at [OSDev wiki](https://wiki.osdev.org/Expanded_Main_Page) and [NAND2Testris](https://www.nand2tetris.org).

Starting from a minimal "hello world" kernel that runs either in [QEMU](https://www.qemu.org) virtually or on the [Raspberry Pi](https://www.raspberrypi.com), I am gradually building up code to do kernel-type things (memory, files, hardware, ...).

## Please don't try to use this OS for anything at all important
This repo is to help me learn how operating systems work. That means I don't yet know, which means I'll probably make mistakes, including mistakes with security implications. I'll do my best as I go to learn how to avoid introducing security vulnerabilities. And anyway, this code is a long, long way from being useful for much of anything. Even so, if you happened to find this repo, and for some reason you felt inclined to try to run it, please don't try to use it on any production system or in any situation where correctness, security, etc. matter.

## Dependencies
- [Unity](https://github.com/ThrowTheSwitch/Unity) (for host-side unit testing)
  - If missing at `third_party/unity`, CMake can fetch it from the official source during configure (`CHARCOALOS_FETCH_UNITY=ON` by default).
- [Doxygen](https://www.doxygen.nl) (optional, for building the documentation)
- [CMake](https://cmake.org)
- AArch64 cross-compiler and linker (for example, `aarch64-elf-gcc` and `aarch64-elf-ld`)
- [QEMU](https://www.qemu.org) (to run/test the `minimal/virt` target)

## Building, running, and testing
- Configure/build:
  - `cmake -S . -B build`
  - `cmake --build build -j`
- Generate documentation (requires Doxygen):
  - `cmake --build build --target docs`
  - HTML output: `build/docs/html/index.html`
- The `run-virt` target builds and then runs the OS on a QEMU virtual machine, equivalent to the following command:
  - `qemu-system-aarch64 -machine virt,accel=hvf -cpu host -m 512M -nographic -serial mon:stdio -kernel <build>/minimal/virt/kernel.elf`
  - Do `cmake --build build --target run-virt` to run the virtual machine in QEMU.
- To run the unit tests, do `ctest --test-dir build -j`.
