UNAME := $(shell uname)

CC_EFI   = x86_64-w64-mingw32-g++

ifeq ($(UNAME), Darwin)
	CC_KERN  = x86_64-elf-g++
	LD_KERN  = x86_64-elf-ld
	OC_KERN  = x86_64-elf-objcopy
else
	CC_KERN = g++
	LD_KERN = ld
	OC_KERN = objcopy
endif

EFI_CFLAGS = -Ignu-efi/inc -Iinclude -Ignu-efi/inc/x86_64 -Ignu-efi/inc/protocol \
             -ffreestanding -fno-stack-protector -mno-red-zone -mno-avx \
             -mno-sse -DGNU_EFI_USE_MS_ABI -DCONFIG_x86_64

KERN_CFLAGS = -Iinclude -ffreestanding -fno-stack-protector -mno-red-zone \
              -mno-avx -mno-sse -O2 -Wall

.PHONY: build run clean

build: esp/EFI/BOOT/BOOTX64.EFI esp/kernel.bin

boot.o: src/boot.cpp
	$(CC_EFI) $(EFI_CFLAGS) -c $< -o $@

esp/EFI/BOOT/BOOTX64.EFI: boot.o | esp/EFI/BOOT
	$(CC_EFI) -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o BOOTX64.EFI $< gnu-efi/x86_64/lib/libefi.a
	cp BOOTX64.EFI $@
	rm BOOTX64.EFI

esp/EFI/BOOT:
	mkdir -p esp/EFI/BOOT

kernel.o: src/kernel.cpp
	$(CC_KERN) $(KERN_CFLAGS) -c $< -o $@

kernel.elf: kernel.o font_psf.o kernel.ld
	$(LD_KERN) -T kernel.ld -o $@ kernel.o font_psf.o

font_psf.o: font.psf
	x86_64-elf-objcopy -O elf64-x86-64 -B i386 -I binary font.psf font_psf.o

kernel.bin: kernel.elf
	$(OC_KERN) -O binary $< $@

esp/kernel.bin: kernel.bin
	cp $< $@

run:
	qemu-system-x86_64 -m 17408 -bios RELEASEX64_OVMF.fd \
	    -drive format=raw,file=fat:rw:esp \
	    -serial stdio -display gtk \
		-device VGA,vgamem_mb=64 \

clean:
	rm -f *.o *.elf *.bin *.EFI
	rm -rf esp
