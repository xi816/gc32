#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <holyc-types.h>

#define FULL_FS_END 0x800000

// The header is the first 32 bytes of the disk
U8 readHeader(U8* disk) {
  puts("Disk info:");
  puts("  Filesystem:\tGovnFS 2.0");
  printf("  Serial:\t%02X%02X%02X%02X\n", disk[0x0C], disk[0x0D], disk[0x0E], disk[0x0F]);
  printf("  Disk letter:\t%c/\n", disk[0x10]);
  return 0;
}

U16 firstFileSector(U8* disk, I8* filename, I8* tag) {
  // Compile the filename and tag into a GovnFS 2.0 header
  I8 combined[16];
  strcpy(combined, filename);
  memcpy(combined+13, tag, 3);

  U8 sector = 0x0001;
  while (disk[sector*512] != 0xF7) {
    if ((disk[sector*512] == 0x01) && (memcmp(disk+sector*512, combined, 16))) {
      printf("File #/%s/%s found\n", filename, tag);
      return sector;
    }
    sector++;
  }
  printf("File #/%s/%s was not found\n", filename, tag);
  return 0;
}

U16 firstEmptySector(U8* disk, U8* lastByte) {
  U16 sector = 0x0001;
  while ((disk[sector*512] != 0x00) && (disk[sector*512] != 0xF7)) sector++;
  *lastByte = disk[sector*512];
  return sector;
}

U16 nextLink(U8* disk, U16 sector) {
  return (((disk[sector*512+0xFF])<<8) + (disk[sector*512+0xFE]));
}

U8 getFileSize(U8* disk, I8* filename, I8* tag) {
  U16 fs = firstFileSector(disk, filename, tag);
  U32 filesize = 494;
  while (fs) {
    fs = nextLink(disk, fs); 
    filesize += 494;
  }
  return filesize;
}

U16 getLastOccupiedSector(U8* disk) {
  U32 addr = FULL_FS_END;
  while (addr >= 0x000200) {
    if (disk[addr]) {
      return addr/0x200;
    }
    addr -= 0x200;
  }
  return addr;
}

U8 writeFile(U8* disk, I8* filename, I8* g_filename, I8* g_tag) {
  FILE* fl = fopen(filename, "rb");
  if (fl == NULL) {
      printf("ugovnfs: \033[91mfatal error:\033[0m file `%s` not found\n", filename);
      return 1;
  }

  fseek(fl, 0, SEEK_END);
  U32 flsize = ftell(fl);
  fseek(fl, 0, SEEK_SET);

  U16 slos = getLastOccupiedSector(disk);
  U32 numSectors = (flsize + 493) / 494; // 494 bytes per sector
  printf("File size: \033[93m%03u\033[0m B, \033[93m%u\033[0m S\t", flsize, numSectors);

  U8 lastByte;
  U16 fes = firstEmptySector(disk, &lastByte);

  // Sector 0 of a file
  disk[fes*512] = 0x01;
  strcpy((I8*)(disk+fes*512+1), g_filename);
  memcpy(disk+fes*512+13, g_tag, 3);

  U32 bytesWritten = 0;
  U16 currentSector = fes;
  U8 buffer[494];

  while (bytesWritten < flsize) {
    size_t toRead = (flsize - bytesWritten > 494) ? 494 : (flsize - bytesWritten);
    fread(buffer, 1, toRead, fl);

    // Link
    if (bytesWritten > 0) {
      disk[currentSector*512 + 0x1FF] = (currentSector + 1) >> 8;
      disk[currentSector*512 + 0x1FE] = (currentSector + 1) & 0xFF;
      currentSector = firstEmptySector(disk, &lastByte);
    }

    if (bytesWritten == 0) {
      memcpy(disk + currentSector * 512 + 16, buffer, toRead);
    }
    else {
      disk[currentSector * 512] = 0x02;
      memset(disk + currentSector * 512 + 1, 0, 15);
      memcpy(disk + currentSector * 512 + 16, buffer, toRead);
    }
    bytesWritten += 494;
  }

  disk[currentSector * 512 + 0x1FF] = 0x00;
  disk[currentSector * 512 + 0x1FE] = 0x00;
  U16 nlos = getLastOccupiedSector(disk)+1;
  if (nlos >= slos) { // Fuck we erased the signature
    disk[nlos*512] = 0xF7;
  }

  return 0;
}

// The header is the first 32 bytes of the disk
U8 readFilenames(U8* disk, U8 c) {
  printf("Listing %c/\n", c);
  U16 sector = 0x0001; // Sector 0 is header data
  while (disk[sector*512] != 0xF7) {
    if (disk[sector*512] == 0x01) {
      printf("  %.11s\t%.3s\n", &(disk[sector*512+1]), &(disk[sector*512+13]));
    }
    sector++;
  }
  return 0;
}

// CLI tool to make GovnFS partitions
I32 main(I32 argc, I8** argv) {
  if (argc == 1) {
    puts("ugovnfs: no arguments given");
    return 1;
  }
  if (argc == 2) {
    puts("ugovnfs: no disk/flag given");
    return 1;
  }
  FILE* fl = fopen(argv[2], "rb+");
  if (fl == NULL) {
    printf("ugovnfs: \033[91mfatal error:\033[0m file `%s` not found\n", argv[1]);
    return 1;
  }
  fseek(fl, 0, SEEK_END);
  U32 flsize = ftell(fl);
  U8* disk = malloc(flsize);
  fseek(fl, 0, SEEK_SET);
  fread(disk, 1, flsize, fl);
  fseek(fl, 0, SEEK_SET);

  // Check the disk
  if (disk[0x000000] != 0x42) {
    printf("ugovnfs: \033[91mdisk corrupted:\033[0m unknown disk header magic byte `$%02X`\n", disk[0x000000]);
    free(disk);
    return 1;
  }

  U8 ugovnfs_errno = 0xFF;
  if (!strcmp(argv[1], "-i")) {
    ugovnfs_errno = readHeader(disk);
  }
  else if (!strcmp(argv[1], "-l")) {
    ugovnfs_errno = readFilenames(disk, disk[0x10]);
  }
  else if (!strcmp(argv[1], "-c")) {
    // ugovnfs -c          disk.img file.bin "file"   "com"
    ugovnfs_errno = writeFile(disk, argv[3], argv[4], argv[5]);
    fwrite(disk, 1, flsize, fl);
    puts("\033[92msuccess\033[0m");
  }
  else if (!strcmp(argv[1], "-s")) {
    U16 fs = firstFileSector(disk, argv[3], argv[4]);
    printf("The file #/%s/%s starts at #%06X\n", argv[3], argv[4], fs*512);
    ugovnfs_errno = 0;
  }
  else {
    printf("ugovnfs: \033[91mfatal error:\033[0m unknown argument: `%s`\n", argv[1]);
    ugovnfs_errno = 1;
  }
  fclose(fl);
  free(disk);
  if (ugovnfs_errno == 0xFF) {
    puts("ugovnfs: no arguments given");
    return 0;
  }
  return 0;
}
