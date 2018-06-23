#include "hal_mhe_def.h"
#include "hal_mhe_reg.h"
#include "hal_mhe_ios.h"

//MODULE_LICENSE("GPL");

//#define   REG_CKG_BASE            0x1F207000
//#define   REG_CKG_MHE_BASE        (REG_CKG_BASE+0x69*4)

#define  GEN_REG_CMD(base, idx, val, buf, buf_cnt, buf_max) (*(u64*)(cmd_buf+((*cmd_len<(buf_max-1))?(*cmd_len)++:(*cmd_len))*8) = ((u64)(((u32)base+4*idx) & 0x00FFFFFF) >> 1) | ((u64)val & 0x0000FFFF) << 32)

static int _SetBank(mhve_ios*, mhve_reg*);
static int _EncFire(mhve_ios*, mhve_job*);
static int _EncFireGenCmd(mhve_ios*, mhve_job*, void* cmd_buf, int* cmd_len);
static int _EncPoll(mhve_ios*);
static int _IsrFunc(mhve_ios*, int);
static int _IrqMask(mhve_ios*, int);
static void _Release(void* p);

static int _busywait(int count)
{
    volatile int i = count;
    while(--i > 0) ;
    return 0;
}

static void _SetOutbsThdSize(void* base,mhe_reg* regs,int size, int enb)
{
    return; //not ready

    //size = 100;

    regs->hev_bank1.reg_hev_ec_bsp_thd_size_low = size & 0xFFFF;
    regs->hev_bank1.reg_hev_ec_bsp_thd_size_high = (size >> 16) & 0x1FFF;
    regs->hev_bank1.reg_hev_ec_bsp_thd_en = enb;

    REGWR(base, 0x6b, regs->hev_bank1.reg6b, "Bitstream size threshold enable");
    REGWR(base, 0x6a, regs->hev_bank1.reg6a, "Bitstream size threshold (Byte)");
}

static void _SetOutbsThdSizeGenCmd(void* base,mhe_reg* regs,int size, int enb, void* cmd_buf, int* cmd_len, int cmd_len_max)
{
    return; //not ready

    //size = 100;

    regs->hev_bank1.reg_hev_ec_bsp_thd_size_low = size & 0xFFFF;
    regs->hev_bank1.reg_hev_ec_bsp_thd_size_high = (size >> 16) & 0x1FFF;
    regs->hev_bank1.reg_hev_ec_bsp_thd_en = enb;

    GEN_REG_CMD(base, 0x6b, regs->hev_bank1.reg6b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x6a, regs->hev_bank1.reg6a, cmd_buf, cmd_len, cmd_len_max);
}

//------------------------------------------------------------------------------
//  Function    : MheIosAcquire
//  Description : Allocate mhe_ios object and link its member function.
//------------------------------------------------------------------------------
mhve_ios* MheIosAcquire(char* tags)
{
    mhe_ios* asic = MEM_ALLC(sizeof(mhe_ios));

    if(asic)
    {
        mhve_ios* mios = &asic->mios;

        mios->release       = _Release;
        mios->set_bank      = _SetBank;
        mios->enc_fire      = _EncFire;
        mios->gen_enc_cmd   = _EncFireGenCmd;
        mios->enc_poll      = _EncPoll;
        mios->isr_func      = _IsrFunc;
        mios->irq_mask      = _IrqMask;
    }
    return &asic->mios;

    return NULL;
}

//EXPORT_SYMBOL(MheIosAcquire);

//------------------------------------------------------------------------------
//  Function    : _Release
//  Description : Release this object.
//------------------------------------------------------------------------------
static void _Release(void* p)
{
    if(p) MEM_FREE(p);
}

