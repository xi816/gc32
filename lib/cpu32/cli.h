// The CLI for using the emulator
#include <cpu32/gpuh.h>
#include <cpu32/cpu32h.h>
#define GC24_VERSION "0.0.1"
#define EXEC_START 1

typedef struct GC32 GC;
char* regf[32] = {
  "EAX","EBX","ECX","EDX","ESI","EGI","ESP","EBP",
  "E8 ","E9 ","E10","E11","E12","E13","E14","E15",
  "E16","E17","E18","E19","E20","E21","E22","E23",
  "E24","E25","E26","E27","E28","E29","E30","E31"
};

U8 cli_DisplayReg(GC* gc) {
  for (U8 i = 0; i < 32; i++) {
    if ((i != 0) && !(i%4)) putchar(10);
    printf("%s:\033[93m$%08X\033[0m  ", regf[i], gc->reg[i]);
  }
  printf("\nPS:\033[93m%08b\033[0m ", gc->PS);
  printf("   EPC:\033[93m$%08X\033[0m  ", gc->EPC);
  printf("\n   -I---ZNC\n");
}

U8 cli_DisplayStack(GC* gc, U16 N) {
  for (U8 i = 0; i < N; i++) {
    printf("\033[93m%08X\033[0m\t\033[96m$%08X\033[0m\n", 0xFEFFFF-(i*3),
      ((gc->mem[0xFEFFFF-(i*3-2)]) + (gc->mem[0xFEFFFF-(i*3-1)] << 8) + (gc->mem[0xFEFFFF-(i*3)] << 16)));
  }
}

U8 putmc(U8 c) {
  switch (c) {
  case 0x00 ... 0x1F:
    putchar('.');
    break;
  case 0x20 ... 0x7E:
    putchar(c);
    break;
  case 0x80 ... 0xFF:
    putchar('~');
    break;
  }
  return 0;
}

U8 cli_DisplayMem(GC* gc, U16 page) {
  fputs("\033[A", stdout);
  for (U32 i = page*256; i < page*256+256; i++) {
    if (!(i % 16)) {
      printf("\n%08X  ", i);
    }
    printf("%02X ", gc->mem[i]);
  }
  putchar(10);
  return 0;
}

U8 cli_DisplayMemX(GC* gc, U16 page) {
  fputs("\033[A", stdout);
  for (U32 i = page*256; i < page*256+256; i++) {
    if (!(i % 16)) {
      printf("\n%08X  ", i);
    }
    putmc(gc->mem[i]);
  }
  putchar(10);
  return 0;
}

U8 cli_InsertMem(GC* gc, U16 addr, U8 byte) {
  gc->mem[addr] = byte;
  return 0;
}

U8 ExecD(GC* gc, U8 trapped) {
  char* inpt;
  char* tokens[10];
  size_t bufsize = 25;
  uint8_t j = 0;
  char* buf = (char*)malloc(bufsize);

  if (trapped) printf("\n\033[91mtrapped\033[0m at PC$%08X\n", gc->EPC);
  else printf("gc32 emu %s\n", GC24_VERSION);

  execloop:
  fputs(": ", stdout);
  getline(&buf, &bufsize, stdin);
  j = 0;
  tokens[j] = strtok(buf, " ");
  while (tokens[j] != NULL) {
    tokens[++j] = strtok(NULL, " ");
  }
  switch ((*tokens)[0]) {
  case 'q':
    free(buf);
    if (trapped) exit(0);
    return 0;
  case 'R':
    free(buf);
    return EXEC_START;
  case 'r':
    cli_DisplayReg(gc);
    break;
  case 'c':
    fputs("\033[H\033[2J", stdout);
    break;
  case 'm':
    if (j == 2)
      cli_DisplayMem(gc, strtol(tokens[1], NULL, 16));
    break;
  case 'M':
    if (j == 2)
      cli_DisplayMemX(gc, strtol(tokens[1], NULL, 16));
    break;
  case 'i':
    if (j == 3)
      cli_InsertMem(gc, strtol(tokens[1], NULL, 16), strtol(tokens[2], NULL, 16));
    break;
  case 's':
    if (j == 2)
      cli_DisplayStack(gc, strtol(tokens[1], NULL, 16));
    break;
  case 'h':
    puts("gc32 cli help:");
    puts("  c       Clear the screen");
    puts("  h       Show help");
    puts("  m <00>  Dump memory");
    puts("  r       Dump registers");
    puts("  R       Run the program");
    puts("  s       Dump the stack");
    puts("  q       Quit");
    break;
  default:
    puts("unknown command");
  }
  goto execloop;
  return 0;
}
