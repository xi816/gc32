// CPU identificator: GC32
#include <cpu32/proc/std.h>
#include <cpu32/proc/interrupts.h>
#include <cpu32/hid.h>
#include <cpu32/gpu.h>
#include <cpu32/spu.h>
#include <cpu32/fpu.h>

#define BIOSNOBNK 16
#define BANKSIZE 65536

/*
  CPU info:
  Speed: 8.5THz
  State: Holy 2.0
*/

enum {
  EAX,EBX,ECX,EDX,ESI,EGI,ESP,EBP,
  E8, E9, E10,E11,E12,E13,E14,E15,
  E16,E17,E18,E19,E20,E21,E22,E23,
  E24,E25,E26,E27,E28,E29,E30,E31
};

enum {
  R0, R1, R2, R3, R4, R5, R6, R7,
  R8, R9, R10,R11,R12,R13,R14,R15,
  R16,R17,R18,R19,R20,R21,R22,R23,
  R24,R25,R26,R27,R28,R29,R30,R31
};

#define IF(ps) (ps & 0b01000000)
#define ZF(ps) (ps & 0b00000100)
#define NF(ps) (ps & 0b00000010)
#define CF(ps) (ps & 0b00000001)

#define SET_IF(ps) (ps |= 0b01000000)
#define SET_ZF(ps) (ps |= 0b00000100)
#define SET_NF(ps) (ps |= 0b00000010)
#define SET_CF(ps) (ps |= 0b00000001)

#define RESET_IF(ps) (ps &= 0b10111111)
#define RESET_ZF(ps) (ps &= 0b11111011)
#define RESET_NF(ps) (ps &= 0b11111101)
#define RESET_CF(ps) (ps &= 0b11111110)

#define printh(c, s) printf("%02X" s, c)

U8 gc_errno;

U0 Reset(GC* gc);
U0 PageDump(GC* gc, U8 page);
U0 StackDump(GC* gc, U16 c);
U0 RegDump(GC* gc);

// NOTE: every multibyte reads and writes are done in Little Endian

/* ReadWord -- Read a 16-bit value from memory */
U16 ReadWord(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8);
}

/* Read24 -- Read a 24-bit value from memory */
U32 Read24(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8) + (gc->mem[addr+2] << 16);
}

/* Read32 -- Read a 32-bit value from memory */
U32 Read32(GC* gc, U32 addr) {
  return (gc->mem[addr]) + (gc->mem[addr+1] << 8) + (gc->mem[addr+2] << 16) + (gc->mem[addr+3] << 24);
}

/* WriteWord -- Write a 16-bit value to memory */
U0 WriteWord(GC* gc, U32 addr, U16 val) {
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = (val >> 8);
}

/* Write24 -- Write a 24-bit value to memory */
U0 Write24(GC* gc, U32 addr, U32 val) {
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = ((val >> 8) % 256);
  gc->mem[addr+2] = ((val >> 16) % 256);
}

/* Write32 -- Write a 32-bit value to memory */
U0 Write32(GC* gc, U32 addr, U32 val) {
  gc->mem[addr] = (val % 256);
  gc->mem[addr+1] = ((val >> 8) % 256);
  gc->mem[addr+2] = ((val >> 16) % 256);
  gc->mem[addr+3] = ((val >> 24) % 256);
}

/* StackPush -- Push a 32-bit value onto the stack */
U8 StackPush(GC* gc, U32 val) {
  gc->mem[gc->reg[ESP]] = (val >> 24);
  gc->mem[gc->reg[ESP]-1] = ((val >> 16) % 256);
  gc->mem[gc->reg[ESP]-2] = ((val >> 8) % 256);
  gc->mem[gc->reg[ESP]-3] = (val % 256);
  gc->reg[ESP] -= 4;
  return 0;
}

/* StackPop -- Pop a 32-bit value and return it */
U32 StackPop(GC* gc) {
  gc->reg[ESP] += 4;
  return Read32(gc, gc->reg[ESP]-3);
}

