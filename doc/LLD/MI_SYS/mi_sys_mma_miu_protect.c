typedef enum
{
    MIU_BLOCK_IDLE = 0,
    MIU_BLOCK_BUSY
}MIU_PROTECT_BLOCK_STATUS;

/* every MIU will have one MIU_ProtectRanges for it */
struct MIU_ProtectRanges
{
    unsigned char miu;                                            // this protect_info is for which MIU
    MIU_PROTECT_BLOCK_STATUS miuBlockStatus[MIU_BLOCK_NUM];        // BLOCK_STATUS: used or available

    unsigned int krange_num;                                    // count of used block
    struct list_head list_head;                                    // list for every protect_info (MIU_ProtectRange)
    struct mutex lock;
};

static struct MIU_ProtectRanges glob_miu_kranges[KERN_CHUNK_NUM]; //record kernel protect ranges on 3 MIUs


/* each protect_info(block) of a MIU */
typedef struct
{
    unsigned char miuBlockIndex;                                // this protect_info is using which block
    unsigned long start_pa;
    unsigned long length;
    struct list_head list_node;
} MIU_ProtectRange;


static inline void phy_to_MiuOffset(unsigned long phy_addr, unsigned int *miu, unsigned long *offset)
{
    *miu = INVALID_MIU;

    if(phy_addr >= ARM_MIU2_BUS_BASE)
    {
        *miu = 2;
        *offset = phy_addr - ARM_MIU2_BUS_BASE;
    }
    else if(phy_addr >= ARM_MIU1_BUS_BASE)
    {
        *miu = 1;
        *offset = phy_addr - ARM_MIU1_BUS_BASE;
    }
    else if(phy_addr >= ARM_MIU0_BUS_BASE)
    {
        *miu = 0;
        *offset = phy_addr - ARM_MIU0_BUS_BASE;
    }
    else
        printk( "\033[35mFunction = %s, Line = %d, Error, Unknown MIU, for phy_addr is 0x%lX\033[m\n", __PRETTY_FUNCTION__, __LINE__, phy_addr);
}

static int g_kprotect_enabled = 1;

static bool _miu_kernel_protect(unsigned char miuBlockIndex, unsigned char *pu8ProtectId,
    unsigned long start, unsigned long end, int flag)
{
    bool ret = true;

    BUG_ON(!g_kernel_protect_client_id);

    if(g_kprotect_enabled)
        ret = MDrv_MIU_Protect(miuBlockIndex, g_kernel_protect_client_id, start, end, flag);
    else
        printk( "ignore kernel protect\n");

    return ret;
}


/*  this API for the case, in same miu, two adjacent lx exist, they can share same miu protect block
  *              LX_MEM                   LX2_MEM
  *  |-----------------| ------------------|
  */
static int _insertKRange(int miu_index, unsigned long lx_addr, unsigned long lx_length)
{
    MIU_ProtectRange *krange = NULL;

    if(!list_empty(&glob_miu_kranges[miu_index].list_head))
    {
        krange = list_entry(glob_miu_kranges[miu_index].list_head.prev, MIU_ProtectRange, list_node);
        if((krange->start_pa + krange->length) == lx_addr)
        {
            _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
                krange->start_pa, krange->start_pa+krange->length, MIU_PROTECT_DISABLE);

            krange->length += lx_length;

            _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
                krange->start_pa, krange->start_pa+krange->length, MIU_PROTECT_ENABLE);
            return 0;
        }
    }
    return -1;
}

static int idleBlockIndx(struct MIU_ProtectRanges * pranges)
{
    int index = 0;

    for(index = 0; index < MIU_BLOCK_NUM; ++index)
    {
        if(pranges->miuBlockStatus[index] == MIU_BLOCK_IDLE)
        {
            return index;
        }
    }

    return -1;
}

