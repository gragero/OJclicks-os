# OJclicks OS

Look, this is a 32-bit x86 operating system and kernel written from scratch in C and Assembly. It doesn't have a bloated web browser, it doesn't have a fancy GUI, and it certainly doesn't care about your feelings. It's just raw code talking directly to the hardware, the way programming was meant to be before people got lazy.

If you don't understand how a GDT, an IDT, or Paging works, go read a textbook before you break my code.

## Features

* Custom 32-bit Kernel entry (because GRUB is too mainstream and bloated)
* GDT, IDT, and Paging initialized from scratch (if you still use standard BIOS interrupts, please leave)
* Programmable Interrupt Controller (PIC) and a Keyboard driver that actually works
* ATA Hard Disk drive driver and a basic File System (don't expect ext4 yet, write it yourself if you're so smart)
* Physical Memory Manager (PMM) and Virtual Memory Heap (managing memory properly, unlike modern web apps)
* Initial Usermode architecture and basic System Calls
* Custom C Shell (ojcsh) for testing this whole thing without losing your mind

## Building and Running

If you are on a broken operating system, go install Arch Linux first. You will need `nasm`, `gcc` (configured for i686-elf targets, don't use your host compiler like an amateur), and `make`.

To compile the kernel and build the disk image, run:

Code output

Sarcastic English file generated successfully.

```bash
make clean
make
```

If it throws compilation errors, it's 99% your environment's fault, not my code. Fix your toolchain.

To test the generated disk.img using QEMU:
Bash

qemu-system-i386 -hda disk.img

If it crashes your machine, congratulations, you just learned how low-level development works.