/* ReadRegClust -- A function to read the register cluster
  A register cluster is a byte that consists of 2 register pointers:
  It is divided into 2 parts like this:
    0000 1100
  The first 4-byte value is a first operand, and the last 4-byte value
  is a second operand.
  The function returns a structure with two members as register pointers.
*/
gcrc_t ReadRegClust(U8 clust) {
  gcrc_t rc = {((clust&0b11110000)>>4), (clust&0b00001111)};
  return rc;
}

// The UNK function is ran when an illegal opcode is encountered
U8 UNK(GC* gc) {
  fprintf(stderr, "\033[31mIllegal\033[0m instruction \033[33m%02X\033[0m\nAt position %08X\n", gc->mem[gc->EPC], gc->EPC);
  old_st_legacy;
  gc_errno = 1;
  return 1;
}

/* Instructions implementation start */
/*
  Instruction structure:
    <opcode8> <imm8> [imm32]
  The imm8 will be in every instruction
  It can be used as a register or an 8-bit immediate (e.g. INT8)
  So even HLT for example will be $0000
*/
// 00           hlt
U8 HLT(GC* gc) {
  gc_errno = gc->mem[gc->EPC+1];
  gc->EPC += 2;
  return 1;
}

// 01           trap
U8 TRAP(GC* gc) {
  old_st_legacy;
  ExecD(gc, 1);
  gc->EPC += 2;
  return 0;
}

// 03           sti
U8 STI(GC* gc) {
  Write32(gc, (((gc->mem[gc->EPC+1]-0x80)*4)+0x2000), gc->reg[ESI]);
  gc->EPC += 2;
  return 0;
}

// 04           iret
U8 IRET(GC* gc) {
  // gc->EPC += 2; is not needed because it overwrites EPC
  gc->PS = StackPop(gc);
  gc->EPC = StackPop(gc);
  return 0;
}

// 05           nop
U8 NOP(GC* gc) {
  gc->EPC += 2;
  return 0;
}

// 08           mul reg imm32
U8 MULri(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] *= Read32(gc, gc->EPC+2);
  gc->EPC += 6;
  return 0;
}

// 10           sub reg imm32
U8 SUBri(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] -= Read32(gc, gc->EPC+2);
  gc->EPC += 6;
  return 0;
}

// 18           sub reg byte[imm32]
U8 SUBrb(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] -= gc->mem[Read32(gc, gc->EPC+2)];
  gc->EPC += 6;
  return 0;
}

// 20           inx reg
U8 INXr(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1]]++;
  gc->EPC += 2;
  return 0;
}

// 28           inx reg
U8 DEXr(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1]]--;
  gc->EPC += 2;
  return 0;
}

// 30           inx #imm32
U8 INXb(GC* gc) {
  U32 addr = Read32(gc, gc->EPC+2);
  gc->mem[addr]++;
  gc->EPC += 6;
  return 0;
}

// 32           dex #imm32
U8 DEXb(GC* gc) {
  U32 addr = Read32(gc, gc->EPC+2);
  gc->mem[addr]--;
  gc->EPC += 6;
  return 0;
}

// 37           cmp rc
U8 CMPrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  I32 val0 = gc->reg[rc.x];
  I32 val1 = gc->reg[rc.y];

  if (!(val0 - val1))    SET_ZF(gc->PS);
  else                   RESET_ZF(gc->PS);
  if ((val0 - val1) < 0) SET_NF(gc->PS);
  else                   RESET_NF(gc->PS);

  gc->EPC += 2;
  return 0;
}

// 38           and rc
U8 ANDrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] &= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// 39           ora rc
U8 ORArc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] |= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// 3A           xor rc
U8 XORrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] ^= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// 40           inx @imm32
U8 INXw(GC* gc) {
  U32 addr = Read32(gc, gc->EPC+2);
  U16 a = ReadWord(gc, addr);
  WriteWord(gc, addr, a+1);
  gc->EPC += 6;
  return 0;
}

