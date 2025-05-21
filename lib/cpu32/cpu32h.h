// Header include file for lib/cpu32/cpu32.h
#ifndef CPU32H_H
#define CPU32H_H 1

/*
  The memory size is set to 32 MiB or 33,554,432 bytes.
*/
#define ROMSIZE 16777216
#define MEMSIZE 33554432

// Register cluster
struct gcrc {
  U8 x;
  U8 y;
};
typedef struct gcrc gcrc_t;

struct GC32 {
  // Govno Core 32's 32 addressable registers, but only 16 can be used without a swap.
  U32 reg[32];
  U8 PS;           // -I---ZNC                 Unaddressable
  U32 EPC;         // Program counter (32-bit) Unaddressable

  // Memory and ROM
  U8* mem;
  U8* rom;
  U8 pin;

  // GPU
  gc_gg16 gg;
  SDL_Renderer* renderer;
};
typedef struct GC32 GC;
void PlayBeep(double frequency);
gcrc_t ReadRegClust(U8 clust);

U0 WriteByte(GC* gc, U32 addr, U8 val);
U0 WriteWord(GC* gc, U32 addr, U16 val);
U0 Write32(GC* gc, U32 addr, U32 val);

U0 InitGC(GC* gc) {
  gc->mem = (U8*)malloc(MEMSIZE);
  gc->rom = (U8*)malloc(ROMSIZE);
}

#endif