//------------------------------------------------------------------------------
//  Function    : _SetBank
//  Description : Set register bank.
//------------------------------------------------------------------------------
static int _SetBank(mhve_ios* ios, mhve_reg* reg)
{
    int err = 0;
    int i;

    mhe_ios* asic = (mhe_ios*)ios;

    switch(reg->i_id)
    {
        case 0:
            for(i = 0 ; i < MHEIOS_MAXBASE_NUM; i++)
                asic->p_base[i] = reg->base[i];
            break;
        default:
            err = -1;
            break;
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : ClrIrq
//  Description :
//------------------------------------------------------------------------------
static void ClrIrq(void* base, uint bits)
{
    REGWR(base, 0x1d, bits, "clr irq");
}

//------------------------------------------------------------------------------
//  Function    : MskIrq
//  Description :
//------------------------------------------------------------------------------
static void MskIrq(void* base, uint bits)
{
    REGWR(base, 0x1c, bits, "msk irq");
}

//------------------------------------------------------------------------------
//  Function    : BitIrq
//  Description :
//------------------------------------------------------------------------------
static uint BitIrq(void* base)
{
    return 0xFF & REGRD(base, 0x1e, "bit irq");
}

//------------------------------------------------------------------------------
//  Function    : SetOutbsAddr
//  Description : Set output bitstream buffer.
//------------------------------------------------------------------------------
static void SetOutbsAddr(void*       base,
                         mhe_reg*   regs,
                         uint32      addr,
                         int         size)
{
    uint32  value, end_value;

    // Set end address
    value = addr;
    end_value = value + size - 1;

    regs->hev_bank0.reg_hev_ec_bspobuf_adr_low = (uint16)(end_value >> 4);   //(16 byte unit)
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_high = (uint16)(end_value >> 20); //16+4
    regs->hev_bank0.reg_hev_ec_bspobuf_id = 1;  //Means end address.
    REGWR(base, 0x31, regs->hev_bank0.reg31, "bsp obuf address low");
    REGWR(base, 0x32, regs->hev_bank0.reg32, "bsp obuf address high");

    regs->hev_bank0.reg_w1c_hev_ec_bspobuf_set_adr = 1;
    regs->hev_bank0.reg_hev_ec_bspobuf_fifo_th = 7;
    REGWR(base, 0x30, regs->hev_bank0.reg30, "set bsp obuf end address");

    // Set start address
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_low = (uint16)(value >> 4);  //(16 byte unit)
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_high = (uint16)(value >> 20); //16+4
    regs->hev_bank0.reg_hev_ec_bspobuf_id = 0;  //Means start address.
    REGWR(base, 0x31, regs->hev_bank0.reg31, "bsp obuf address low");
    REGWR(base, 0x32, regs->hev_bank0.reg32, "bsp obuf address high");

    regs->hev_bank0.reg_w1c_hev_ec_bspobuf_set_adr = 1;
    regs->hev_bank0.reg_hev_ec_bspobuf_fifo_th = 7;
    REGWR(base, 0x30, regs->hev_bank0.reg30, "set bsp obuf start address");

}

//------------------------------------------------------------------------------
//  Function    : PutCodedBits
//  Description : Put VPS/SPS/PPS/User data/Slice header to FDC.
//------------------------------------------------------------------------------
static void PutCodedBits(void*       base,
                         mhe_reg*   regs,
                         uchar*      code,
                         int         size)
{


    // Calculate total round.
    int round = (size + 15) >> 4;
    ushort word = 0;

    regs->hev_bank0.reg49 = 0;
    regs->hev_bank0.reg4a = 0;
    // Reset reg4b fdc, keep mdc settings.
    regs->hev_bank0.reg_ro_hev_ec_fdc_ack = 0;
    regs->hev_bank0.reg_w1c_hev_ec_fdc_done_clr = 0;
    regs->hev_bank0.reg_ro_hev_ec_fdc_done = 0;
    regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0;

    if(size == 0)
    {
        regs->hev_bank0.reg_hev_ec_fdc_bs_count = 0;
        REGWR(base, 0x4a, regs->hev_bank0.reg4a, "fdc round count");
        regs->hev_bank0.reg_hev_ec_fdc_bs = 0;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = 0;
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        REGWR(base, 0x49, regs->hev_bank0.reg49, "fdc bs ");
        REGWR(base, 0x4a, regs->hev_bank0.reg4a, "fdc len");
        REGWR(base, 0x4b, regs->hev_bank0.reg4b, "fdc vld");
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
        return;
    }

    // Fill the total round regs
    regs->hev_bank0.reg_hev_ec_fdc_bs_count  = round - 1; // Count from 0
    REGWR(base, 0x4a, regs->hev_bank0.reg4a, "fdc round count");

    while(size > 16)
    {
        word = (code[0] << 8) | code[1];
        code += 2;
        size -= 16;

        regs->hev_bank0.reg_hev_ec_fdc_bs = word;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = 15; // This is wrong. But HW don't care; HW uses this value only when last FDC round.
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        REGWR(base, 0x49, regs->hev_bank0.reg49, "fdc bs ");
        REGWR(base, 0x4a, regs->hev_bank0.reg4a, "fdc len");
        REGWR(base, 0x4b, regs->hev_bank0.reg4b, "fdc vld");
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
    }

    if(size > 0)
    {
        word = code[0] << 8;
        if(size > 8)
            word += code[1];

        regs->hev_bank0.reg_hev_ec_fdc_bs = word;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = size;
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        REGWR(base, 0x49, regs->hev_bank0.reg49, "fdc bs ");
        REGWR(base, 0x4a, regs->hev_bank0.reg4a, "fdc len");
        REGWR(base, 0x4b, regs->hev_bank0.reg4b, "fdc vld");
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
    }

    // Clear FDC registers
    regs->hev_bank0.reg_hev_ec_fdc_bs_count = 0;
    regs->hev_bank0.reg_hev_ec_fdc_bs = 0;
    regs->hev_bank0.reg_hev_ec_fdc_bs_len = 0;

}



//------------------------------------------------------------------------------
//  Function    : _EncFire
//  Description : Trigger encode.
//------------------------------------------------------------------------------
static int _EncFire(mhve_ios* ios, mhve_job* job)
{
    mhe_ios* asic = (mhe_ios*)ios;
    mhe_reg* regs = (mhe_reg*)job;
    void* hev_base0 =  asic->p_base[0]; //0x171d00
#if defined(MMHE_IMI_BUF_ADDR)
    void* mhe_base0 =  asic->p_base[1]; //0x171e00
#endif
    void* hev_base1 =  asic->p_base[2]; //0x171f00
    void* hev_base2 =  asic->p_base[3]; //0x170900

    asic->p_regs = regs;    //used in _IsrFunc

    /* Enable Clock */
    //REGWR(REG_CKG_MHE_BASE, 0x00, regs->regClk, "Set Clk");

    /* Flush regs to mhe-hw */
    MskIrq(hev_base0, IRQ_MASK);

    // stream reset
    if(!regs->coded_framecnt)
    {
        regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
        regs->hev_bank1.reg_mhe_dual_bs0_rstz = 0;  //reset
        REGWR(hev_base1, 0x54, regs->hev_bank1.reg54, "stream reset start");

        regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
        regs->hev_bank1.reg_mhe_dual_bs0_rstz = 1;  //not reset
        REGWR(hev_base1, 0x54, regs->hev_bank1.reg54, "stream reset done");
    }

    // SW reset
    regs->hev_bank0.reg_hev_soft_rstz = 0;      //reset
    REGWR(hev_base0, 0x00, regs->hev_bank0.reg00, "SW reset 0");
    _busywait(800);

    REGWR(hev_base0, 0x01, regs->hev_bank0.reg01, "global");
    REGWR(hev_base0, 0x02, regs->hev_bank0.reg02, "encoding picture width");
    REGWR(hev_base0, 0x03, regs->hev_bank0.reg03, "encoding picture height");
    REGWR(hev_base0, 0x04, regs->hev_bank0.reg04, "input source picture luma width");
    REGWR(hev_base0, 0x05, regs->hev_bank0.reg05, "input picture chroma width");
    REGWR(hev_base0, 0x06, regs->hev_bank0.reg06, "current y base address (byte) low");
    REGWR(hev_base0, 0x07, regs->hev_bank0.reg07, "current y base address (byte) high");
    REGWR(hev_base0, 0x08, regs->hev_bank0.reg08, "current c base address (byte) low");
    REGWR(hev_base0, 0x09, regs->hev_bank0.reg09, "current c base address (byte) high");


    REGWR(hev_base0, 0x10, regs->hev_bank0.reg10, "encoding output CTB counts -1");
    REGWR(hev_base0, 0x20, regs->hev_bank0.reg20, "ME setting");
    REGWR(hev_base0, 0x21, regs->hev_bank0.reg21, "IME PIPELINE");
    REGWR(hev_base0, 0x22, regs->hev_bank0.reg22, "me search range");

    REGWR(hev_base0, 0x23, regs->hev_bank0.reg23, "me mvx min/max (-32 ~ 31)");
    REGWR(hev_base0, 0x24, regs->hev_bank0.reg24, "me mvy min/max (-32 ~ 31)");

    REGWR(hev_base0, 0x25, regs->hev_bank0.reg25, "FME PIPELINE");
    REGWR(hev_base0, 0x26, regs->hev_bank0.reg26, "MCC cu32/16 merge mode");

    // [QP]
    REGWR(hev_base1, 0x01, regs->hev_bank1.reg01, "constant QP enable");
    regs->hev_bank0.reg_hev_ec_bsp_fdc_skip = 0;
    REGWR(hev_base0, 0x33, regs->hev_bank0.reg33, "QP delta enable");
    REGWR(hev_base0, 0x48, regs->hev_bank0.reg48, "Slice QP");

    // [GN]
    REGWR(hev_base0, 0x35, regs->hev_bank0.reg35, "GN memory ctb32 width");


    // [MVC]
    REGWR(hev_base0, 0x40, regs->hev_bank0.reg40, "Current top POC Value HM low");
    REGWR(hev_base0, 0x41, regs->hev_bank0.reg41, "Current top POC Value HM high");
    REGWR(hev_base0, 0x42, regs->hev_bank0.reg42, "L0 reference frame POC of list 0 low");
    REGWR(hev_base0, 0x43, regs->hev_bank0.reg43, "L0 reference frame POC of list 0 high");
    REGWR(hev_base0, 0x44, regs->hev_bank0.reg44, "L0 reference frame POC of list 1 low");
    REGWR(hev_base0, 0x45, regs->hev_bank0.reg45, "L0 reference frame POC of list 1 high");
    REGWR(hev_base0, 0x46, regs->hev_bank0.reg46, "value");
    REGWR(hev_base0, 0x47, regs->hev_bank0.reg47, "reference frame flag");

    REGWR(hev_base0, 0x4b, regs->hev_bank0.reg4b, "MDC");

    // [GDR] (Multi-Slice)
    REGWR(hev_base0, 0x4c, regs->hev_bank0.reg4c, "GDR enable");
    REGWR(hev_base0, 0x4d, regs->hev_bank0.reg4d, "nal unit type");

    //RDO
    REGWR(hev_base0, 0x50, regs->hev_bank0.reg50, "RDO c1 for intra/inter TU 4x4");
    REGWR(hev_base0, 0x51, regs->hev_bank0.reg51, "RDO c1 for intra/inter TU 8x8");
    REGWR(hev_base0, 0x52, regs->hev_bank0.reg52, "RDO c1 for intra/inter TU 16x16");
    REGWR(hev_base0, 0x53, regs->hev_bank0.reg53, "RDO c1 for intra/inter TU 32x32");
    REGWR(hev_base0, 0x54, regs->hev_bank0.reg54, "RDO c0/beta for intra TU 4x4");
    REGWR(hev_base0, 0x55, regs->hev_bank0.reg55, "RDO c0/beta for intra TU 8x8");
    REGWR(hev_base0, 0x56, regs->hev_bank0.reg56, "RDO c0/beta for intra TU 16x16");
    REGWR(hev_base0, 0x57, regs->hev_bank0.reg57, "RDO c0/beta for intra TU 32x32");
    REGWR(hev_base0, 0x58, regs->hev_bank0.reg58, "RDO c0/beta for inter TU 8x8");
    REGWR(hev_base0, 0x59, regs->hev_bank0.reg59, "RDO c0/beta for inter TU 8x8");
    REGWR(hev_base0, 0x5a, regs->hev_bank0.reg5a, "RDO c0/beta for inter TU 8x8");
    REGWR(hev_base0, 0x5b, regs->hev_bank0.reg5b, "RDO c0/beta for inter TU 8x8");

    //penalty
    REGWR(hev_base0, 0x5c, regs->hev_bank0.reg5c, "penalty on/off");
    REGWR(hev_base0, 0x5d, regs->hev_bank0.reg5d, "penalty of rdcost for cu8 intra");
    REGWR(hev_base0, 0x5e, regs->hev_bank0.reg5e, "penalty of rdcost for cu8 inter");
    REGWR(hev_base0, 0x5f, regs->hev_bank0.reg5f, "penalty of rdcost for c16 intra");
    REGWR(hev_base0, 0x60, regs->hev_bank0.reg60, "penalty of rdcost for cu16 inter mvp");
    REGWR(hev_base0, 0x61, regs->hev_bank0.reg61, "penalty of rdcost for cu16 inter merge");
    REGWR(hev_base0, 0x62, regs->hev_bank0.reg62, "penalty of rdcost for cu16 inter mvp nores");
    REGWR(hev_base0, 0x63, regs->hev_bank0.reg63, "penalty of rdcost for cu16 inter merge nores");
    REGWR(hev_base0, 0x64, regs->hev_bank0.reg64, "penalty of rdcost for c32 intra");
    REGWR(hev_base0, 0x65, regs->hev_bank0.reg65, "penalty of rdcost for cu32 inter merge");
    REGWR(hev_base0, 0x65, regs->hev_bank0.reg66, "penalty of rdcost for cu32 inter merge nores");

    //deblocking
    REGWR(hev_base0, 0x6c, regs->hev_bank0.reg6c, "deblock tc/beta offset");
    REGWR(hev_base0, 0x6d, regs->hev_bank0.reg6d, "deblock idc");

    REGWR(hev_base0, 0x74, regs->hev_bank0.reg74, "");
    REGWR(hev_base0, 0x75, regs->hev_bank0.reg75, "");
    REGWR(hev_base0, 0x76, regs->hev_bank0.reg76, "");
    REGWR(hev_base0, 0x77, regs->hev_bank0.reg77, "");
    REGWR(hev_base0, 0x78, 0, "");
    REGWR(hev_base0, 0x79, regs->hev_bank0.reg79, "");
    REGWR(hev_base0, 0x7a, regs->hev_bank0.reg7a, "");
    REGWR(hev_base0, 0x7b, regs->hev_bank0.reg7b, "");
    REGWR(hev_base0, 0x7c, regs->hev_bank0.reg7c, "");
    REGWR(hev_base0, 0x7d, regs->hev_bank0.reg7d, "");
    REGWR(hev_base0, 0x7e, regs->hev_bank0.reg7e, "");

    //PMBR
    REGWR(hev_base1, 0x00, regs->hev_bank1.reg00, "CTB Allocate Bit & LUT");
    REGWR(hev_base1, 0x01, regs->hev_bank1.reg01, "CTB Allocate Bit & LUT");
    REGWR(hev_base1, 0x02, regs->hev_bank1.reg02, "Second Stage");
    REGWR(hev_base1, 0x03, regs->hev_bank1.reg03, "Second Stage");
    REGWR(hev_base1, 0x04, regs->hev_bank1.reg04, "Texture Weight");
    // Pixel Color Weight
    REGWR(hev_base1, 0x05, regs->hev_bank1.reg05, "Protected color Y Max/Min");
    REGWR(hev_base1, 0x06, regs->hev_bank1.reg06, "Protected color Cb Max/Min");
    REGWR(hev_base1, 0x07, regs->hev_bank1.reg07, "Protected color Cr Max/Min");
    REGWR(hev_base1, 0x08, regs->hev_bank1.reg08, "Protected color CB_PLUS_CR_MIN");
    REGWR(hev_base1, 0x09, regs->hev_bank1.reg09, "Protected color CR offset");
    REGWR(hev_base1, 0x0a, regs->hev_bank1.reg0a, "Protected color Cr MaxProtected color");
    REGWR(hev_base1, 0x0b, regs->hev_bank1.reg0b, "Bit Weight Fusion");
    REGWR(hev_base1, 0x0c, regs->hev_bank1.reg0c, "ROI");

    /* Set buffer address */
    regs->hev_bank0.reg_hev_gn_mem_sadr_low = regs->gn_mem >> 4;
    regs->hev_bank0.reg_hev_gn_mem_sadr_high = regs->gn_mem >> 16;
    REGWR(hev_base0, 0x36, regs->hev_bank0.reg36, "GN addr low");
    REGWR(hev_base0, 0x37, regs->hev_bank0.reg37, "GN addr high");


    REGWR(hev_base1, 0x14, regs->hev_bank1.reg14, "MBR READ MAP low (ROI MAP)");
    REGWR(hev_base1, 0x15, regs->hev_bank1.reg15, "MBR READ MAP high (ROI MAP)");

    REGWR(hev_base1, 0x12, regs->hev_bank1.reg12, "MBR LUT low (LUT read from start address (byte) low)");
    REGWR(hev_base1, 0x13, regs->hev_bank1.reg13, "MBR LUT high (LUT read from start address (byte) high)");

    REGWR(hev_base1, 0x0e, regs->hev_bank1.reg0e, "MBR GN W low (CTB_weight write out start address low)");
    REGWR(hev_base1, 0x0f, regs->hev_bank1.reg0f, "MBR GN W high (CTB_weight write out start address high)");

    REGWR(hev_base1, 0x10, regs->hev_bank1.reg10, "MBR GN R low (CTB_weight read from start address low)");
    REGWR(hev_base1, 0x11, regs->hev_bank1.reg11, "MBR GN R high (CTB_weight read from start address high)");

    if(regs->coded_framecnt)
    {
        regs->hev_bank0.reg_hev_col_r_sadr0_low = regs->col_w_sadr_buf[0] >> 4;
        regs->hev_bank0.reg_hev_col_r_sadr0_high = regs->col_w_sadr_buf[0] >> 16;
    }
    REGWR(hev_base0, 0x3a, regs->hev_bank0.reg3a, "Col-located MV of L0 Read start addr low");
    REGWR(hev_base0, 0x3b, regs->hev_bank0.reg3b, "Col-located MV of L0 Read start addr high");


    regs->hev_bank0.reg_hev_col_w_sadr_low = regs->col_w_sadr_buf[1] >> 4;
    regs->hev_bank0.reg_hev_col_w_sadr_high = regs->col_w_sadr_buf[1] >> 16;
    REGWR(hev_base0, 0x38, regs->hev_bank0.reg38, "Col-located MV of L0 write start addr low");
    REGWR(hev_base0, 0x39, regs->hev_bank0.reg39, "Col-located MV of L0 write start addr high");

    // [PPU]
    regs->hev_bank0.reg_hev_ppu_fb_b_y_base_low = regs->ppu_int_b >> 9;
    regs->hev_bank0.reg_hev_ppu_fb_b_y_base_high = regs->ppu_int_b >> 16;
    REGWR(hev_base0, 0x68, regs->hev_bank0.reg68, "PPU intermedia B data addr low");
    REGWR(hev_base0, 0x69, regs->hev_bank0.reg69, "PPU intermedia B data addr high");

    regs->hev_bank0.reg_hev_ppu_fb_a_y_base_low = regs->ppu_int_a >> 9;
    regs->hev_bank0.reg_hev_ppu_fb_a_y_base_high = regs->ppu_int_a >> 16;
    REGWR(hev_base0, 0x6a, regs->hev_bank0.reg6a, "PPU intermedia A data addr low");
    REGWR(hev_base0, 0x6b, regs->hev_bank0.reg6b, "PPU intermedia A data addr high");

    REGWR(hev_base0, 0x6f, regs->hev_bank0.reg6f, "ppu_frame buffer pitch value");

    //reconstruct/reference buffer
    regs->hev_bank0.reg_hev_ppu_fblut_luma_base_low = regs->ppu_y_base_buf[0] >> 9;
    regs->hev_bank0.reg_hev_ppu_fblut_luma_base_high = regs->ppu_y_base_buf[0] >> 16;
    REGWR(hev_base0, 0x70, regs->hev_bank0.reg70, "reconstructed Y buffer low");
    REGWR(hev_base0, 0x71, regs->hev_bank0.reg71, "reconstructed Y buffer high");
    //CamOsPrintf("rec Y: bank:0x%08X - 0x%08X\n", hev_base0, regs->ppu_y_base_buf[0]);

    regs->hev_bank0.reg_hev_ppu_fblut_chroma_base_low = regs->ppu_c_base_buf[0] >> 9;
    regs->hev_bank0.reg_hev_ppu_fblut_chroma_base_high = regs->ppu_c_base_buf[0] >> 16;
    REGWR(hev_base0, 0x72, regs->hev_bank0.reg72, "reconstructed C buffer low");
    REGWR(hev_base0, 0x73, regs->hev_bank0.reg73, "reconstructed C buffer high");

#if defined(MMHE_IMI_BUF_ADDR)
    regs->hev_bank2.reg_hev_refy_imi_sadr_low = regs->imi_ref_y_buf >> 5;
    regs->hev_bank2.reg_hev_refy_imi_sadr_high = regs->imi_ref_y_buf >> 16;

    REGWR(hev_base2, 0x61, regs->hev_bank2.reg61, "imi reference y low");
    REGWR(hev_base2, 0x62, regs->hev_bank2.reg62, "imi reference y high");

    regs->hev_bank2.reg_hev_mcc_imi_sadr_low = regs->imi_ref_c_buf >> 5;
    regs->hev_bank2.reg_hev_mcc_imi_sadr_high = regs->imi_ref_c_buf >> 16;

    REGWR(hev_base2, 0x63, regs->hev_bank2.reg63, "imi reference c low");
    REGWR(hev_base2, 0x64, regs->hev_bank2.reg64, "imi reference c high");
#endif


    regs->hev_bank0.reg_hev_ref_y_adr_low = regs->ppu_y_base_buf[1] >> 8;
    regs->hev_bank0.reg_hev_ref_y_adr_high = regs->ppu_y_base_buf[1] >> 16;
    REGWR(hev_base0, 0x28, regs->hev_bank0.reg28, "reference Y buffer low");
    REGWR(hev_base0, 0x29, regs->hev_bank0.reg29, "reference Y buffer high");

    //CamOsPrintf("ref Y: bank:0x%08X - 0x%08X\n", hev_base0, regs->ppu_y_base_buf[1]);

    regs->hev_bank0.reg_hev_ref_c_adr_low = regs->ppu_c_base_buf[1] >> 8;
    regs->hev_bank0.reg_hev_ref_c_adr_high = regs->ppu_c_base_buf[1] >> 16;
    REGWR(hev_base0, 0x2a, regs->hev_bank0.reg2a, "reference C buffer low");
    REGWR(hev_base0, 0x2b, regs->hev_bank0.reg2b, "reference C buffer high");

    /* Set output threshold size */
    _SetOutbsThdSize(hev_base1, regs, 10, 1);

    /* Set dump register address */
    REGWR(hev_base2, 0x65, regs->hev_bank2.reg65, "write registers to DRAM start address low");
    REGWR(hev_base2, 0x66, regs->hev_bank2.reg66, "write registers to DRAM start address high");
    REGWR(hev_base2, 0x67, regs->hev_bank2.reg67, "write registers to DRAM enable");

    //Dual core
    REGWR(hev_base1, 0x53, regs->hev_bank1.reg53, "dual core dbf");
    regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
    regs->hev_bank1.reg_mhe_dual_bs0_rstz = 1;  //not reset
    REGWR(hev_base1, 0x54, regs->hev_bank1.reg54, "stream reset done");

    //NewME
    regs->hev_bank2.reg_hev_newme_h_search_max = 0x6;
    regs->hev_bank2.reg_hev_newme_v_search_max = 0x3;
    REGWR(hev_base2, 0x5a, regs->hev_bank2.reg5a, "integer search range");
    regs->hev_bank2.reg_hev_newme_en = 1;
    REGWR(hev_base2, 0x5c, regs->hev_bank2.reg5c, "enable newme");

#if defined(MMHE_IMI_BUF_ADDR)
    REGWR(hev_base2, 0x60, regs->hev_bank2.reg60, "IMI");
    REGWR(hev_base2, 0x61, regs->hev_bank2.reg61, "IMI");
    REGWR(hev_base2, 0x62, regs->hev_bank2.reg62, "IMI");
    REGWR(hev_base2, 0x63, regs->hev_bank2.reg63, "IMI");
    REGWR(mhe_base0, 0x00, regs->mhe_bank0.reg00, "IMI");
    REGWR(mhe_base0, 0x01, regs->mhe_bank0.reg01, "IMI");
    REGWR(mhe_base0, 0x40, regs->mhe_bank0.reg40, "IMI");
    REGWR(mhe_base0, 0x45, regs->mhe_bank0.reg45, "IMI");
#endif

#if 0
    CamOsPrintf("reg_hev_enc_pel_width_m1 = %x\n", regs->hev_bank0.reg_hev_enc_pel_width_m1);
    CamOsPrintf("reg_hev_enc_pel_height_m1 = %x\n", regs->hev_bank0.reg_hev_enc_pel_height_m1);
    CamOsPrintf("reg_hev_src_luma_pel_width = %x\n", regs->hev_bank0.reg_hev_src_luma_pel_width);
    CamOsPrintf("reg_hev_src_chroma_pel_width = %x\n", regs->hev_bank0.reg_hev_src_chroma_pel_width);
    CamOsPrintf("reg_hev_enc_ctb_cnt_m1 = %x\n", regs->hev_bank0.reg_hev_enc_ctb_cnt_m1);
    CamOsPrintf("reg_hev_gn_sz_ctb_m1 = %x\n", regs->hev_bank0.reg_hev_gn_sz_ctb_m1);
    CamOsPrintf("reg_hev_ppu_fb_pitch = %x\n", regs->hev_bank0.reg_hev_ppu_fb_pitch);
    CamOsPrintf("reg_hev_ppu_fb_pitch_lsb = %x\n", regs->hev_bank0.reg_hev_ppu_fb_pitch_lsb);
    CamOsPrintf("reg_hev_ec_multislice_1st_ctby = %x\n", regs->hev_bank0.reg_hev_ec_multislice_1st_ctby);
#endif

    /* Clear and mask CmdQ IRQ */
    REGWR(hev_base1, 0x2c, regs->hev_bank1.reg2c, "cmdq_irq_mask and cmdq_irq_force");
    REGWR(hev_base1, 0x2d, regs->hev_bank1.reg2d, "cmdq_irq_clr");

    regs->hev_bank0.reg_hev_soft_rstz = 1;
    REGWR(hev_base0, 0x00, regs->hev_bank0.reg00, "SW reset 1");
    regs->hev_bank0.reg_hev_soft_rstz = 1;
    REGWR(hev_base0, 0x00, regs->hev_bank0.reg00, "SW reset 1");

    /* Set output address */
    SetOutbsAddr(hev_base0, regs, regs->outbs_addr, regs->outbs_size);

    /* Enable HW */
    regs->hev_bank0.reg_w1c_hev_frame_start = 1;
    regs->hev_bank0.reg_hev_soft_rstz = 1; //not reset
    REGWR(hev_base0, 0x00, regs->hev_bank0.reg00, "frame start (write one clear)");
    regs->hev_bank0.reg_w1c_hev_frame_start = 0;

    //CamOsHexdump(regs->coded_data, regs->coded_bits / 8);

    /* Write SPS/PPS/User data/Slice header */
    PutCodedBits(hev_base0, regs, regs->coded_data, regs->coded_bits);

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : SetOutbsAddrGenCmd
//  Description : Set output bitstream buffer.
//------------------------------------------------------------------------------
static void _SetOutbsAddrGenCmd(void*       base,
                                mhe_reg*   regs,
                                uint32      addr,
                                int         size,
                                void* cmd_buf,
                                int* cmd_len,
                                int cmd_len_max)
{
    uint32  value, end_value;

    // Set end address
    value = addr;
    end_value = value + size - 1;

    //CamOsPrintf("[%s %d] start addr: 0x%p, end addr: 0x%p, size = %d\n", __FUNCTION__,__LINE__,addr, end_value, size);

    regs->hev_bank0.reg_hev_ec_bspobuf_adr_low = (uint16)(end_value >> 4);   //(16 byte unit)
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_high = (uint16)(end_value >> 20); //16+4
    regs->hev_bank0.reg_hev_ec_bspobuf_id = 1;  //Means end address.
    GEN_REG_CMD(base, 0x31, regs->hev_bank0.reg31, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x32, regs->hev_bank0.reg32, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_w1c_hev_ec_bspobuf_set_adr = 1;
    regs->hev_bank0.reg_hev_ec_bspobuf_fifo_th = 7;
    GEN_REG_CMD(base, 0x30, regs->hev_bank0.reg30,  cmd_buf, cmd_len, cmd_len_max);

    // Set start address
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_low = (uint16)(value >> 4);  //(16 byte unit)
    regs->hev_bank0.reg_hev_ec_bspobuf_adr_high = (uint16)(value >> 20); //16+4
    regs->hev_bank0.reg_hev_ec_bspobuf_id = 0;  //Means start address.
    GEN_REG_CMD(base, 0x31, regs->hev_bank0.reg31, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x32, regs->hev_bank0.reg32, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_w1c_hev_ec_bspobuf_set_adr = 1;
    regs->hev_bank0.reg_hev_ec_bspobuf_fifo_th = 7;
    GEN_REG_CMD(base, 0x30, regs->hev_bank0.reg30, cmd_buf, cmd_len, cmd_len_max);

}

//------------------------------------------------------------------------------
//  Function    : PutCodedBitsGenCmd
//  Description : Put SPS/PPS/User data/Slice header to FDC.
//------------------------------------------------------------------------------
static void _PutCodedBitsGenCmd(void*       base,
                                mhe_reg*   regs,
                                uchar*      code,
                                int         size,
                                void* cmd_buf,
                                int* cmd_len,
                                int cmd_len_max)
{

    // Calculate total round.
    int round = (size + 15) >> 4;
    ushort word = 0;

    regs->hev_bank0.reg49 = 0;
    regs->hev_bank0.reg4a = 0;
    // Reset reg4b fdc, keep mdc settings.
    regs->hev_bank0.reg_ro_hev_ec_fdc_ack = 0;
    regs->hev_bank0.reg_w1c_hev_ec_fdc_done_clr = 0;
    regs->hev_bank0.reg_ro_hev_ec_fdc_done = 0;
    regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0;

    if(size == 0)
    {
        regs->hev_bank0.reg_hev_ec_fdc_bs_count = 0;
        GEN_REG_CMD(base, 0x4a, regs->hev_bank0.reg4a, cmd_buf, cmd_len, cmd_len_max);
        regs->hev_bank0.reg_hev_ec_fdc_bs = 0;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = 0;
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x49, regs->hev_bank0.reg49, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4a, regs->hev_bank0.reg4a, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4b, regs->hev_bank0.reg4b, cmd_buf, cmd_len, cmd_len_max);
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
        return;
    }

    // Fill the total round regs
    regs->hev_bank0.reg_hev_ec_fdc_bs_count  = round - 1; // Count from 0
    GEN_REG_CMD(base, 0x4a, regs->hev_bank0.reg4a, cmd_buf, cmd_len, cmd_len_max);

    while(size > 16)
    {
        word = (code[0] << 8) | code[1];
        code += 2;
        size -= 16;

        regs->hev_bank0.reg_hev_ec_fdc_bs = word;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = 15; // This is wrong. But HW don't care; HW uses this value only when last FDC round.
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x49, regs->hev_bank0.reg49, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4a, regs->hev_bank0.reg4a, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4b, regs->hev_bank0.reg4b, cmd_buf, cmd_len, cmd_len_max);
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
    }

    if(size > 0)
    {
        word = code[0] << 8;
        if(size > 8)
            word += code[1];

        regs->hev_bank0.reg_hev_ec_fdc_bs = word;
        regs->hev_bank0.reg_hev_ec_fdc_bs_len = size;
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x49, regs->hev_bank0.reg49, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4a, regs->hev_bank0.reg4a, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x4b, regs->hev_bank0.reg4b, cmd_buf, cmd_len, cmd_len_max);
        regs->hev_bank0.reg_w1c_hev_ec_fdc_bs_vld = 0; // HW is write-one-clear
    }

    // Clear FDC registers
    regs->hev_bank0.reg_hev_ec_fdc_bs_count = 0;
    regs->hev_bank0.reg_hev_ec_fdc_bs = 0;
    regs->hev_bank0.reg_hev_ec_fdc_bs_len = 0;

}
//------------------------------------------------------------------------------
//  Function    : _EncFireGenCmd
//  Description : Generate encode register command sequence.
//------------------------------------------------------------------------------
static int _EncFireGenCmd(mhve_ios* ios, mhve_job* job, void* cmd_buf, int* cmd_len)
{

    mhe_ios* asic = (mhe_ios*)ios;
    mhe_reg* regs = (mhe_reg*)job;
    //void* base = asic->p_base;
    void* hev_base0 =  asic->p_base[0];
#if defined(MMHE_IMI_BUF_ADDR)
    void* mhe_base0 =  asic->p_base[1];
#endif
    void* hev_base1 =  asic->p_base[2];
    void* hev_base2 =  asic->p_base[3];
    int cmd_len_max = 0;

    if(!cmd_buf || !cmd_len)
        return -1;

    cmd_len_max = *cmd_len;

    *cmd_len = 0;

    asic->p_regs = regs;

    /* Enable Clock */
    //REGWR_P(REG_CKG_MHE_BASE, 0x00, regs->regClk, "Set Clk");

    /* Flush regs to mhe-hw */
    GEN_REG_CMD(hev_base0, 0x1c, IRQ_MASK, cmd_buf, cmd_len, cmd_len_max); //MskIrq(base, IRQ_MASK);

    // stream reset
    if(!regs->coded_framecnt)
    {
        regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
        regs->hev_bank1.reg_mhe_dual_bs0_rstz = 0;  //reset
        GEN_REG_CMD(hev_base1, 0x54, regs->hev_bank1.reg54, cmd_buf, cmd_len, cmd_len_max);

        regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
        regs->hev_bank1.reg_mhe_dual_bs0_rstz = 1;  //not reset
        GEN_REG_CMD(hev_base1, 0x54, regs->hev_bank1.reg54, cmd_buf, cmd_len, cmd_len_max);
    }

    // SW reset
    regs->hev_bank0.reg_hev_soft_rstz = 0;      //reset
    GEN_REG_CMD(hev_base0, 0x00, regs->hev_bank0.reg00, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base0, 0x01, regs->hev_bank0.reg01, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x02, regs->hev_bank0.reg02, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x03, regs->hev_bank0.reg03, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x04, regs->hev_bank0.reg04, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x05, regs->hev_bank0.reg05, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x06, regs->hev_bank0.reg06, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x07, regs->hev_bank0.reg07, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x08, regs->hev_bank0.reg08, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x09, regs->hev_bank0.reg09, cmd_buf, cmd_len, cmd_len_max);


    GEN_REG_CMD(hev_base0, 0x10, regs->hev_bank0.reg10, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x20, regs->hev_bank0.reg20, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x21, regs->hev_bank0.reg21, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x22, regs->hev_bank0.reg22, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base0, 0x23, regs->hev_bank0.reg23, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x24, regs->hev_bank0.reg24, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base0, 0x25, regs->hev_bank0.reg25, cmd_buf, cmd_len, cmd_len_max);

    // [QP]
    GEN_REG_CMD(hev_base1, 0x01, regs->hev_bank1.reg01, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x33, regs->hev_bank0.reg33, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x48, regs->hev_bank0.reg48, cmd_buf, cmd_len, cmd_len_max);

    // [GN]
    GEN_REG_CMD(hev_base0, 0x35, regs->hev_bank0.reg35, cmd_buf, cmd_len, cmd_len_max);


    // [MVC]
    GEN_REG_CMD(hev_base0, 0x40, regs->hev_bank0.reg40, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x41, regs->hev_bank0.reg41, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x42, regs->hev_bank0.reg42, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x43, regs->hev_bank0.reg43, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x44, regs->hev_bank0.reg44, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x45, regs->hev_bank0.reg45, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x46, regs->hev_bank0.reg46, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x47, regs->hev_bank0.reg47, cmd_buf, cmd_len, cmd_len_max);

    // [GDR] (Multi-Slice)
    GEN_REG_CMD(hev_base0, 0x4c, regs->hev_bank0.reg4c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x4d, regs->hev_bank0.reg4d, cmd_buf, cmd_len, cmd_len_max);

    //RDO
    GEN_REG_CMD(hev_base0, 0x50, regs->hev_bank0.reg50, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x51, regs->hev_bank0.reg51, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x52, regs->hev_bank0.reg52, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x53, regs->hev_bank0.reg53, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x54, regs->hev_bank0.reg54, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x55, regs->hev_bank0.reg55, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x56, regs->hev_bank0.reg56, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x57, regs->hev_bank0.reg57, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x58, regs->hev_bank0.reg58, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x59, regs->hev_bank0.reg59, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x5a, regs->hev_bank0.reg5a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x5b, regs->hev_bank0.reg5b, cmd_buf, cmd_len, cmd_len_max);

    //penalty
    GEN_REG_CMD(hev_base0, 0x5c, regs->hev_bank0.reg5c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x5d, regs->hev_bank0.reg5d, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x5e, regs->hev_bank0.reg5e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x5f, regs->hev_bank0.reg5f, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x60, regs->hev_bank0.reg60, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x61, regs->hev_bank0.reg61, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x62, regs->hev_bank0.reg62, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x63, regs->hev_bank0.reg63, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x64, regs->hev_bank0.reg64, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x65, regs->hev_bank0.reg65, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x65, regs->hev_bank0.reg66, cmd_buf, cmd_len, cmd_len_max);

    //deblocking
    GEN_REG_CMD(hev_base0, 0x6c, regs->hev_bank0.reg6c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x6d, regs->hev_bank0.reg6d, cmd_buf, cmd_len, cmd_len_max);

    //PMBR
    GEN_REG_CMD(hev_base1, 0x00, regs->hev_bank1.reg00, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x01, regs->hev_bank1.reg01, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x02, regs->hev_bank1.reg02, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x03, regs->hev_bank1.reg03, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x04, regs->hev_bank1.reg04, cmd_buf, cmd_len, cmd_len_max);
    // Pixel Color Weight
    GEN_REG_CMD(hev_base1, 0x05, regs->hev_bank1.reg05, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x06, regs->hev_bank1.reg06, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x07, regs->hev_bank1.reg07, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x08, regs->hev_bank1.reg08, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x09, regs->hev_bank1.reg09, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x0a, regs->hev_bank1.reg0a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x0b, regs->hev_bank1.reg0b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x0c, regs->hev_bank1.reg0c, cmd_buf, cmd_len, cmd_len_max);

    /* Set buffer address */
    regs->hev_bank0.reg_hev_gn_mem_sadr_low = regs->gn_mem >> 4;
    regs->hev_bank0.reg_hev_gn_mem_sadr_high = regs->gn_mem >> 16;
    GEN_REG_CMD(hev_base0, 0x36, regs->hev_bank0.reg36, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x37, regs->hev_bank0.reg37, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base1, 0x14, regs->hev_bank1.reg14, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x15, regs->hev_bank1.reg15, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base1, 0x12, regs->hev_bank1.reg12, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x13, regs->hev_bank1.reg13, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base1, 0x0e, regs->hev_bank1.reg0e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x0f, regs->hev_bank1.reg0f, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base1, 0x10, regs->hev_bank1.reg10, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x11, regs->hev_bank1.reg11, cmd_buf, cmd_len, cmd_len_max);

    if(regs->coded_framecnt)
    {
        regs->hev_bank0.reg_hev_col_r_sadr0_low = regs->col_w_sadr_buf[0] >> 4;
        regs->hev_bank0.reg_hev_col_r_sadr0_high = regs->col_w_sadr_buf[0] >> 16;
    }
    GEN_REG_CMD(hev_base0, 0x3a, regs->hev_bank0.reg3a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x3b, regs->hev_bank0.reg3b, cmd_buf, cmd_len, cmd_len_max);


    regs->hev_bank0.reg_hev_col_w_sadr_low = regs->col_w_sadr_buf[1] >> 4;
    regs->hev_bank0.reg_hev_col_w_sadr_high = regs->col_w_sadr_buf[1] >> 16;
    GEN_REG_CMD(hev_base0, 0x38, regs->hev_bank0.reg38, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x39, regs->hev_bank0.reg39, cmd_buf, cmd_len, cmd_len_max);

    // [PPU]
    regs->hev_bank0.reg_hev_ppu_fb_b_y_base_low = regs->ppu_int_b >> 9;
    regs->hev_bank0.reg_hev_ppu_fb_b_y_base_high = regs->ppu_int_b >> 16;
    GEN_REG_CMD(hev_base0, 0x68, regs->hev_bank0.reg68, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x69, regs->hev_bank0.reg69, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_hev_ppu_fb_a_y_base_low = regs->ppu_int_a >> 9;
    regs->hev_bank0.reg_hev_ppu_fb_a_y_base_high = regs->ppu_int_a >> 16;
    GEN_REG_CMD(hev_base0, 0x6a, regs->hev_bank0.reg6a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x6b, regs->hev_bank0.reg6b, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(hev_base0, 0x6f, regs->hev_bank0.reg6f,  cmd_buf, cmd_len, cmd_len_max);

    //reconstruct/reference buffer
    regs->hev_bank0.reg_hev_ppu_fblut_luma_base_low = regs->ppu_y_base_buf[0] >> 9;
    regs->hev_bank0.reg_hev_ppu_fblut_luma_base_high = regs->ppu_y_base_buf[0] >> 16;
    GEN_REG_CMD(hev_base0, 0x70, regs->hev_bank0.reg70, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x71, regs->hev_bank0.reg71, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_hev_ppu_fblut_chroma_base_low = regs->ppu_c_base_buf[0] >> 9;
    regs->hev_bank0.reg_hev_ppu_fblut_chroma_base_high = regs->ppu_c_base_buf[0] >> 16;
    GEN_REG_CMD(hev_base0, 0x72, regs->hev_bank0.reg72, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x73, regs->hev_bank0.reg73, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_hev_ref_y_adr_low = regs->ppu_y_base_buf[1] >> 8;
    regs->hev_bank0.reg_hev_ref_y_adr_high = regs->ppu_y_base_buf[1] >> 16;
    GEN_REG_CMD(hev_base0, 0x28, regs->hev_bank0.reg28, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x29, regs->hev_bank0.reg29, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_hev_ref_c_adr_low = regs->ppu_c_base_buf[1] >> 8;
    regs->hev_bank0.reg_hev_ref_c_adr_high = regs->ppu_c_base_buf[1] >> 16;
    GEN_REG_CMD(hev_base0, 0x2a, regs->hev_bank0.reg2a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base0, 0x2b, regs->hev_bank0.reg2b, cmd_buf, cmd_len, cmd_len_max);

#if defined(MMHE_IMI_BUF_ADDR)
    regs->hev_bank2.reg_hev_refy_imi_sadr_low = regs->imi_ref_y_buf >> 5;
    regs->hev_bank2.reg_hev_refy_imi_sadr_high = regs->imi_ref_y_buf >> 16;

    GEN_REG_CMD(hev_base2, 0x61, regs->hev_bank2.reg61, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x62, regs->hev_bank2.reg62, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank2.reg_hev_mcc_imi_sadr_low = regs->imi_ref_c_buf >> 5;
    regs->hev_bank2.reg_hev_mcc_imi_sadr_high = regs->imi_ref_c_buf >> 16;

    GEN_REG_CMD(hev_base2, 0x63, regs->hev_bank2.reg63, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x64, regs->hev_bank2.reg64, cmd_buf, cmd_len, cmd_len_max);
#endif

    /* Set output threshold size */
    _SetOutbsThdSizeGenCmd(hev_base1, regs, regs->outbs_size, 1, cmd_buf,cmd_len,cmd_len_max);

    /* Set dump register address */
    GEN_REG_CMD(hev_base2, 0x65, regs->hev_bank2.reg65, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x66, regs->hev_bank2.reg66, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x67, regs->hev_bank2.reg67, cmd_buf, cmd_len, cmd_len_max);

    /* Dual core */
    GEN_REG_CMD(hev_base1, 0x53, regs->hev_bank1.reg53, cmd_buf, cmd_len, cmd_len_max);
    regs->hev_bank1.reg_mhe_dual_bs1_rstz = 1;  //not reset
    regs->hev_bank1.reg_mhe_dual_bs0_rstz = 1;  //not reset
    GEN_REG_CMD(hev_base1, 0x54, regs->hev_bank1.reg54, cmd_buf, cmd_len, cmd_len_max);

    /*  NewME */
    regs->hev_bank2.reg_hev_newme_h_search_max = 0x6;
    regs->hev_bank2.reg_hev_newme_v_search_max = 0x3;
    GEN_REG_CMD(hev_base2, 0x5a, regs->hev_bank2.reg5a, cmd_buf, cmd_len, cmd_len_max);
    regs->hev_bank2.reg_hev_newme_en = 1;
    GEN_REG_CMD(hev_base2, 0x5c, regs->hev_bank2.reg5c, cmd_buf, cmd_len, cmd_len_max);

    /* IMI */
#if defined(MMHE_IMI_BUF_ADDR)
    GEN_REG_CMD(hev_base2, 0x60, regs->hev_bank2.reg60, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x61, regs->hev_bank2.reg61, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x62, regs->hev_bank2.reg62, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base2, 0x63, regs->hev_bank2.reg63, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(mhe_base0, 0x00, regs->mhe_bank0.reg00, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(mhe_base0, 0x01, regs->mhe_bank0.reg01, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(mhe_base0, 0x40, regs->mhe_bank0.reg40, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(mhe_base0, 0x45, regs->mhe_bank0.reg45, cmd_buf, cmd_len, cmd_len_max);
#endif

    /* Clear and mask CmdQ IRQ */
    GEN_REG_CMD(hev_base1, 0x2c, regs->hev_bank1.reg2c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(hev_base1, 0x2d, regs->hev_bank1.reg2d, cmd_buf, cmd_len, cmd_len_max);

    regs->hev_bank0.reg_hev_soft_rstz = 1;
    GEN_REG_CMD(hev_base0, 0x00, regs->hev_bank0.reg00, cmd_buf, cmd_len, cmd_len_max);
    regs->hev_bank0.reg_hev_soft_rstz = 1;
    GEN_REG_CMD(hev_base0, 0x00, regs->hev_bank0.reg00, cmd_buf, cmd_len, cmd_len_max);

    /* Set output address */
    _SetOutbsAddrGenCmd(hev_base0, regs, regs->outbs_addr, regs->outbs_size, cmd_buf, cmd_len, cmd_len_max);

    /* Enable HW */
    regs->hev_bank0.reg_w1c_hev_frame_start = 1;
    regs->hev_bank0.reg_hev_soft_rstz = 1; //not reset
    GEN_REG_CMD(hev_base0, 0x00, regs->hev_bank0.reg00,  cmd_buf, cmd_len, cmd_len_max);
    regs->hev_bank0.reg_w1c_hev_frame_start = 0;

    //CamOsHexdump(regs->coded_data, regs->coded_bits / 8);

    /* Write SPS/PPS/User data/Slice header */
    _PutCodedBitsGenCmd(hev_base0, regs, regs->coded_data, regs->coded_bits, cmd_buf, cmd_len, cmd_len_max);


    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncPoll
//  Description : Polling encode done and update statistic data.
//------------------------------------------------------------------------------
static int _EncPoll(mhve_ios* ios)
{
    mhe_ios* asic = (mhe_ios*)ios;
    mhe_reg* regs = asic->p_regs;
    //void* base = asic->p_base;
    void* hev_base0 =  asic->p_base[0];
    void* mhe_base0 =  asic->p_base[1];
    void* hev_base1 =  asic->p_base[2];
    void* hev_base2 =  asic->p_base[3];

    int i = 0;
    int regaddr;
    int err = -1;
    int bit = BitIrq(hev_base0);

    if(bit & BIT_ENC_DONE)
        err = 0;
    else if(bit & BIT_BUF_FULL)
        err = 1;
    if(err != 0)
        return err;

    // feedback
    regs->hev_bank1.reg6d = REGRD(hev_base1, 0x6d, "bits size:lo");
    regs->hev_bank1.reg6e = REGRD(hev_base1, 0x6e, "bits size:hi");
    regs->mhe_bank0.reg74 = REGRD(mhe_base0, 0x74, "cycle count0");
    regs->mhe_bank0.reg75 = REGRD(mhe_base0, 0x75, "cycle count1");

    regs->enc_cycles = ((uint)(regs->mhe_bank0.reg75 & 0xFF) << 16) + regs->mhe_bank0.reg74;
    regs->enc_bitcnt = ((uint)(regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_high) << 16) + regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_low;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta); // Get the bit counts of pure bitstream and slice header.

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regaddr = 0x16 + i;
        regs->pmbr_lut_hist[i] = REGRD(hev_base1, regaddr, "LUT entry histogram entry");
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        regaddr = 0x20 + i;
        regs->pmbr_tc_hist[i] = REGRD(hev_base2, regaddr, "mbr tc hist");
        regaddr = 0x00 + i;
        regs->pmbr_pc_hist[i] = REGRD(hev_base2, regaddr, "mbr pc hist");
    }

    regs->hev_bank1.reg29 = REGRD(hev_base1, 0x29, "Texture weight accumulation lo");
    regs->hev_bank1.reg2a = REGRD(hev_base1, 0x2a, "Texture weight accumulation hi");
    regs->pmbr_tc_accum = ((uint)(regs->hev_bank1.reg_mbr_tc_accum_high) << 16) + regs->hev_bank1.reg_mbr_tc_accum_low;

    ClrIrq(hev_base0, 0xFF);

    return err;

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _IsrFunc
//  Description : Polling encode done and update statistic data.
//------------------------------------------------------------------------------
static int _IsrFunc(mhve_ios* ios, int irq)
{
    mhe_ios* asic = (mhe_ios*)ios;
    mhe_reg* regs = asic->p_regs;
    void* hev_base0;
    void* mhe_base0;
    void* hev_base1;
    void* hev_base2;
    uint bit = 0;
    int i = 0;
    int regaddr;

    if(asic)
    {
        regs = asic->p_regs;
        hev_base0 =  asic->p_base[0];
        mhe_base0 =  asic->p_base[1];
        hev_base1 =  asic->p_base[2];
        hev_base2 =  asic->p_base[3];
    }
    else
    {
        MHE_MSG(MHE_MSG_ERR, "> _IsrFunc asic NULL\n");
        return -1;
    }

    if(!regs)
    {
        MHE_MSG(MHE_MSG_ERR, "> _IsrFunc regs NULL\n");
        return -1;
    }

    //bit = BitIrq(mhe_base0);
    //CamOsPrintf("> [%s %d] 0x%p, bit = 0x%x\n", __FUNCTION__,__LINE__, mhe_base0, bit);

    bit = BitIrq(hev_base0);
    //CamOsPrintf("> [%s %d] 0x%p, bit = 0x%x\n", __FUNCTION__,__LINE__, hev_base0, bit);

    MskIrq(hev_base0, 0xFF);

    regs->irq_status  = bit;

    if(!CHECK_IRQ_STATUS(bit, IRQ_FRAME_DONE))
    {
        //IRQ Err Check
        if(CHECK_IRQ_STATUS(bit, IRQ_MARB_BSPOBUF_FULL))
        {
            regs->hev_bank1.reg6d = REGRD(hev_base1, 0x6d, "bits size:lo");
            regs->hev_bank1.reg6e = REGRD(hev_base1, 0x6e, "bits size:hi");
            regs->enc_bitcnt = ((uint)(regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_high) << 16) + regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_low;
            MHE_MSG(MHE_MSG_ERR, "> MHE buffer full %s:%d EncodeSize:%d\n", __FUNCTION__, __LINE__,regs->enc_bitcnt);
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_IMG_BUF_FULL))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_IMG_BUF_FULL \n");
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_EARLY_BSPOBUF_FULL))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_EARLY_BSPOBUF_FULL \n");

            regs->hev_bank0.reg5c = REGRD(hev_base0, 0x5c, "TXIP IDCT");
            MHE_MSG(MHE_MSG_ERR, ">>> Dump IDCT4-8-16-32(%d, %d, %d, %d)\n",
                    regs->hev_bank0.reg_hev_txip_idct4_intrpt,
                    regs->hev_bank0.reg_hev_txip_idct8_intrpt,
                    regs->hev_bank0.reg_hev_txip_idct16_intrpt,
                    regs->hev_bank0.reg_hev_txip_idct32_intrpt);

            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_TXIP_TIME_OUT))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_TXIP_TIME_OUT \n");
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_FS_FAIL))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_FS_FAIL \n");
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_NET_TRIGGER))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_NET_TRIGGER \n");
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
        else if(CHECK_IRQ_STATUS(bit, IRQ_LESS_ROW_DONE))
        {
            MHE_MSG(MHE_MSG_ERR, "> IRQ_LESS_ROW_DONE \n");
            ClrIrq(hev_base0, 0xFF);
            MskIrq(hev_base0, IRQ_MASK);

            return 1;
        }
    }

    // TODO: handle enc_done/buf_full
    regs->hev_bank1.reg6d = REGRD(hev_base1, 0x6d, "bits size:lo");
    regs->hev_bank1.reg6e = REGRD(hev_base1, 0x6e, "bits size:hi");
    regs->mhe_bank0.reg74 = REGRD(mhe_base0, 0x74, "cycle count0");
    regs->mhe_bank0.reg75 = REGRD(mhe_base0, 0x75, "cycle count1");
    regs->enc_cycles = ((uint)(regs->mhe_bank0.reg75 & 0xFF) << 16) + regs->mhe_bank0.reg74;
    regs->enc_bitcnt = ((uint)(regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_high) << 16) + regs->hev_bank1.reg_ro_hev_ec_bsp_bit_cnt_low;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta); // Get the bit counts of pure bitstream and slice header.

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regaddr = 0x16 + i;
        regs->pmbr_lut_hist[i] = REGRD(hev_base1, regaddr, "LUT entry histogram entry");
    }

    for(i = 0; i < (1 << PMBR_LOG2_HIST_SIZE); i++)
    {
        regaddr = 0x20 + i;
        regs->pmbr_tc_hist[i] = REGRD(hev_base2, regaddr, "mbr tc hist");
        regaddr = 0x00 + i;
        regs->pmbr_pc_hist[i] = REGRD(hev_base2, regaddr, "mbr pc hist");
    }


    regs->hev_bank1.reg29 = REGRD(hev_base1, 0x29, "Texture weight accumulation lo");
    regs->hev_bank1.reg2a = REGRD(hev_base1, 0x2a, "Texture weight accumulation hi");
    regs->pmbr_tc_accum = ((uint)(regs->hev_bank1.reg_mbr_tc_accum_high) << 16) + regs->hev_bank1.reg_mbr_tc_accum_low;


    ClrIrq(hev_base0, 0xFF);
    MskIrq(hev_base0, IRQ_MASK);

    /* Trigger CmdQ IRQ */
    if (bit & BIT_ENC_DONE)
    {
        REGWR(hev_base1, 0x2d, 0xFFFF, "cmdq_irq_clr");
        REGWR(hev_base1, 0x2c, 0x807F, "cmdq_irq_mask and cmdq_irq_force");
        REGWR(hev_base1, 0x2c, 0x00FF, "cmdq_irq_mask and cmdq_irq_force");
        REGWR(hev_base1, 0x2d, 0xFFFF, "cmdq_irq_clr");
    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _IrqMask
//  Description : Mask IRQ.
//------------------------------------------------------------------------------
static int _IrqMask(mhve_ios* ios, int msk)
{

    mhe_ios* asic = (mhe_ios*)ios;
    //void* base = asic->p_base;
    void* hev_base0 =  asic->p_base[0];
    //void* mhe_base0 =  asic->p_base[1];
    //void* hev_base1 =  asic->p_base[2];
    //void* hev_base2 =  asic->p_base[3];

    ClrIrq(hev_base0, 0xFF);
    MskIrq(hev_base0, 0xFF);

    return 0;
}
