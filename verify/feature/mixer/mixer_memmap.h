#ifndef _MIXER_MEMMAP_H_
#define _MIXER_MEMMAP_H_

#define BANK_TO_ADDR32(b) (b<<9)
#define REG_ADDR(riu_base,bank,reg_offset) ((riu_base)+BANK_TO_ADDR32(bank)+(reg_offset*4))

typedef struct
{
	unsigned char *virt_addr;
	unsigned char *mmap_base;
	unsigned int mmap_length;
}MmapHandle;
MmapHandle* devMemMMap(unsigned int phys_addr, unsigned int length);
int devMemUmap(MmapHandle* handle);

#endif //_MIXER_MEMMAP_H_