// 41           int imm8
U8 INT(GC* gc) {
  if (!IF(gc->PS)) goto intend;
  if (gc->mem[gc->EPC+1] >= 0x80) { // Custom interrupt
    StackPush(gc, gc->EPC+2); // Return address
    StackPush(gc, gc->PS); // Flags
    gc->EPC = Read32(gc, ((gc->mem[gc->EPC+1]-0x80)*4)+0x2000);
    return 0;
  }
  switch (gc->mem[gc->EPC+1]) {
  case INT_EXIT:
    gc_errno = StackPop(gc);
    return 1;
  case INT_READ:
    char a = getchar();
    StackPush(gc, a);
    break;
  case INT_WRITE:
    putchar(StackPop(gc));
    fflush(stdout);
    break;
  case INT_RESET:
    Reset(gc);
    return 0;
  case INT_CANREAD:
      struct pollfd pfds[1];
      pfds[0].fd = fileno(stdin);
      pfds[0].events = POLLIN;
      pfds[0].revents = 0;
      poll(&pfds[0], 1, 0);
      gc->reg[EDX] = pfds[0].revents & POLLIN ? 1 : 0;
      break;
  case INT_VIDEO_FLUSH:
    GGpage(gc);
    break;
  case INT_VIDEO_CLEAR:
    GGflush(gc);
    break;
  case INT_PPU_DRAW:
    GGsprite_256(gc);
    break;
  case INT_PPU_READ:
    GGsprite_read_256(gc);
    break;
  case INT_PPU_DRAWM:
    GGsprite_mono(gc);
    break;
  case INT_RAND:
    gc->reg[EDX] = rand() % 65536;
    break;
  case INT_DATE:
    gc->reg[EDX] = GC_GOVNODATE();
    break;
  case INT_WAIT:
    usleep((U32)(gc->reg[EDX])*1000); // the maximum is about 65.5 seconds
    if(hid_events(gc)) {
      gc_errno = 0;
      return 1;
    }
    break;
  case INT_BEEP:
    double freq = (double)StackPop(gc);
    PlayBeep(freq);
    break;
  default:
    fprintf(stderr, "gc32: \033[91mIllegal\033[0m hardware interrupt: $%02X\n", gc->mem[gc->EPC+1]);
    return 1;
  }
  intend: gc->EPC += 2;
  return 0;
}

// 42           dex @imm32
U8 DEXw(GC* gc) {
  U32 addr = Read32(gc, gc->EPC+2);
  U16 a = ReadWord(gc, addr);
  WriteWord(gc, addr, a-1);
  gc->EPC += 6;
  return 0;
}

// 47           add rc
U8 ADDrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] += gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// 48           add reg imm32
U8 ADDri(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] += Read32(gc, gc->EPC+2);
  gc->EPC += 6;
  return 0;
}

// 50           add reg byte[imm32]
U8 ADDrb(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] += gc->mem[Read32(gc, gc->EPC+2)];
  gc->EPC += 6;
  return 0;
}

// 58           mov reg word[imm32]
U8 ADDrw(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] += ReadWord(gc, Read32(gc, gc->EPC+2));
  gc->EPC += 6;
  return 0;
}

// 60           mov byte[imm32] reg
U8 ADDbr(GC* gc) {
  gc->mem[Read32(gc, gc->EPC+2)] += gc->reg[gc->mem[gc->EPC+1] % 32];
  gc->EPC += 6;
  return 0;
}

// 68           add word[imm32] reg
U8 ADDwr(GC* gc) {
  U16 addr = Read32(gc, gc->EPC+2);
  U16 w = ReadWord(gc, addr);
  WriteWord(gc, addr, w+gc->reg[gc->mem[gc->EPC+1] % 32]);
  gc->EPC += 6;
  return 0;
}

