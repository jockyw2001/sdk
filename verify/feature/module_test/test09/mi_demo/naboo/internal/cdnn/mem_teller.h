#ifndef _MEM_TELLER_H_
#define _MEM_TELLER_H_

#include "mi_common_datatype.h"

typedef  struct
{
    int sysfd;
    int memfd;
} mem_teller;

int mem_teller_create(mem_teller* teller);
int mem_teller_release(mem_teller* teller);
int mem_teller_alloc(mem_teller* teller, MI_PHY *phys_addr, unsigned int size, const char* buf_name);
int mem_teller_free(mem_teller* teller,  MI_PHY phys_addr, const char* buf_name);
int mem_teller_mmap(mem_teller* teller, MI_PHY phys_addr, MI_U8  **virt_addr, unsigned int size);
int mem_teller_unmmap(mem_teller* teller, MI_U8 *addr, unsigned int size);

#endif // _MEM_TELLER_H_