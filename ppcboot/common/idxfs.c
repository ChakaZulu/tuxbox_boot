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

#ifndef IDXFSTOOLS
#include <ppcboot.h>
#include "mpc8xx.h"
#include "idxfs.h"
#endif

void idxfs_dump_info(unsigned char *mem, unsigned int mem_size)
{

  sIdxFsHdr *hdr;
  sIdxFsFatEntry *fat;
  unsigned int offs;

  if (sizeof(sIdxFsHdr) > mem_size)
    return;    

  hdr = (sIdxFsHdr *)mem;

  if (hdr->Magic != IDXFS_MAGIC)
    return;

  if (hdr->Version != IDXFS_VERSION)
    return;

  if ((!hdr->FatOffsFirst) || (hdr->FatOffsFirst + sizeof(sIdxFsFatEntry) > mem_size))
    return;
    
  offs = hdr->FatOffsFirst;
  fat = (sIdxFsFatEntry *)&mem[offs];

  while (1) {
  
    printf("IdxFs entry: Offset->0x%08X Next->0x%08X Filename->%s\n", offs + sizeof(sIdxFsFatEntry), fat->OffsNext, fat->Name);
    
    if ((!fat->OffsNext) || (fat->OffsNext + sizeof(sIdxFsFatEntry) > mem_size))
      return;
      
    offs = fat->OffsNext;
    fat = (sIdxFsFatEntry *)&mem[offs];  
  
  }
  
}

unsigned int idxfs_file_info(unsigned char *mem, unsigned int mem_size, unsigned char *file_name, unsigned int *file_offset, unsigned int *file_size) 
{

  sIdxFsHdr *hdr = (sIdxFsHdr *)mem;
  sIdxFsFatEntry *fat;
  
  if ((!hdr) || (!file_name))
    return 0;
    
  if (sizeof(sIdxFsHdr) > mem_size)
    return 0;    

  if (hdr->Magic != IDXFS_MAGIC)
    return 0;

  if (hdr->Version != IDXFS_VERSION)
    return 0;

  if ((!hdr->FatOffsFirst) || (hdr->FatOffsFirst + sizeof(sIdxFsFatEntry) > mem_size))
    return 0;
    
  fat = (sIdxFsFatEntry *)&mem[hdr->FatOffsFirst];
    
  while (1) {
  
    if (fat->Name) {
    
      if (file_offset)
        *file_offset = (unsigned int)(fat + sizeof(sIdxFsFatEntry));

      if (file_size)
        *file_size = fat->Size;
	
      return 1;	

    }  
    
    if ((!fat->OffsNext) || (fat->OffsNext + sizeof(sIdxFsFatEntry) > mem_size))
      return 0;
      
    fat = (sIdxFsFatEntry *)&mem[fat->OffsNext];  
  
  }
  
  return 0;
  
}

