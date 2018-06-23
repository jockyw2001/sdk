#ifndef _VDISPCUSTOMERALLOCATORUSERMAP_H_
#define _VDISPCUSTOMERALLOCATORUSERMAP_H_
void * vdisp_map_usr(phys_addr_t phy, unsigned long len);
int vdisp_unmap_usr(phys_addr_t phy);
#endif