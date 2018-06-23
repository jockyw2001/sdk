#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/hashtable.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/idr.h>
#include <asm/atomic.h>
#include <asm/string.h>
#include <linux/fs.h>
#include <linux/mman.h>
#include <linux/hashtable.h>
#include <linux/anon_inodes.h>
#include <linux/mman.h>
#include <linux/file.h>

#include "mi_device.h"
#include "mi_common_internal.h"
#include "mi_print.h"

#include "mi_sys.h"
#include "mi_sys_internal.h"

#include "mi_vdisp.h"
#include "sub_buf_allocator.h"
#include "mi_vdisp_impl.h"
#include "vdisp_custom_allocator.h"
#include "vdisp_customer_allocator_usermap.h"

typedef void * VDISP_USER_MAP_CTX;
typedef struct vdisp_user_map_record_s
{
  struct hlist_node node;
  phys_addr_t phy;
  void *user_va;
  unsigned long len;
  VDISP_USER_MAP_CTX usr_process_ctx;
  int refcnt;
}vdisp_user_map_record_t;

static DEFINE_MUTEX(usrmaprcd_hashtbl_mtx);
static DEFINE_HASHTABLE(usrmaprcd_hashtbl, 8);
static int _vdisp_allocation_file_mmap(struct file *file, struct vm_area_struct *vma)
{
    vdisp_user_map_record_t *pmaprecd = (vdisp_user_map_record_t *)(file->private_data);
    unsigned long addr = vma->vm_start;
    phys_addr_t phy=mi_sys_Miu2Cpu_BusAddr(pmaprecd->phy);
    int ret;
    ret = remap_pfn_range(vma, addr, __phys_to_pfn(phy), pmaprecd->len,vma->vm_page_prot);
    return ret;
}
static int _vdisp_allocation_file_open (struct inode *node, struct file *filp)
{
    return 0;
}
static int _vdisp_allocation_file_release (struct inode *node, struct file *filp)
{
    return 0;
}
static struct file_operations vdisp_allocation_fops = {
    .owner        = THIS_MODULE,
    .open        =_vdisp_allocation_file_open ,
    .release    = _vdisp_allocation_file_release,
    .mmap        =  _vdisp_allocation_file_mmap,
};


static inline VDISP_USER_MAP_CTX _vdisp_allocation_get_cur_user_map_ctx(void)
{
    return (VDISP_USER_MAP_CTX)current->mm;
}

inline static void _vdisp_add_record(vdisp_user_map_record_t *rcd)
{
    hash_add(usrmaprcd_hashtbl, &rcd->node, rcd->phy);
}
inline static void _vdisp_remove_record(vdisp_user_map_record_t *rcd)
{
    hash_del(&rcd->node);
}
inline static vdisp_user_map_record_t * _vdisp_find_record(phys_addr_t phys)
{
    vdisp_user_map_record_t *rcd=NULL;
    hash_for_each_possible(usrmaprcd_hashtbl, rcd, node, phys){
        if(rcd->phy == phys && rcd->usr_process_ctx == _vdisp_allocation_get_cur_user_map_ctx())
            break;
    }
    return rcd;
}

void * vdisp_map_usr(phys_addr_t phy, unsigned long len)
{
    void *va=NULL;
    vdisp_user_map_record_t *pmaprecd=NULL;
    unsigned long populate;
    struct file *file;
    phys_addr_t oldpa=phy;
    mutex_lock(&usrmaprcd_hashtbl_mtx);
    phy=(phy&PAGE_MASK);
    len=PAGE_ALIGN(oldpa+len-phy);
    pmaprecd=_vdisp_find_record(phy);
    if(pmaprecd){
        va=((char*)(pmaprecd->user_va))+(oldpa-phy);
        pmaprecd->refcnt++;
        goto exit;
    }
    pmaprecd=kmalloc(sizeof(vdisp_user_map_record_t), GFP_KERNEL);
    if(!pmaprecd)
        goto exit;
    pmaprecd->len=len;
    pmaprecd->phy=phy;
    pmaprecd->usr_process_ctx=_vdisp_allocation_get_cur_user_map_ctx();
    file = anon_inode_getfile("_vdisp_usr_map_file", &vdisp_allocation_fops, pmaprecd, O_RDWR);
    if (IS_ERR(file))
        goto free_recd;

    down_write(&current->mm->mmap_sem);
    va = (void *)do_mmap_pgoff(file, 0, len, PROT_READ|PROT_WRITE, MAP_SHARED, 0, &populate);
    up_write(&current->mm->mmap_sem);

    fput(file);
    if(IS_ERR_VALUE((unsigned long)(va))) {
        va = NULL;
        goto free_recd;
    }
    if (populate)
        mm_populate((unsigned long)va, populate);

    pmaprecd->refcnt=1;
    pmaprecd->user_va=va;
    _vdisp_add_record(pmaprecd);
    va=((char*)va)+(oldpa-phy);
    goto exit;
free_recd:
    kfree(pmaprecd);
exit:
    mutex_unlock(&usrmaprcd_hashtbl_mtx);
    return va;
}

int vdisp_unmap_usr(phys_addr_t phy)
{
    int ret=-1;
    vdisp_user_map_record_t *pmaprecd=NULL;
    mutex_lock(&usrmaprcd_hashtbl_mtx);
    phy=(phy&PAGE_MASK);
    pmaprecd=_vdisp_find_record(phy);
    if(!pmaprecd){
        goto exit;
    }
    pmaprecd->refcnt--;
    if(pmaprecd->refcnt>0)
        goto exit;
    down_write(&current->mm->mmap_sem);
    ret=do_munmap(pmaprecd->usr_process_ctx, (unsigned long)pmaprecd->user_va, pmaprecd->len);
    up_write(&current->mm->mmap_sem);
    _vdisp_remove_record(pmaprecd);
    kfree(pmaprecd);
exit:
    mutex_unlock(&usrmaprcd_hashtbl_mtx);
    return ret;
}