// 70           cmp reg imm32
U8 CMPri(GC* gc) {
  I32 val0 = gc->reg[gc->mem[gc->EPC+1] % 32];
  I32 val1 = Read32(gc, gc->EPC+2);

  if (!(val0 - val1))    SET_ZF(gc->PS);
  else                   RESET_ZF(gc->PS);
  if ((val0 - val1) < 0) SET_NF(gc->PS);
  else                   RESET_NF(gc->PS);

  gc->EPC += 6;
  return 0;
}

// 78           call imm32
U8 CALLa(GC* gc) {
  StackPush(gc, gc->EPC+6);
  // gc->EPC += 2; but PC is overwritten
  gc->EPC = Read32(gc, gc->EPC+2);
  return 0;
}

// 79           ret
U8 RET(GC* gc) {
  // gc->EPC += 2; but PC is overwritten
  gc->EPC = StackPop(gc);
  return 0;
}

// 7A           sal reg imm5
U8 SALrg(GC* gc) {
  gc->reg[(gc->mem[gc->EPC+1]&0b11100000)>>5] <<= gc->mem[gc->EPC+1]&0b00011111;
  gc->EPC += 2;
  return 0;
}

// 7B           sar reg imm5
U8 SARrg(GC* gc) {
  gc->reg[(gc->mem[gc->EPC+1]&0b11100000)>>5] >>= gc->mem[gc->EPC+1]&0b00011111;
  gc->EPC += 2;
  return 0;
}

// 7E           stob rc
U8 STOBc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->mem[gc->reg[rc.x]] = gc->reg[rc.y];
  gc->reg[rc.x]++;
  gc->EPC += 2;
  return 0;
}

// 7F           lodb rc
U8 LODBc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.y] = gc->mem[gc->reg[rc.x]];
  gc->reg[rc.x]++;
  gc->EPC += 2;
  return 0;
}

// 8E           stow rc
U8 STOWc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  WriteWord(gc, gc->reg[rc.x], gc->reg[rc.y]);
  gc->reg[rc.x] += 2;
  gc->EPC += 2;
  return 0;
}

// 8F           lodw rc
U8 LODWc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.y] = ReadWord(gc, gc->reg[rc.x]);
  gc->reg[rc.x] += 2;
  gc->EPC += 2;
  return 0;
}

// 9E           stod rc
U8 STODc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  Write32(gc, gc->reg[rc.x], gc->reg[rc.y]);
  gc->reg[rc.x] += 3;
  gc->EPC += 2;
  return 0;
}

// 9F           lodd rc
U8 LODDc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.y] = Read32(gc, gc->reg[rc.x]);
  gc->reg[rc.x] += 3;
  gc->EPC += 2;
  return 0;
}

// 80           div reg imm32
U8 DIVri(GC* gc) {
  U32 a = Read32(gc, gc->EPC+2);
  U8 r = gc->mem[gc->EPC+1] % 32; // reg
  gc->reg[EDX] = (gc->reg[r] % a);
  gc->reg[r] /= a;
  gc->EPC += 6;
  return 0;
}

// 86           jmp imm32
U8 JMPa(GC* gc) {
  gc->EPC = Read32(gc, gc->EPC+2);
  return 0;
}

// 90           sub reg word[imm32]
U8 SUBrw(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] -= ReadWord(gc, Read32(gc, gc->EPC+2));
  gc->EPC += 6;
  return 0;
}

// A0           je imm32
U8 JEa(GC* gc) {
  if (ZF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
    RESET_ZF(gc->PS);
  }
  else gc->EPC += 6;
  return 0;
}

