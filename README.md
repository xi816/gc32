# English

# GC32 - The OG GC32 CPU, based on GC24

## Description
A 32-bit CPU, that was actually [mentioned](https://github.com/xi816/cgovnos/blob/master/docs/govnocore32.odt) in CGovnOS way before even GC16X came out. But, it is assembler-backwards-compatible with the GC24 with minimal ABI changes, and also requiring minimal changes to the code written for GC24.

## Usage
```bash
gcc core/ball.c -o ball        # Bootstrap (use only on non-x86 platforms)
./ball                         # Build the tools and the emulator
./prepare-disk disk.img        # Build GovnBIOS & GovnOS
./gc32 -b bios.img -d disk.img # Run GovnOS on GC32
```
