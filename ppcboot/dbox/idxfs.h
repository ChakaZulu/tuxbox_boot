#ifndef IDXFS_H
#define IDXFS_H

#define IDXFS_MAGIC 0x19780526
#define IDXFS_VERSION 0x0101
#define IDXFS_MAX_NAME_LEN 32

typedef struct {

  unsigned int Magic;
  unsigned short Version;
  unsigned int FatOffsFirst;

} sIdxFsHdr;

typedef struct {

  unsigned int Size;
  unsigned char Name[IDXFS_MAX_NAME_LEN];
  unsigned int OffsNext;

} sIdxFsFatEntry;

#endif