// A1           jne imm32
U8 JNEa(GC* gc) {
  if (!ZF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else gc->EPC += 6;
  return 0;
}

// A2           jc imm32
U8 JCa(GC* gc) {
  if (CF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
    RESET_CF(gc->PS);
  }
  else gc->EPC += 6;
  return 0;
}

// A3           jnc imm32
U8 JNCa(GC* gc) {
  if (!CF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else gc->EPC += 6;
  return 0;
}

// A4           js imm32
U8 JSa(GC* gc) {
  if (!NF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else gc->EPC += 6;
  return 0;
}

// A5           jn imm32
U8 JNa(GC* gc) {
  if (NF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
    RESET_NF(gc->PS);
  }
  else gc->EPC += 6;
  return 0;
}

// A6           ji imm32
U8 JIa(GC* gc) {
  if (IF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else gc->EPC += 6;
  return 0;
}

// A7           jni imm32
U8 JNIa(GC* gc) {
  if (!IF(gc->PS)) {
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else gc->EPC += 6;
  return 0;
}

// A8           re imm32
U8 RE(GC* gc) {
  if (ZF(gc->PS)) {
    gc->EPC = StackPop(gc);
    RESET_ZF(gc->PS);
  }
  else gc->EPC += 2;
  return 0;
}

// A9           rne imm32
U8 RNE(GC* gc) {
  if (!ZF(gc->PS)) {
    gc->EPC = StackPop(gc);
  }
  else gc->EPC += 2;
  return 0;
}

// AA           rc imm32
U8 RC(GC* gc) {
  if (CF(gc->PS)) {
    gc->EPC = StackPop(gc);
    RESET_CF(gc->PS);
  }
  else gc->EPC += 2;
  return 0;
}

// AB           rnc imm32
U8 RNC(GC* gc) {
  if (!CF(gc->PS)) {
    gc->EPC = StackPop(gc);
  }
  else gc->EPC += 2;
  return 0;
}

// AC           rs imm32
U8 RS(GC* gc) {
  if (!NF(gc->PS)) {
    gc->EPC = StackPop(gc);
  }
  else gc->EPC += 2;
  return 0;
}

// AD           rn imm32
U8 RN(GC* gc) {
  if (NF(gc->PS)) {
    gc->EPC = StackPop(gc);
    RESET_NF(gc->PS);
  }
  else gc->EPC += 2;
  return 0;
}

// AE           ri imm32
U8 RI(GC* gc) {
  if (IF(gc->PS)) {
    gc->EPC = StackPop(gc);
    RESET_IF(gc->PS);
  }
  else gc->EPC += 2;
  return 0;
}

// AF           rni imm32
U8 RNI(GC* gc) {
  if (!IF(gc->PS)) {
    gc->EPC = StackPop(gc);
  }
  else gc->EPC += 2;
  return 0;
}

// B0           push imm32
U8 PUSHi(GC* gc) {
  StackPush(gc, Read32(gc, gc->EPC+2));
  gc->EPC += 6;
  return 0;
}

// B5           push reg
U8 PUSHr(GC* gc) {
  StackPush(gc, gc->reg[gc->mem[gc->EPC+1] % 32]);
  gc->EPC += 2;
  return 0;
}

// B6           pop reg
U8 POPr(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] = StackPop(gc);
  gc->EPC += 2;
  return 0;
}

// B8           loop imm32
U8 LOOPa(GC* gc) {
  if (gc->reg[ECX]) {
    gc->reg[ECX]--;
    gc->EPC = Read32(gc, gc->EPC+2);
  }
  else {
    gc->EPC += 6;
  }
  return 0;
}

// B9           ldds
U8 LDDS(GC* gc) {
  gc->reg[EAX] = gc->rom[gc->reg[ESI]];
  gc->EPC += 2;
  return 0;
}

// BA           lddg
U8 LDDG(GC* gc) {
  gc->reg[EAX] = gc->rom[gc->reg[ESI]];
  gc->EPC += 2;
  return 0;
}

// BB           stds
U8 STDS(GC* gc) {
  gc->rom[gc->reg[ESI]] = gc->reg[gc->mem[gc->EPC+1]];
  gc->EPC += 2;
  return 0;
}

// BC           stdg
U8 STDG(GC* gc) {
  gc->rom[gc->reg[EGI]] = gc->reg[gc->mem[gc->EPC+1] % 32];
  gc->EPC += 2;
  return 0;
}

// BF           pow rc
U8 POWrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] = pow(gc->reg[rc.x], gc->reg[rc.y]);
  gc->EPC += 2;
  return 0;
}

// C0           mov reg imm32
U8 MOVri(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] = Read32(gc, gc->EPC+2);
  gc->EPC += 6;
  return 0;
}

// CF           mov rc
U8 MOVrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] = gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// C8           sub rc
U8 SUBrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] -= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// C9           mul rc
U8 MULrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[rc.x] *= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// CA           div rc
U8 DIVrc(GC* gc) {
  gcrc_t rc = ReadRegClust(gc->mem[gc->EPC+1]);
  gc->reg[EDX] = gc->reg[rc.x] % gc->reg[rc.y]; // Remainder into %dx
  gc->reg[rc.x] /= gc->reg[rc.y];
  gc->EPC += 2;
  return 0;
}

