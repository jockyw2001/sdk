#ifndef _SUBBUFALLOCATOR_H_
#define _SUBBUFALLOCATOR_H_

#define VDISP_MAX_BUFPLANE_NUM    3

#define VDISP_SUBBUF_ALLOC_FLAG_FREERUN 1

typedef enum
{
    VIDSP_BUF_TYPE_NORMAL,
    VIDSP_BUF_TYPE_SUB,
    VIDSP_BUF_TYPE_NORMAL_FIRST,
    VIDSP_BUF_TYPE_SUB_FIRST
}subbufallc_buf_type_e;

typedef struct
{
    void *vaddr;
    phys_addr_t paddr;
    unsigned long stride;
    unsigned long (*pixel_offset)(unsigned long x, unsigned long y);
}subbufallc_plane_t;

typedef struct
{
    subbufallc_buf_type_e type;
    struct list_head node;
    struct list_head subbuf_list;
    unsigned long width;
    unsigned long height;
    unsigned long planenum;
    subbufallc_plane_t bufplane[VDISP_MAX_BUFPLANE_NUM];
    MI_U64 u64pts;
    void *priv;
}subbufallc_bufhead_t;

typedef struct
{
    subbufallc_buf_type_e type;
    struct list_head node;
    subbufallc_bufhead_t *buf;
    struct list_head same_subbuf;
    subbufallc_subbufallocinfo_t *suballcinfo;
}subbufallc_subbufhead_t;

typedef struct
{
    struct list_head node;
    subbufallc_bufhead_t *cur_alloc;
    struct list_head samesub_list;
    struct list_head bufout_list;
    int sub_buf_id;
    int enabled;
    unsigned long x;
    unsigned long y;
    unsigned long width;
    unsigned long height;
    int64_t ptsadjust;
    int bunregisted;
    uint64_t alloccounter;
    void *priv;
}subbufallc_subbufallocinfo_t;

typedef struct
{
    struct list_head buf_queue; //subbufallc_bufhead_t queue
    struct list_head subbufinfo_list; //subbufallc_subbufallocinfo_t.samesub_list
    struct list_head subbufinfo_out_list; //subbufallc_subbufallocinfo_t.bufout_list
    int benable;
    int count;
    spinlock_t lock;
}subbufallc_subbuf_queue_t;

#define subbufallc_bufcnt(subbufq) (subbufq->count)
#define subbufallc_subbufx(subbuf) (subbuf->suballcinfo->x)
#define subbufallc_subbufy(subbuf) (subbuf->suballcinfo->y)
#define subbufallc_subbufwidth(subbuf) (subbuf->suballcinfo->width)
#define subbufallc_subbufheight(subbuf) (subbuf->suballcinfo->height)

inline unsigned long subbufallc_subbufvalidwidth(subbufallc_subbufhead_t *subbuf)
{
    unsigned long x0,w0,w1;
    x0=subbufallc_subbufx(subbuf);
    w0=subbufallc_subbufwidth(subbuf);
    w1=subbuf->buf->width;
    if(x0>=w1){
        return 0;
    }else if((x0+w0)>w1){
        return w1-x0;
    }else{
        return w0;
    }
}
inline unsigned long subbufallc_subbufvalidheight(subbufallc_subbufhead_t *subbuf)
{
    unsigned long y0,h0,h1;
    y0=subbufallc_subbufy(subbuf);
    h0=subbufallc_subbufheight(subbuf);
    h1=subbuf->buf->height;
    if(y0>=h1){
        return 0;
    }else if((y0+h0)>h1){
        return h1-y0;
    }else{
        return h0;
    }
}

int subbufallc_init(void);
int subbufallc_deinit(void);

int subbufallc_subbufqinit(subbufallc_subbuf_queue_t *subbufq);
int subbufallc_subbufqdestroy(subbufallc_subbuf_queue_t *subbufq);

subbufallc_subbufhead_t* subbufallc_alloc(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id,
    uint64_t u64pts,
    unsigned long flag);
void subbufallc_free(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_subbufhead_t* subbufhead);
int subbufallc_queuebuf(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_bufhead_t *buf);
subbufallc_bufhead_t *subbufallc_dequeuebuf(
    subbufallc_subbuf_queue_t *subbufq);

int subbufallc_registersubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    unsigned long x, unsigned long y,
    unsigned long width, unsigned long height,
    int old_subbufid, void *priv);

void subbufallc_unregistersubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id);

int subbufallc_enable(
    subbufallc_subbuf_queue_t *subbufq,
    int sub_buf_id, int enable);

subbufallc_bufhead_t *subbufallc_firstbuf(
    subbufallc_subbuf_queue_t *subbufq);
void subbufallc_skipallocfromfirst(
    subbufallc_subbuf_queue_t *subbufq);

subbufallc_subbufhead_t *subbufallc_findsubbuf(
    subbufallc_subbuf_queue_t *subbufq,
    subbufallc_bufhead_t *buf,
    void *allcinfo_priv);
subbufallc_bufhead_t * subbufallc_newbufhead(void);
void subbufallc_freebufhead(subbufallc_bufhead_t * bufhead);

#endif
