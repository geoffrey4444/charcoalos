// Build
// aarch64-elf-as boot.s -o boot.o
// aarch64-elf-gcc -ffreestanding -fno-builtin -nostdlib -c kernel.c -o kernel.o
// aarch64-elf-ld -nostdlib -Tlinker.ld boot.o kernel.o -o kernel.elf
// aarch64-elf-objcopy -O binary kernel.elf kernel8.img
// Run
// qemu-system-aarch64 -machine virt,accel=hvf -cpu host -m 512M -nographic -serial mon:stdio -kernel kernel.elf
// While running: CTRL-A x exits, CTRL-A c switches, CTRL-A h help
//   mon is qemu monitor, --serial mon:stdio sets uart to stdio with monitor
// DTB = device table, bare metal boot puts it at start of RAM
// Examine with aarch64-elf-objdump

#include <stdint.h>

#define MMIO_BASE    0xFE000000UL
#define UART0_BASE   (MMIO_BASE + 0x201000UL) // Pi 4 UART0 (PL011) base
#define UART0_DR     (*(volatile uint32_t *)(UART0_BASE + 0x00)) // TX ("data register")
#define UART0_FR     (*(volatile uint32_t *)(UART0_BASE + 0x18)) // Status ("flag register")

// volatile: do not optimize away or cache with repeated reads
//           don't reorder vs other volatile accesses
//           basically stops compiler from optimizing stuff away
// static: only this file can see it
// const after *: pointer address is constant
static volatile uint8_t * const uart = (uint8_t *) 0x09000000;

static inline void uart_putchar(char character) {
  // if bit 5 of UART0_FR is high, the FIFO for UART is full, so wait
  while (UART0_FR & (1u << 5)) {
    // wait until UART0_FR's 5th bit is not 1
  }
  UART0_DR = (uint32_t)character; // write one character to uart_tx
}

static void uart_print(const char* text) {
  while (*text != '\0') {
    // Continue untl reaching string null terminator
    // Note: we're assuming the characters can be put quickly, and whatever
    // backs it (FIFO of some size) doesn't fill up.
    char character = *text;
    // serial terminal needs CRLF, not just LF (\r\n, not just \n)
    if (character == '\n') {
      uart_putchar('\r');
    }
    uart_putchar(character);
    text++;
  }
}

void kmain(void) {
  uart_print("Hello, world!\n");

  // Put the processor politely to sleep
  // wfe = wait for next event in arm64 assembly
  // __asm__ embeds assembly
  for (;;) {
    __asm__ volatile ("wfe");
  }
}
