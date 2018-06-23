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
}sba_buf_type_e;

typedef struct sba_plane_s
{
    void *vaddr;
    phys_addr_t paddr;
    unsigned long stride;
    unsigned long (*pixel_offset)(struct sba_plane_s *plane, unsigned long x, unsigned long y);
}sba_plane_t;

#define SBA_BUFHEAD_FLAG_SKIP  0x01
typedef struct
{
    sba_buf_type_e type;
    struct list_head node;
    struct list_head subbuf_list;
    unsigned long flags;
    MI_SYS_PixelFormat_e ePixelFormat;
    unsigned long width;
    unsigned long height;
    unsigned long planenum;
    sba_plane_t bufplane[VDISP_MAX_BUFPLANE_NUM];
    uint64_t u64pts; //us
    unsigned long pts_tolerance;
    unsigned long buf_id;
    void *priv;
    unsigned char padding[16];
}sba_bufhead_t;

typedef struct
{
    struct list_head node;
    sba_bufhead_t *cur_alloc;
    struct list_head allcinfo_subbuf_list;
    struct list_head pending_list_node;
    int sub_buf_id;
    int enabled;
    unsigned long x;
    unsigned long y;
    unsigned long width;
    unsigned long height;
    int64_t ptsadjust;
    int bunregisted;
    uint64_t alloccounter;
    int unsync_cnt;
    void *priv;
}sba_subbufallocinfo_t;

typedef struct
{
    sba_buf_type_e type;
    struct list_head node;
    sba_bufhead_t *buf;
    struct list_head allcinfo_subbuf_node;
    sba_subbufallocinfo_t *suballcinfo;
    unsigned char padding[36];
}sba_subbufhead_t;

typedef struct
{
    struct list_head buf_queue;
    struct list_head subbufinfo_list;
    struct list_head subbufinfo_pending_list;
    int benable;
    int count;
    unsigned long next_buf_id;
    struct mutex mtx;
}sba_buf_queue_t;

#define sba_bufcnt(subbufq) ((subbufq)->count)
#define sba_subbufx(subbuf) ((subbuf)->suballcinfo->x)
#define sba_subbufy(subbuf) ((subbuf)->suballcinfo->y)
#define sba_subbufwidth(subbuf) ((subbuf)->suballcinfo->width)
#define sba_subbufheight(subbuf) ((subbuf)->suballcinfo->height)

unsigned long sba_subbufvalidwidth(sba_subbufhead_t *subbuf);
unsigned long sba_subbufvalidheight(sba_subbufhead_t *subbuf);

int sba_init(void);
int sba_deinit(void);

int sba_bufqinit(sba_buf_queue_t *subbufq);
int sba_bufqdestroy(sba_buf_queue_t *subbufq);

sba_subbufhead_t* sba_alloc(
    sba_buf_queue_t *subbufq,
    int sub_buf_id,
    uint64_t u64pts,
    unsigned long flag);
void sba_free(
    sba_buf_queue_t *subbufq,
    sba_subbufhead_t* subbufhead);
int sba_queuebuf(
    sba_buf_queue_t *subbufq,
    sba_bufhead_t *buf);
sba_bufhead_t *sba_dequeuebuf(
    sba_buf_queue_t *subbufq);

int sba_registersubbuf(
    sba_buf_queue_t *subbufq,
    unsigned long x, unsigned long y,
    unsigned long width, unsigned long height,
    int old_subbufid, void *priv);

void sba_unregistersubbuf(
    sba_buf_queue_t *subbufq,
    int sub_buf_id);

int sba_enable(
    sba_buf_queue_t *subbufq,
    int sub_buf_id, int enable);

sba_bufhead_t *sba_firstbuf(
    sba_buf_queue_t *subbufq);
void sba_skipallocfromfirst(
    sba_buf_queue_t *subbufq);

sba_subbufhead_t *sba_findsubbuf(
    sba_buf_queue_t *subbufq,
    sba_bufhead_t *buf,
    void *allcinfo_priv);
sba_bufhead_t * sba_newbufhead(void);
void sba_freebufhead(sba_bufhead_t * bufhead);
unsigned long sba_getnextbufid(
    sba_buf_queue_t *bufq,
    int subbufid);
#endif