// D0           mov reg byte[imm32]
U8 MOVrb(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] = gc->mem[Read32(gc, gc->EPC+2)];
  gc->EPC += 6;
  return 0;
}

// D4           mov reg dword[imm32]
U8 MOVrd(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] = Read32(gc, Read32(gc, gc->EPC+2));
  gc->EPC += 6;
  return 0;
}

// D8           mov reg word[imm32]
U8 MOVrw(GC* gc) {
  gc->reg[gc->mem[gc->EPC+1] % 32] = ReadWord(gc, Read32(gc, gc->EPC+2));
  gc->EPC += 6;
  return 0;
}

// E0           mov byte[imm32] reg
U8 MOVbr(GC* gc) {
  gc->mem[Read32(gc, gc->EPC+2)] = gc->reg[gc->mem[gc->EPC+1] % 32];
  gc->EPC += 6;
  return 0;
}

// E4           mov dword[imm32] reg
U8 MOVdr(GC* gc) {
  Write32(gc, Read32(gc, gc->EPC+2), gc->reg[gc->mem[gc->EPC+1] % 32]);
  gc->EPC += 6;
  return 0;
}

// E8           mov word[imm32] reg
U8 MOVwr(GC* gc) {
  WriteWord(gc, Read32(gc, gc->EPC+2), gc->reg[gc->mem[gc->EPC+1] % 32]);
  gc->EPC += 6;
  return 0;
}

// F0           rswp
U8 RSWP(GC* gc) {
  U8 regaddr = 0;
  U8 temp;
  while (regaddr < 16) {
    temp = gc->reg[regaddr];
    gc->reg[regaddr] = gc->reg[regaddr+16];
    gc->reg[regaddr+16] = temp;
    regaddr++;
  }
  gc->EPC += 2;
  return 0;
}

/* Instructions implementation end */

U8 PG0F(GC*); // Page 0F - Additional instructions page

// Zero page instructions
U8 (*INSTS[256])() = {
  &HLT  , &TRAP , &UNK  , &STI  , &IRET , &NOP  , &UNK  , &UNK  , &MULri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &SUBri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &SUBrb, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &INXr , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &DEXr , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &INXb , &UNK  , &DEXb , &UNK  , &UNK  , &UNK  , &UNK  , &CMPrc, &ANDrc, &ORArc, &XORrc, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &INXw , &INT  , &DEXw , &UNK  , &UNK  , &UNK  , &UNK  , &ADDrc, &ADDri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &ADDrb, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &ADDrw, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &ADDbr, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &ADDwr, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &CMPri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &CALLa, &RET  , &SALrg, &SARrg, &UNK  , &UNK  , &STOBc, &LODBc,
  &DIVri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &JMPa , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &STOWc, &LODWc,
  &SUBrw, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &STODc, &LODDc,
  &JEa  , &JNEa , &JCa  , &JNCa , &JSa  , &JNa  , &JIa  , &JNIa , &RE   , &RNE  , &RC   , &RNC  , &RS   , &RN   , &RI   , &RNI  ,
  &PUSHi, &UNK  , &UNK  , &UNK  , &UNK  , &PUSHr, &POPr , &UNK  , &LOOPa, &LDDS , &LDDG , &STDS , &STDG , &UNK  , &UNK  , &POWrc,
  &MOVri, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &SUBrc, &MULrc, &DIVrc, &UNK  , &UNK  , &UNK  , &UNK  , &MOVrc,
  &MOVrb, &UNK  , &UNK  , &UNK  , &MOVrd, &UNK  , &UNK  , &UNK  , &MOVrw, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &MOVbr, &UNK  , &UNK  , &UNK  , &MOVdr, &UNK  , &UNK  , &UNK  , &MOVwr, &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &RSWP , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &CIF  , &CFI  , &ADDF , &SUBF , &MULF , &DIVF , &NEGF , &UNK
/*INSTS_END*/};

