/*
 * (C) Copyright 2001
 * derget & Jolt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include "../ppcboot/include/idxfs.h"

#define BUFFER_SIZE 1024

void write_file(FILE *image, char *filename, unsigned char last)
{

  unsigned char buffer[BUFFER_SIZE];
  sIdxFsFatEntry idxfs_fat;
  FILE *idxfs_file;
  unsigned int size;

  idxfs_file = fopen(filename, "rb");

  fseek(idxfs_file, 0, 2);
  size = ftell(idxfs_file);
  fseek(idxfs_file, 0, 0);
  
  idxfs_fat.Size = size;
  
  if (last)
    idxfs_fat.OffsNext = 0;
  else
    idxfs_fat.OffsNext = ftell(image) + sizeof(sIdxFsFatEntry) + size;
  
  fwrite(&idxfs_fat, sizeof(sIdxFsFatEntry), 1, image);
  printf("sizeof(sIdxFsFatEntry): %d\n", sizeof(sIdxFsFatEntry));
  
  while ((size = fread(buffer, 1, BUFFER_SIZE, idxfs_file))) {
  
    printf("Writing 0x%X\n", size);
    fwrite(buffer, size, 1, image);
  
  }
  
  fclose(idxfs_file);

}

int main(void)
{

  FILE *idxfs_img;
  sIdxFsHdr idxfs_hdr;

  idxfs_img = fopen("image.idx", "wb");
  
  idxfs_hdr.Magic = IDXFS_MAGIC;
  idxfs_hdr.Version = IDXFS_VERSION;
  idxfs_hdr.FatOffsFirst = sizeof(sIdxFsHdr);
  
  fwrite(&idxfs_hdr, sizeof(sIdxFsHdr), 1, idxfs_img);
  printf("sizeof(sIdxFsHdr): %d\n", sizeof(sIdxFsHdr));
  
  write_file(idxfs_img, "kernel", 0);
  write_file(idxfs_img, "logo-fb", 1);
  
  fclose(idxfs_img);
  
  return 0;

}

