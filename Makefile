ASM=nasm
CC=i686-elf-gcc
LD=i686-elf-ld
BUILD=build

CFLAGS=-ffreestanding -m32 -Wall -Wextra -c -Ikernel
LDFLAGS=-T link.ld -nostdlib -m elf_i386

KERNEL_C_SOURCES=kernel/kernel.c kernel/vga.c kernel/string.c kernel/keyboard.c kernel/idt.c kernel/isr.c kernel/pic.c kernel/timer.c kernel/pmm.c kernel/paging.c kernel/gdt.c kernel/heap.c kernel/task.c kernel/ata.c kernel/fs.c kernel/usermode.c kernel/syscall.c kernel/elf.c
KERNEL_C_OBJECTS=$(BUILD)/kernel.o $(BUILD)/vga.o $(BUILD)/string.o $(BUILD)/keyboard.o $(BUILD)/idt.o $(BUILD)/isr.o $(BUILD)/pic.o $(BUILD)/timer.o $(BUILD)/pmm.o $(BUILD)/paging.o $(BUILD)/gdt.o $(BUILD)/heap.o $(BUILD)/task.o $(BUILD)/ata.o $(BUILD)/fs.o $(BUILD)/usermode.o $(BUILD)/syscall.o $(BUILD)/elf.o
KERNEL_ASM_OBJECTS=$(BUILD)/kernel_entry.o $(BUILD)/isr_stubs.o $(BUILD)/paging_asm.o $(BUILD)/gdt_asm.o $(BUILD)/task_asm.o $(BUILD)/usermode_asm.o

all: $(BUILD)/os-image.bin

$(BUILD)/boot.bin: boot/boot.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f bin -o $@

$(BUILD)/kernel_entry.o: kernel/kernel_entry.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/isr_stubs.o: kernel/isr.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/paging_asm.o: kernel/paging_asm.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/gdt_asm.o: kernel/gdt_asm.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/task_asm.o: kernel/task_asm.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/usermode_asm.o: kernel/usermode_asm.asm
	mkdir -p $(BUILD)
	$(ASM) $< -f elf32 -o $@

$(BUILD)/kernel.o: kernel/kernel.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/vga.o: kernel/vga.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/string.o: kernel/string.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/keyboard.o: kernel/keyboard.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/idt.o: kernel/idt.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/isr.o: kernel/isr.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/pic.o: kernel/pic.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/timer.o: kernel/timer.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/pmm.o: kernel/pmm.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/paging.o: kernel/paging.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/gdt.o: kernel/gdt.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/heap.o: kernel/heap.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/task.o: kernel/task.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/ata.o: kernel/ata.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/fs.o: kernel/fs.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/usermode.o: kernel/usermode.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/syscall.o: kernel/syscall.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/elf.o: kernel/elf.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD)/kernel.elf: $(KERNEL_ASM_OBJECTS) $(KERNEL_C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_ASM_OBJECTS) $(KERNEL_C_OBJECTS)

$(BUILD)/kernel.bin: $(BUILD)/kernel.elf
	objcopy -O binary $< $@

$(BUILD)/os-image.bin: $(BUILD)/boot.bin $(BUILD)/kernel.bin
	cat $(BUILD)/boot.bin $(BUILD)/kernel.bin > $(BUILD)/os-image.bin
	truncate -s 1474560 $(BUILD)/os-image.bin

$(BUILD)/hello.elf: user/hello.c user/link_user.ld
	mkdir -p $(BUILD)
	$(CC) -ffreestanding -m32 -nostdlib -T user/link_user.ld -o $@ user/hello.c

embed_hello: $(BUILD)/hello.elf
	dd if=$(BUILD)/hello.elf of=hdd.img bs=512 seek=500 conv=notrunc

run: all embed_hello
	bochs -f .bochsrc -q

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