/* when alloc from cma heap, call this API to deleteKRange of this allocted buffer */
int deleteKRange(unsigned long long start_pa, unsigned long length)
{
    struct MIU_ProtectRanges  *pranges ;
    MIU_ProtectRange * range,  * r_front = NULL, * r_back= NULL;
    MIU_ProtectRange old;
    unsigned long r_front_len = 0, r_back_len = 0;
    int miuBlockIndex = -1;
    bool find = false, protect_ret = false;
    int ret = MI_SUCCESS;
    
    unsigned long offset = 0;
    int miu_index = 0;  

    if(length == 0)
        return MI_SUCCESS;

    phy_to_MiuOffset(start_pa, &miu_index, &offset);
    pranges = &glob_miu_kranges[miu_index];



    /*
         * kernel protect range( before allocate buffer)
         *
         * |--------------------------------|
         *
         * kernel protect range(buffer location in this range, after buffer allocated)
         *  r_front        allocated buffer    r_back
         *
         * |------|=============|-------|
         *
         * case: r_front = 0; r_back = 0; r_front=r_back=0;
         */
    mutex_lock(&pranges->lock);
    list_for_each_entry(range, &pranges->list_head, list_node)
    {
        if((start_pa >= range->start_pa)
            && ((start_pa+length) <= (range->start_pa+range->length)))
        {
            find = true;
            old.start_pa = range->start_pa;
            old.length = range->length;
            old.miuBlockIndex = range->miuBlockIndex;
            break;
        }
    }

    if(!find)
    {
       ret = MI_ERR_NO_ITEM;
       printk("not find the buffer: start_pa %lx length %lu\n", start_pa, length);
       goto DELETE_KRANGE_DONE;
    }

    r_front_len = start_pa - range->start_pa;
    r_back_len = range->start_pa + range->length - (start_pa + length);

    if((r_front_len != 0) && (r_back_len != 0))
    {
        miuBlockIndex = idleBlockIndx(pranges);
        if(miuBlockIndex < 0)
        {
           ret = MI_ERR_NO_ITEM;
           printk("no idle miu protect block in miu %d\n", (int)miu_index);
           goto DELETE_KRANGE_DONE;
        }

        r_back = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
        if(!r_back)
        {
           ret = MI_ERR_NO_MEM;
           printk( "no memory\n");
           goto DELETE_KRANGE_DONE;
        }

        r_front = range;
        r_front->length = r_front_len;

        r_back->start_pa = start_pa + length;
        r_back->length = r_back_len;
        r_back->miuBlockIndex = miuBlockIndex;
        INIT_LIST_HEAD(&r_back->list_node);
        list_add(&r_back->list_node, &r_front->list_node);
        pranges->krange_num++;
    }
    else if(r_front_len != 0) //and (r_back_len == 0)
    {
        r_front = range;
        r_front->length = r_front_len;
    }
    else if(r_back_len != 0) //and (r_front_len == 0)
    {
        r_back = range;
        r_back->start_pa = start_pa + length;
        r_back->length = r_back_len;
    }
    else //((r_front_len == 0) && (r_back_len == 0))
    {
        list_del(&range->list_node);
        kfree(range);
        pranges->krange_num--;
    }

    protect_ret = _miu_kernel_protect(old.miuBlockIndex, g_kernel_protect_client_id, old.start_pa,
        old.start_pa + old.length, MIU_PROTECT_DISABLE);
    BUG_ON(!protect_ret);
    pranges->miuBlockStatus[old.miuBlockIndex] = MIU_BLOCK_IDLE;

    if(r_front)
    {
        protect_ret = _miu_kernel_protect(r_front->miuBlockIndex, g_kernel_protect_client_id,
            r_front->start_pa, r_front->start_pa+r_front->length, MIU_PROTECT_ENABLE);
        BUG_ON(!protect_ret);
        pranges->miuBlockStatus[r_front->miuBlockIndex] = MIU_BLOCK_BUSY;
    }

    if(r_back)
    {
        protect_ret = _miu_kernel_protect(r_back->miuBlockIndex, g_kernel_protect_client_id,
            r_back->start_pa, r_back->start_pa+r_back->length, MIU_PROTECT_ENABLE);
        BUG_ON(!protect_ret);
        pranges->miuBlockStatus[r_back->miuBlockIndex] = MIU_BLOCK_BUSY;
    }

DELETE_KRANGE_DONE:
    mutex_unlock(&pranges->lock);
    return ret;
}
/* when free to cma heap, call this API to add KRange of this allocted buffer */
int addKRange(unsigned long long start_pa, unsigned long length)
{
    struct MIU_ProtectRanges *pranges ;
    MIU_ProtectRange *r_prev = NULL, *r_next= NULL;
    MIU_ProtectRange *range;
    int miuBlockIndex = -1;
    bool protect_ret = false;
    int ret = MI_SUCCESS;
     unsigned long offset = 0;
    int miu_index = 0;  


    if(length == 0)
        return MI_SUCCESS;

     phy_to_MiuOffset(start_pa, &miu_index, &offset);
     pranges = &glob_miu_kranges[miu_index];

    /*
         * kernel protect range (before freed buffer)
         *      r_prev       allocated buffer     r_next
         * |-------------|====================|------------|
         *
         * kernel protect range(freed buffer location in this range)
         *   r_prev   freed buffer    r_next
         * |--------|?-------------?|-------|
         *
    */
    mutex_lock(&pranges->lock);
    list_for_each_entry(range, &pranges->list_head, list_node)    // find this miu all kernel_protect setting(range)
    {
        if((range->start_pa + range->length) <= start_pa)
        {
            //printk("\033[35mFunction = %s, Line = %d, find r_prev form 0x%lX to 0x%lX\033[m\n", __PRETTY_FUNCTION__, __LINE__, range->start_pa, (range->start_pa + range->length));
            r_prev = range;
            continue;    // should be continue, we are going to find a nearest one k_range before this buffer
        }
    }

    if(r_prev)    // find a kernel_protect range before this buffer
    {
        if(!list_is_last(&r_prev->list_node,&pranges->list_head))
        {
            r_next = container_of(r_prev->list_node.next, MIU_ProtectRange, list_node);        // if prev_krange is not the last one, the next one krange will be r_next
            //printk("\033[35mFunction = %s, Line = %d, find r_next form 0x%lX to 0x%lX\033[m\n", __PRETTY_FUNCTION__, __LINE__, r_next->start_pa, (r_next->start_pa + r_next->length));
        }
    }
    else        // no kernel_protect range before this buffer ==> all k_range is behind this buffer
    {
        if(list_empty(&pranges->list_head))
            r_next = NULL;
        else
            r_next = list_first_entry(&pranges->list_head, MIU_ProtectRange, list_node);    // r_next will be first krange
    }

    //till now, find the prev range and next range of buffer freed
    if(r_prev && r_next)
    {
        if(((r_prev->start_pa + r_prev->length) == start_pa)
            && ((start_pa + length) == r_next->start_pa))    // the buffer is just the hole between r_prev and r_next
        {
            // disable r_prev
            protect_ret = _miu_kernel_protect(r_prev->miuBlockIndex, g_kernel_protect_client_id,
                r_prev->start_pa, r_prev->start_pa + r_prev->length, MIU_PROTECT_DISABLE);
            BUG_ON(!protect_ret);

            // disable r_next
            protect_ret = _miu_kernel_protect(r_next->miuBlockIndex, g_kernel_protect_client_id,
                r_next->start_pa, r_next->start_pa + r_next->length, MIU_PROTECT_DISABLE);
            BUG_ON(!protect_ret);
            pranges->miuBlockStatus[r_next->miuBlockIndex] = MIU_BLOCK_IDLE;    // mark a k_range is available

            r_prev->length += (r_next->length + length);                // extend the r_prev length, and protect it
            protect_ret = _miu_kernel_protect(r_prev->miuBlockIndex, g_kernel_protect_client_id,
                r_prev->start_pa, r_prev->start_pa + r_prev->length, MIU_PROTECT_ENABLE);
            BUG_ON(!protect_ret);

            list_del(&r_next->list_node);
            kfree(r_next);
            pranges->krange_num--;

            goto ADD_KRANGE_DONE;
        }
    }

    if(r_prev)
    {
        if((r_prev->start_pa + r_prev->length) == start_pa)
        {
            protect_ret = _miu_kernel_protect(r_prev->miuBlockIndex, g_kernel_protect_client_id,
                r_prev->start_pa, r_prev->start_pa + r_prev->length, MIU_PROTECT_DISABLE);
            BUG_ON(!protect_ret);

            r_prev->length += length;
            protect_ret = _miu_kernel_protect(r_prev->miuBlockIndex, g_kernel_protect_client_id,
                r_prev->start_pa, r_prev->start_pa + r_prev->length, MIU_PROTECT_ENABLE);
            BUG_ON(!protect_ret);

            goto ADD_KRANGE_DONE;
        }
    }

    if(r_next)
    {
        if((start_pa + length) == r_next->start_pa)
        {
            protect_ret = _miu_kernel_protect(r_next->miuBlockIndex, g_kernel_protect_client_id,
                r_next->start_pa, r_next->start_pa + r_next->length, MIU_PROTECT_DISABLE);
            BUG_ON(!protect_ret);

            r_next->start_pa = start_pa;
            r_next->length += length;
            protect_ret = _miu_kernel_protect(r_next->miuBlockIndex, g_kernel_protect_client_id,
                r_next->start_pa, r_next->start_pa + r_next->length, MIU_PROTECT_ENABLE);
            BUG_ON(!protect_ret);

            goto ADD_KRANGE_DONE;
        }
    }

    // use a new k_range for this buffer
    miuBlockIndex = idleBlockIndx(pranges);
    if(miuBlockIndex < 0)
    {
       ret = MI_ERR_NO_ITEM;
       printk("no idle miu protect block in miu %d\n", (int)miu_index);
       goto ADD_KRANGE_DONE;
    }
    //printk( "\033[35mFunction = %s, Line = %d, use a new k_range for this buffer, miu_protect %d for 0x%lX to 0x%lX\033[m\n", __PRETTY_FUNCTION__, __LINE__, miuBlockIndex, start_pa, (start_pa+length));

    range = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
    if(!range)
    {
       ret = MI_ERR_NO_MEM;
       printk( "no memory\n");
       goto ADD_KRANGE_DONE;
    }
    range->start_pa = start_pa;
    range->length = length;
    range->miuBlockIndex = miuBlockIndex;
    INIT_LIST_HEAD(&range->list_node);
    if(r_prev)
        list_add(&range->list_node, &r_prev->list_node);
    else
        list_add(&range->list_node, &pranges->list_head);

    protect_ret = _miu_kernel_protect(range->miuBlockIndex, g_kernel_protect_client_id,
        range->start_pa, range->start_pa + range->length, MIU_PROTECT_ENABLE);
    BUG_ON(!protect_ret);
    pranges->miuBlockStatus[range->miuBlockIndex] = MIU_BLOCK_BUSY;
    pranges->krange_num++;

ADD_KRANGE_DONE:
    mutex_unlock(&pranges->lock);
    return ret;
}


