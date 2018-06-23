#ifndef _VDISP_CUSTOM_ALLCATOR_H_
#define _VDISP_CUSTOM_ALLCATOR_H_

#define VDISP_CUSALLOCATOR_DBG 0

#define VDISP_CUSALLOCATION_MAGIC 0x56445000

typedef enum
{
    VDISP_PORT_INPUT,
    VDISP_PORT_OUTPUT
}vdisp_port_type_e;

typedef struct
{
    mi_sys_Allocator_t sys_allocator;
    vdisp_port_type_e porttype;
    MI_SYS_ChnPort_t *port; //point to vdisp_outputport_t.outputport or vdisp_inputport_t.inputport
    struct mutex mtx;
    int breleased;
    int allocation_cnt;
#if VDISP_CUSALLOCATOR_DBG
    struct list_head dbg_node;
    struct list_head dbg_allocation_list;
#endif
}vdisp_allocator_t;

typedef struct
{
    struct hlist_node node;
    MI_U32 magic;
    MI_SYS_BufferAllocation_t sys_allocation;
    vdisp_allocator_t *allocator;
    sba_subbufhead_t* subbufhd;
    phys_addr_t phys[3];
#if VDISP_CUSALLOCATOR_DBG
    struct list_head dbg_node;
#endif
    unsigned char padding[16];
}vdisp_allocation_t;

static inline int vdisp_IsCorrectAllocation(vdisp_allocation_t * allocation)
{
    return allocation->magic==VDISP_CUSALLOCATION_MAGIC;
}
int vdisp_allcator_init(void);
void vdisp_allcator_deinit(void);
vdisp_allocator_t* vdisp_allcator_create(vdisp_port_type_e porttype,
    MI_SYS_ChnPort_t *port);
vdisp_allocation_t * vdisp_bufinfo2allocation(MI_SYS_BufInfo_t *bufinfo);

#if VDISP_CUSALLOCATOR_DBG
void vdisp_dbg_add_allocation(vdisp_allocation_t *allocation);
void vdisp_dbg_rmv_allocation(vdisp_allocation_t *allocation);
void vdisp_dbg_add_allocator(vdisp_allocator_t* allocator);
void vdisp_dbg_rmv_allocator(vdisp_allocator_t* allocator);
void vdisp_dbg_dump_allocators(void);
#endif

#endif