U8 (*INSTS_PG0F[256])() = {
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  ,
  &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK  , &UNK
};

U8 PG0F(GC* gc) {   // 0FH
  gc->EPC++;
  return (INSTS_PG0F[gc->mem[gc->EPC]])(gc);
}

U0 Reset(GC* gc) {
  gc->EPC = 0x00700000;
  // Reset the general purpose registers
  for (U8 i = 0; i < 32; i++) 
    gc->reg[i] = 0x00000000;
  gc->reg[ESP] = 0x00FEFFFF;
  gc->reg[EBP] = 0x00FEFFFF;
  // Adjust ESP+/EBP+ extra registers
  gc->reg[R22] = 0x00FEFFFF;
  gc->reg[R23] = 0x00FEFFFF;

  gc->PS = 0b01000000;
}

U0 PageDump(GC* gc, U8 page) {
  for (U16 i = (page*256); i < (page*256)+256; i++) {
    if (!(i % 16)) putchar(10);
    printf("%02X ", gc->mem[i]);
  }
}

U0 StackDump(GC* gc, U16 c) {
  for (U32 i = 0xFEFFFF; i > 0xFEFFFF-c; i--) {
    if (i != gc->reg[ESP]) printf("%08X: %02X\n", i, gc->mem[i]);
    else                   printf("\033[92m%08X: %02X\033[0m\n", i, gc->mem[i]);
  }
}

U0 MemDump(GC* gc, U32 start, U32 end, U8 newline) {
  for (U32 i = start; i < end; i++) {
    printf("%02X ", gc->mem[i]);
  }
  putchar(8);
  putchar(10*newline);
}

U0 RegDump(GC* gc) {
  printf("pc: %08X;  eax: %08X\n", gc->EPC, gc->reg[EAX]);
  printf("ebx: %08X;     ecx: %08X\n", gc->reg[EBX], gc->reg[ECX]);
  printf("edx: %08X;     esi: %08X\n", gc->reg[EDX], gc->reg[ESI]);
  printf("egi: %08X;     esp: %08X\n", gc->reg[EGI], gc->reg[ESP]);
  printf("ps: %08b; ebp: %08X\n", gc->PS, gc->reg[EBP]);
  printf("   -I---ZNC\033[0m\n");
}

U8 Exec(GC* gc, const U32 memsize, U8 verbosemode) {
  U8 exc = 0;
  U32 insts = 0;
  SDL_ShowCursor(SDL_DISABLE);
  execloop:
    exc = (INSTS[gc->mem[gc->EPC]])(gc);
    insts++;
    // printh(gc->EPC, "\n");
    if (exc != 0) {
      U32 s = ceil(log10(insts))-1;
      printf("gc32: executed %.3lfE%d instructions\n", insts/pow(10, s), s);
      printf(verbosemode ? "last executed instruction: \033[32m$%02X\033[0m\n" : "", gc->mem[gc->EPC]);
      printf(verbosemode ? "last executed address: \033[33m#%08X\033[0m\n" : "", gc->EPC);
      return gc_errno;
    }
    goto execloop;
  return exc;
}