void init_glob_miu_kranges(void)
{
    unsigned long offset = 0;
    MIU_ProtectRange *krange = NULL;
    int i = 0, miu_index = 0;

    for(i = 0; i < KERN_CHUNK_NUM; ++i)
    {
        glob_miu_kranges[i].miu = i;
        memset(glob_miu_kranges[i].miuBlockStatus, 0, sizeof(unsigned char)*MIU_BLOCK_NUM);
        glob_miu_kranges[i].krange_num = 0;
        mutex_init(&glob_miu_kranges[i].lock);
        INIT_LIST_HEAD(&glob_miu_kranges[i].list_head);
    }

    if(lx_mem_size != INVALID_PHY_ADDR)
    {
        phy_to_MiuOffset(PHYS_OFFSET, &miu_index, &offset);

        printk("\033[35mFunction = %s, Line = %d, Insert KProtect for LX @ MIU: %d\033[m\n", __PRETTY_FUNCTION__, __LINE__, miu_index);
        krange = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
        BUG_ON(!krange);
        INIT_LIST_HEAD(&krange->list_node);
        krange->start_pa = PHYS_OFFSET;
        krange->length = lx_mem_size;
        krange->miuBlockIndex = glob_miu_kranges[miu_index].krange_num;    // use miu_index's kernel protect
        // kernel protect block index start with 0

        printk("\033[35mFunction = %s, Line = %d, [INIT] for LX0 kprotect: from 0x%lX to 0x%lX, using block %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, krange->start_pa, krange->start_pa + krange->length, krange->miuBlockIndex);
        _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
            krange->start_pa, krange->start_pa + krange->length, MIU_PROTECT_ENABLE);
        glob_miu_kranges[miu_index].miuBlockStatus[krange->miuBlockIndex] = MIU_BLOCK_BUSY;

        glob_miu_kranges[miu_index].krange_num++;                        // next miu_index's kernel protect id
        list_add_tail(&krange->list_node, &glob_miu_kranges[miu_index].list_head);
    }

    if(lx_mem2_size != INVALID_PHY_ADDR)
    {
        phy_to_MiuOffset(lx_mem2_addr, &miu_index, &offset);

        printk("\033[35mFunction = %s, Line = %d, Insert KProtect for LX2 @ MIU: %d\033[m\n", __PRETTY_FUNCTION__, __LINE__, miu_index);
        if(_insertKRange(miu_index, lx_mem2_addr, lx_mem2_size))    // we first check if LX2 can be combined to an existed protect_block(krange), if not, we add a new protect_block(krange)
        {
            krange = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
            BUG_ON(!krange);
            INIT_LIST_HEAD(&krange->list_node);
            krange->start_pa = lx_mem2_addr;
            krange->length = lx_mem2_size;
            krange->miuBlockIndex = glob_miu_kranges[miu_index].krange_num;
            //kernel protect block index start with 0

            printk("\033[35mFunction = %s, Line = %d, [INIT] for LX2 kprotect: from 0x%lX to 0x%lX, using block %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, krange->start_pa, krange->start_pa + krange->length, krange->miuBlockIndex);
            _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
                krange->start_pa, krange->start_pa + krange->length, MIU_PROTECT_ENABLE);
            glob_miu_kranges[miu_index].miuBlockStatus[krange->miuBlockIndex] = MIU_BLOCK_BUSY;

            glob_miu_kranges[miu_index].krange_num++;
            list_add_tail(&krange->list_node, &glob_miu_kranges[miu_index].list_head);
        }
    }

    if(lx_mem3_size != INVALID_PHY_ADDR)
    {
        phy_to_MiuOffset(lx_mem3_addr, &miu_index, &offset);

        printk("\033[35mFunction = %s, Line = %d, Insert KProtect for LX3 @ MIU: %d\033[m\n", __PRETTY_FUNCTION__, __LINE__, miu_index);
        if(_insertKRange(miu_index, lx_mem3_addr, lx_mem3_size))    // we first check if LX3 can be combined to an existed protect_block(krange), if not, we add a new protect_block(krange)
        {
            krange = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
            BUG_ON(!krange);
            INIT_LIST_HEAD(&krange->list_node);
            krange->start_pa = lx_mem3_addr;
            krange->length = lx_mem3_size;
            krange->miuBlockIndex = glob_miu_kranges[miu_index].krange_num;
            //kernel protect block index start with 0

            printk("\033[35mFunction = %s, Line = %d, [INIT] for LX3 kprotect: from 0x%lX to 0x%lX, using block %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, krange->start_pa, krange->start_pa + krange->length, krange->miuBlockIndex);
            _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
                krange->start_pa, krange->start_pa+krange->length, MIU_PROTECT_ENABLE);
            glob_miu_kranges[miu_index].miuBlockStatus[krange->miuBlockIndex] = MIU_BLOCK_BUSY;

            glob_miu_kranges[miu_index].krange_num++;

            list_add_tail(&krange->list_node, &glob_miu_kranges[miu_index].list_head);
        }
    }

    if(lx_mem4_size != INVALID_PHY_ADDR)
    {
        phy_to_MiuOffset(lx_mem4_addr, &miu_index, &offset);

        printk("\033[35mFunction = %s, Line = %d, Insert KProtect for LX4 @ MIU: %d\033[m\n", __PRETTY_FUNCTION__, __LINE__, miu_index);
        if(_insertKRange(miu_index, lx_mem4_addr, lx_mem4_size))    // we first check if LX4 can be combined to an existed protect_block(krange), if not, we add a new protect_block(krange)
        {
            krange = (MIU_ProtectRange *)kzalloc(sizeof(MIU_ProtectRange), GFP_KERNEL);
            BUG_ON(!krange);
            INIT_LIST_HEAD(&krange->list_node);
            krange->start_pa = lx_mem4_addr;
            krange->length = lx_mem4_size;
            krange->miuBlockIndex = glob_miu_kranges[miu_index].krange_num;
            //kernel protect block index start with 0

            printk("\033[35mFunction = %s, Line = %d, [INIT] for LX4 kprotect: from 0x%lX to 0x%lX, using block %u\033[m\n", __PRETTY_FUNCTION__, __LINE__, krange->start_pa, krange->start_pa + krange->length, krange->miuBlockIndex);
            _miu_kernel_protect(krange->miuBlockIndex, g_kernel_protect_client_id,
                krange->start_pa, krange->start_pa+krange->length, MIU_PROTECT_ENABLE);
            glob_miu_kranges[miu_index].miuBlockStatus[krange->miuBlockIndex] = MIU_BLOCK_BUSY;

            glob_miu_kranges[miu_index].krange_num++;

            list_add_tail(&krange->list_node, &glob_miu_kranges[miu_index].list_head);
        }
    }
}

