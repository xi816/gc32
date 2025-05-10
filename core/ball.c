#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ptrlen(t) (sizeof(t)/sizeof(t[0]))

int32_t main(int argc, char** argv) {
  char* color = "\033[32m";
  char* rcolor = "\033[0m";
  if (strcmp(getenv("TERM"), "xterm-256color")) {
    color = "\0";
    rcolor = "\0";
  }
  char* targets[]  = {"gc32", "gboot", "mkfs.govnfs", "ugovnfs", "prepare-disk"};
  char* build_commands[] = {
    "gcc -Wall --std=gnu99 core/main.c -Ilib/ -lm -lSDL2 -o gc32",
    "gcc -Wall --std=gnu99 core/gboot/main.c -o gboot",
    "gcc -Wall --std=gnu99 core/mkfs.govnfs/main.c -o mkfs.govnfs",
    "gcc -Wall --std=gnu99 core/ugovnfs/main.c -Ilib/ -lm -o ugovnfs",
    "gcc -Wall --std=gnu99 core/prepare-disk.c -o prepare-disk"
  };
  char* clean_commands[] = {
    "rm -f gc32",
    "rm -f gboot",
    "rm -f mkfs.govnfs",
    "rm -f ugovnfs",
    "rm -f prepare-disk",
  };

  if (argc == 1) {
    printf("rebuilding %sball%s\n", color, rcolor);
    system("gcc core/ball.c -o ball");
    for (uint16_t i = 0; i < ptrlen(targets); i++) {
      printf("building %s%s%s\n", color, targets[i], rcolor);
      fflush(stdout);
      system(build_commands[i]);
    }
    return 0;
  }
  else if ((argc == 2) && (!strcmp(argv[1], "clean"))) {
    for (uint16_t i = 0; i < ptrlen(targets); i++) {
      printf("removing %s%s%s\n", color, targets[i], rcolor);
      system(clean_commands[i]);
    }
    puts("removing .bin files...");
    system("find . -name \"*.bin\" | xargs rm -f");
    puts("removing .exp files...");
    system("find . -name \"*.exp\" | xargs rm -f");
    puts("done!");
    return 0;
  }
  else {
    printf("ball: unknown argument `%s`\n", argv[1]);
  }
  return 0;
}
