// Floating point unit for GC32
#include <cpu32/cpu32h.h>

/* FPU instructions start */

// F8           cif
U8 CIF(GC* gc) {
  F32 reg = gc->reg[(gc->mem[gc->EPC+1])];
  gc->reg[gc->mem[gc->EPC+1]] = *(U32*)&reg;
  gc->EPC += 2;
  return 0;
}

// F9           cfi
U8 CFI(GC* gc) {
  U32 reg = *(F32*)(gc->reg + gc->mem[gc->EPC+1]);
  gc->reg[gc->mem[gc->EPC+1]] = reg;
  gc->EPC += 2;
  return 0;
}

// FA           addf32
U8 ADDF(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  F32 r1 = *(F32*)(gc->reg+rc.x);
  F32 r2 = *(F32*)(gc->reg+rc.y);
  F32 res = r1 + r2;
  gc->reg[rc.x] = *(U32*)& res;
  gc->EPC += 2;
  return 0;
}

// FB           subf32
U8 SUBF(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  F32 r1 = *(F32*)(gc->reg+rc.x);
  F32 r2 = *(F32*)(gc->reg+rc.y);
  F32 res = r1 - r2;
  gc->reg[rc.x] = *(U32*)& res;
  gc->EPC += 2;
  return 0;
}

// FC           mulf32
U8 MULF(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  F32 r1 = *(F32*)(gc->reg+rc.x);
  F32 r2 = *(F32*)(gc->reg+rc.y);
  F32 res = r1 * r2;
  gc->reg[rc.x] = *(U32*)& res;
  gc->EPC += 2;
  return 0;
}

// FD           divf32
U8 DIVF(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  F32 r1 = *(F32*)(gc->reg+rc.x);
  F32 r2 = *(F32*)(gc->reg+rc.y);
  F32 res = r1 / r2;
  gc->reg[rc.x] = *(U32*)& res;
  gc->EPC += 2;
  return 0;
}

// FE           negf32
U8 NEGF(GC* gc) {
  U32 r1 = gc->reg[gc->mem[gc->EPC+1]];
  r1 ^= 0x80000000; // Flip the sign bit
  gc->reg[gc->mem[gc->EPC+1]] = r1;
  gc->EPC += 2;
  return 0;
}

/* FPU instructions end */
