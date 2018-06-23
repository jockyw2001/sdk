#include "hal_mfe_def.h"
#include "hal_mfe_reg.h"
#include "hal_mfe_ios.h"

#define REG_CKG_BASE            0x1F207000
#define REG_CKG_MFE_BASE        (REG_CKG_BASE+0x69*4)

#define  GEN_REG_CMD(base, idx, val, buf, buf_cnt, buf_max) (*(u64*)(cmd_buf+((*cmd_len<(buf_max-1))?(*cmd_len)++:(*cmd_len))*8) = ((u64)(((u32)base+4*idx) & 0x00FFFFFF) >> 1) | ((u64)val & 0x0000FFFF) << 32)

static int _SetBank(mhve_ios*, mhve_reg*);
static int _EncFire(mhve_ios*, mhve_job*);
static int _EncFireGenCmd(mhve_ios*, mhve_job*, void* cmd_buf, int* cmd_len);
static int _EncPoll(mhve_ios*);
static int _IsrFunc(mhve_ios*, int);
static int _IrqMask(mhve_ios*, int);
static void _Release(void* p);

static unsigned long partial_offset;

static int _busywait(int count)
{
    volatile int i = count;
    while (--i > 0) ;
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : MfeIosAcquire
//  Description : Allocate mfe6_ios object and link its member function.
//------------------------------------------------------------------------------
mhve_ios* MfeIosAcquire(char* tags)
{
    mfe6_ios* asic = MEM_ALLC(sizeof(mfe6_ios));

    if (asic)
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
}

//------------------------------------------------------------------------------
//  Function    : _Release
//  Description : Release this object.
//------------------------------------------------------------------------------
static void _Release(void* p)
{
    if (p) MEM_FREE(p);
}

//------------------------------------------------------------------------------
//  Function    : _SetBank
//  Description : Set register bank.
//------------------------------------------------------------------------------
static int _SetBank(mhve_ios* ios, mhve_reg* reg)
{
    int err = 0;
    mfe6_ios* asic = (mfe6_ios*)ios;

    switch (reg->i_id)
    {
        case 0:
            asic->p_base = reg->base;
            break;
        default:
            err = -1;
            break;
    }
    return err;
}

//------------------------------------------------------------------------------
//  Function    : _ClrIrq
//  Description :
//------------------------------------------------------------------------------
static void _ClrIrq(void* base, uint bits)
{
    REGWR(base, 0x1d, bits, "clr irq");
}

//------------------------------------------------------------------------------
//  Function    : _MskIrq
//  Description :
//------------------------------------------------------------------------------
static void _MskIrq(void* base, uint bits)
{
    REGWR(base, 0x1c, bits, "msk irq");
}

//------------------------------------------------------------------------------
//  Function    : _BitIrq
//  Description :
//------------------------------------------------------------------------------
static uint _BitIrq(void* base)
{
    return 0xFF & REGRD(base, 0x1e, "bit irq");
}

//------------------------------------------------------------------------------
//  Function    : _SetOutbsAddr
//  Description : Set output bitstream buffer.
//------------------------------------------------------------------------------
static void _SetOutbsAddr(  void*       base,
                            mfe6_reg*   regs,
                            uint32      addr,
                            int         size)
{
    uint32  value;

    // Enable set-obuf
    regs->reg3f_s_bspobuf_update_adr = 1;
    REGWR(base, 0x3f, regs->reg3f, "ref_mfe_s_bspobuf_update_adr");
    regs->reg3f_s_bspobuf_update_adr = 0;  // write-one-clear

    // Set start address
    value = addr;
    regs->reg3e = 0;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr"); // EROY CHECK : Write reg 2 times?
    //REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 0; // Means start address.
    REGWR(base, 0x3c, regs->reg3c, "bspobuf addr_lo");
    REGWR(base, 0x3d, regs->reg3d, "bspobuf addr_hi");
    regs->reg3e_s_obuf_write_id_adr = 1;
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");

    // Set end address
    value += size;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    //REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 1; // Means end address.
    REGWR(base, 0x3c, regs->reg3c, "bspobuf addr_lo");
    REGWR(base, 0x3d, regs->reg3d, "bspobuf addr_hi");
    regs->reg3e_s_obuf_write_id_adr = 1;
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");

    // Address set
    regs->reg3b_s_mvobuf_set_adr = 0;
    regs->reg3b_s_mvobuf_fifo_th = 0;
    regs->reg3b_s_bspobuf_fifo_th = 1;
    regs->reg3b_s_bspobuf_set_adr = 1;
    REGWR(base, 0x3b, regs->reg3b, "set bsp obuf");
    regs->reg3b_s_bspobuf_set_adr = 0; // HW is write-one-clear
}

static void _UpdateOutbsAddr(   void*       base,
                                mfe6_reg*   regs,
                                uint32      addr,
                                int         size)
{
    uint32  value;

    // Set start address
    value = addr;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr"); // EROY CHECK : Write reg 2 times?
    //REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 0; // Means start address.
    REGWR(base, 0x3c, regs->reg3c, "bspobuf addr_lo");
    REGWR(base, 0x3d, regs->reg3d, "bspobuf addr_hi");
    regs->reg3e_s_obuf_write_id_adr = 1;
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");

    // Set end address
    value += size;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    //REGWR(base, 0x3e, regs->reg3e, "write_id_addr");
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 1; // Means end address.
    REGWR(base, 0x3c, regs->reg3c, "bspobuf addr_lo");
    REGWR(base, 0x3d, regs->reg3d, "bspobuf addr_hi");
    regs->reg3e_s_obuf_write_id_adr = 1;
    REGWR(base, 0x3e, regs->reg3e, "write_id_addr");

    // Address set
    regs->reg3b_s_bspobuf_set_adr = 1;
    REGWR(base, 0x3b, regs->reg3b, "set bsp obuf");
    regs->reg3b_s_bspobuf_set_adr = 0; // HW is write-one-clear
}

//------------------------------------------------------------------------------
//  Function    : _PutCodedBits
//  Description : Put SPS/PPS/User data/Slice header to FDC.
//------------------------------------------------------------------------------
static void _PutCodedBits(  void*       base,
                            mfe6_reg*   regs,
                            uchar*      code,
                            int         size)
{
    int round = (size+15)>>4;
    ushort word = 0;

    regs->reg46 = regs->reg47 = regs->reg48 = 0;

    if (size == 0)
    {
        regs->reg47_s_fdc_bs_count = 0;
        REGWR(base, 0x47, regs->reg47, "fdc round count");
        regs->reg46_s_fdc_bs = 0;
        regs->reg47_s_fdc_bs_len = 0;
        regs->reg48_s_fdc_bs_vld = 1;
        REGWR(base, 0x46, regs->reg46, "fdc bs ");
        REGWR(base, 0x47, regs->reg47, "fdc len");
        REGWR(base, 0x48, regs->reg48, "fdc vld");
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
        return;
    }

    // Fill the total round regs
    regs->reg47_s_fdc_bs_count = round - 1; // Count from 0
    REGWR(base, 0x47, regs->reg47, "fdc round count");

    while (size > 16)
    {
        word = (code[0]<<8) | code[1];
        code += 2;
        size -= 16;

        regs->reg46_s_fdc_bs = word;
        regs->reg47_s_fdc_bs_len = 15;
        regs->reg48_s_fdc_bs_vld = 1;
        REGWR(base, 0x46, regs->reg46, "fdc bs ");
        REGWR(base, 0x47, regs->reg47, "fdc len");
        REGWR(base, 0x48, regs->reg48, "fdc vld");
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
    }

    if (size > 0)
    {
        word = code[0]<<8;
        if (size > 8)
            word += code[1];

        regs->reg46_s_fdc_bs = word;
        regs->reg47_s_fdc_bs_len = size;
        regs->reg48_s_fdc_bs_vld = 1;
        REGWR(base, 0x46, regs->reg46, "fdc bs ");
        REGWR(base, 0x47, regs->reg47, "fdc len");
        REGWR(base, 0x48, regs->reg48, "fdc vld");
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
    }
}

//------------------------------------------------------------------------------
//  Function    : _EncFire
//  Description : Trigger encode.
//------------------------------------------------------------------------------
static int _EncFire(mhve_ios* ios, mhve_job* job)
{
    mfe6_ios* asic = (mfe6_ios*)ios;
    mfe6_reg* regs = (mfe6_reg*)job;
    void* base = asic->p_base;

    asic->p_regs = regs;

    /* Enable Clock */
    //REGWR(REG_CKG_MFE_BASE, 0x00, regs->regClk, "Set Clk");

    /* Flush regs to mfe-hw */
    _MskIrq(base, 0x00);

    // Switch to sw mode
    regs->reg03_g_tbc_mode = 0;
    REGWR(base, 0x03, regs->reg03, "tbc_mode=0");
    // Switch to hw mode
    regs->reg03_g_tbc_mode = 1;
    REGWR(base, 0x03, regs->reg03, "tbc_mode=1");

    // SW reset
    regs->reg00_g_soft_rstz = 0;
    REGWR(base, 0x00, regs->reg00, "SW reset 0");
    _busywait(800);
    regs->reg00_g_soft_rstz = 1;
    REGWR(base, 0x00, regs->reg00, "SW reset 1");
    REGWR(base, 0x80, regs->reg80, "pp mode");
    REGWR(base, 0x01, regs->reg01, "pic width");
    REGWR(base, 0x02, regs->reg02, "pic height");
    REGWR(base, 0x90, regs->reg90, "capture width y");
    REGWR(base, 0x91, regs->reg91, "capture width c");
    REGWR(base, 0xf2, regs->regf2, "roi settings");
    REGWR(base, 0xf3, regs->regf3, "plnrldr setting");
    REGWR(base, 0xf4, regs->regf4, "plnrldr enable");
    REGWR(base, 0xfb, regs->regfb, "roi qmap addr lo");
    REGWR(base, 0xfc, regs->regfc, "roi qmap addr hi");
    REGWR(base, 0xfd, regs->regfd, "eco");
    REGWR(base, 0x03, regs->reg03, "value");
    REGWR(base, 0x04, regs->reg04, "er_bs mode threshold");
    REGWR(base, 0x05, regs->reg05, "inter prediction perference");

    // MB-pitch / packed422 / yuv-loader
    REGWR(base, 0x8b, regs->reg8b, "mb pitch");
    REGWR(base, 0xd2, regs->regd2, "packed422 mode");
    REGWR(base, 0xd6, regs->regd6, "yuv loader");

    // SAMBC
    REGWR(base, 0xdb, regs->regdb, "sambc");
    REGWR(base, 0xdc, regs->regdc, "sambc");
    REGWR(base, 0xdd, regs->regdd, "sambc");
    REGWR(base, 0xde, regs->regde, "sambc");
    REGWR(base, 0xdf, regs->regdf, "sambc");
    REGWR(base, 0xe0, regs->rege0, "sambc");
    REGWR(base, 0xe1, regs->rege1, "sambc");

    // Set picture-buffer address
    REGWR(base, 0x06, regs->reg06, "curr luma base addr_lo");
    REGWR(base, 0x07, regs->reg07, "curr luma base addr_hi");
    REGWR(base, 0x08, regs->reg08, "curr cbcr base addr_lo");
    REGWR(base, 0x09, regs->reg09, "curr cbcr base addr_hi");
    REGWR(base, 0x0a, regs->reg0a, "ref0 luma base addr_lo");
    REGWR(base, 0x0b, regs->reg0b, "ref0 luma base addr_hi");
    REGWR(base, 0x0c, regs->reg0c, "ref1 luma base addr_lo");
    REGWR(base, 0x0d, regs->reg0d, "ref1 luma base addr_hi");
    REGWR(base, 0x0e, regs->reg0e, "ref0 cbcr base addr_lo");
    REGWR(base, 0x0f, regs->reg0f, "ref0 cbcr base addr_hi");
    REGWR(base, 0x10, regs->reg10, "ref1 cbcr base addr_lo");
    REGWR(base, 0x11, regs->reg11, "ref1 cbcr base addr_hi");
    REGWR(base, 0x12, regs->reg12, "recn luma base addr_lo");
    REGWR(base, 0x13, regs->reg13, "recn luma base addr_hi");
    REGWR(base, 0x14, regs->reg14, "recn cbcr base addr_lo");
    REGWR(base, 0x15, regs->reg15, "recn cbcr base addr_hi");

    // Clock gating
    REGWR(base, 0x16, regs->reg16, "clock gating");

    // ME setting
    REGWR(base, 0x20, regs->reg20, "me partition setting");
    REGWR(base, 0x21, regs->reg21, "value");
    REGWR(base, 0x22, regs->reg22, "me search range max depth");
    REGWR(base, 0x23, regs->reg23, "me mvx");
    REGWR(base, 0x24, regs->reg24, "me mvy");
    REGWR(base, 0x25, regs->reg25, "FME");

    // GN
    REGWR(base, 0x4c, regs->reg4c, "regmfe_s_gn_saddr_lo");
    REGWR(base, 0x4d, regs->reg4d, "regmfe_s_gn_saddr_hi");

    // MBR
    REGWR(base, 0x26, regs->reg26, "MBR: mbbits");
    REGWR(base, 0x27, regs->reg27, "MBR: frame qstep");
    REGWR(base, 0x29, regs->reg29, "h264 qp offset");
    REGWR(base, 0x2a, regs->reg2a, "QP min/max");
    REGWR(base, 0x6e, regs->reg6e, "QStep min");
    REGWR(base, 0x6f, regs->reg6f, "QStep max");

    // Perceptual MBR
#if !defined(MFE_DISABLE_PMBR)
    REGWR(base, 0x100, regs->reg100, "PMBR");
    REGWR(base, 0x101, regs->reg101, "PMBR");
    REGWR(base, 0x102, regs->reg102, "PMBR");
    REGWR(base, 0x103, regs->reg103, "PMBR");
    REGWR(base, 0x104, regs->reg104, "PMBR");
    REGWR(base, 0x105, regs->reg105, "PMBR");
    REGWR(base, 0x106, regs->reg106, "PMBR");
    REGWR(base, 0x107, regs->reg107, "PMBR");
    REGWR(base, 0x108, regs->reg108, "PMBR");
    REGWR(base, 0x109, regs->reg109, "PMBR");
    REGWR(base, 0x10a, regs->reg10a, "PMBR");
    REGWR(base, 0x10b, regs->reg10b, "PMBR");
    REGWR(base, 0x10c, regs->reg10c, "PMBR");
    REGWR(base, 0x10e, regs->reg10e, "PMBR Out Map Addr");
    REGWR(base, 0x10f, regs->reg10f, "PMBR Out Map Addr");

    REGWR(base, 0x110, regs->reg110, "PMBR In Map Addr");
    REGWR(base, 0x111, regs->reg111, "PMBR In Map Addr");
    REGWR(base, 0x112, regs->reg112, "PMBR LUT Addr");
    REGWR(base, 0x113, regs->reg113, "PMBR LUT Addr");
    REGWR(base, 0x114, regs->reg114, "PMBR");
    REGWR(base, 0x115, regs->reg115, "PMBR");
#endif

    // INTRA UPDATE
    REGWR(base, 0x2f, regs->reg2f, "value");
    REGWR(base, 0x30, regs->reg30, "value");
    REGWR(base, 0x31, regs->reg31, "value");
    REGWR(base, 0x32, regs->reg32, "value");

    // MDC
    REGWR(base, 0x39, regs->reg39, "value");

    // DBF
    REGWR(base, 0x3a, regs->reg3a, "value");

    // CRC
    regs->reg73_g_crc_mode = 0xC;
    regs->reg73_g_debug_tcycle_chk_en = 0x1;
    regs->reg73_g_debug_tcycle_chk_sel = 0x1;
    regs->reg73_g_debug_en = 0; // TEST
    REGWR(base, 0x73, regs->reg73, "crc mode");
    REGWR(base, 0x2c, regs->reg2c, "last zigzag");

    // IEAP
    REGWR(base, 0x2b, regs->reg2b, "ieap");

    // Cross-format wrong regs setting prevention
    REGWR(base, 0x18, regs->reg18, "jpe encode mode");
    REGWR(base, 0x1b, regs->reg1b, "mpeg4 field dct");

    // regs->reg19 MUST be zero
    REGWR(base, 0x19, 0, "0/1:MPEG4 enable/disable p skip mode");

    // Prefetch & Low bandwidth mode
    REGWR(base, 0x68, regs->reg68, "prefetch & low bandwidth mode");

    // Prefetch
    REGWR(base, 0x6d, regs->reg6d, "prefetch mb idle count");

    // Low bandwidth
    REGWR(base, 0x6b, regs->reg6b, "low bandwidth: IMI addr_lo");
    REGWR(base, 0x6c, regs->reg6c, "low bandwidth: IMI addr_hi");

    // IMI DBF
    REGWR(base, 0x4e, regs->reg4e, "DBF: IMI addr_lo");
    REGWR(base, 0x4f, regs->reg4f, "DBF: IMI addr_hi");

    // ZMV
    REGWR(base, 0x86, regs->reg86, "ZMv base addr lo");

    // Penalties
    REGWR(base, 0x87, regs->reg87, "ZMv base addr hi & i4x penalty");
    REGWR(base, 0x88, regs->reg88, "penalties i16 & inter");
    REGWR(base, 0x89, regs->reg89, "penalties i16  planar");

    // Reset any StopAndGo or StopAndDrop setting.
    regs->reg2d_s_txip_sng_mb = 0;
    REGWR(base, 0x2d, regs->reg2d, "reg2d_s_txip_sng_mb=0");

    // Enable eco item
    REGWR(base, 0x7d, regs->reg7d, "reg7d_s_txip_eco0=1");

    // POC
    REGWR(base, 0xf7, regs->regf7, "sh->i_poc");
    REGWR(base, 0xf8, regs->regf8, "poc_enable & poc_width");

    /* Set output address */
    partial_offset = 0;
    if (regs->notify_size)
    {
        _SetOutbsAddr(base, regs, regs->outbs_addr, regs->notify_size-8);
    }
    else
    {
        _SetOutbsAddr(base, regs, regs->outbs_addr, regs->outbs_size);
    }

    /* Multi-Slice */
    REGWR(base, 0xca, regs->regca, "multislice_mby");
    REGWR(base, 0xcb, regs->regcb, "multislice_1st_mby");

    /* Set dump register address */
    REGWR(base, 0xa7, regs->rega7, "dump_reg_addr_lo");
    REGWR(base, 0xa8, regs->rega8, "dump_reg_addr_hi");
    REGWR(base, 0xa9, regs->rega9, "dump_reg en");

    /* Clear and mask CmdQ IRQ */
    REGWR(base, 0x17b, regs->reg17b, "cmdq_irq_mask and cmdq_irq_force");
    REGWR(base, 0x17c, regs->reg17c, "cmdq_irq_clr");

#ifndef SIMULATE_ON_I3
    /* Enable HW */
    regs->reg00_g_frame_start_sw = 1;
    REGWR(base, 0x00, regs->reg00, "frame start");
    regs->reg00_g_frame_start_sw = 0;    // HW is write-one-clear
#endif
    /* Write SPS/PPS/User data/Slice header */
    _PutCodedBits(base, regs, regs->coded_data, regs->coded_bits);

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _SetOutbsAddrGenCmd
//  Description : Set output bitstream buffer.
//------------------------------------------------------------------------------
static void _SetOutbsAddrGenCmd(void*       base,
                               mfe6_reg*   regs,
                               uint32      addr,
                               int         size,
                               void* cmd_buf,
                               int* cmd_len,
                               int cmd_len_max)
{
    uint32  value;

    // Enable set-obuf
    regs->reg3f_s_bspobuf_update_adr = 1;
    GEN_REG_CMD(base, 0x3f, regs->reg3f, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3f_s_bspobuf_update_adr = 0;  // write-one-clear

    // Set start address
    value = addr;
    regs->reg3e = 0;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max); // EROY CHECK : Write reg 2 times?
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 0; // Means start address.
    GEN_REG_CMD(base, 0x3c, regs->reg3c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x3d, regs->reg3d, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3e_s_obuf_write_id_adr = 1;
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max);

    // Set end address
    value += size;
    regs->reg3e_s_obuf_write_id_adr = 0; // For Safety
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3c_s_bspobuf_lo = (uint16)(value>> 3);
    regs->reg3d_s_bspobuf_hi = (uint16)(value>>19);
    regs->reg3d_s_obuf_id = 1; // Means end address.
    GEN_REG_CMD(base, 0x3c, regs->reg3c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x3d, regs->reg3d, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3e_s_obuf_write_id_adr = 1;
    GEN_REG_CMD(base, 0x3e, regs->reg3e, cmd_buf, cmd_len, cmd_len_max);

    // Address set
    regs->reg3b_s_mvobuf_set_adr = 0;
    regs->reg3b_s_mvobuf_fifo_th = 0;
    regs->reg3b_s_bspobuf_fifo_th = 1;
    regs->reg3b_s_bspobuf_set_adr = 1;
    GEN_REG_CMD(base, 0x3b, regs->reg3b, cmd_buf, cmd_len, cmd_len_max);
    regs->reg3b_s_bspobuf_set_adr = 0; // HW is write-one-clear
}

//------------------------------------------------------------------------------
//  Function    : _PutCodedBitsGenCmd
//  Description : Put SPS/PPS/User data/Slice header to FDC.
//------------------------------------------------------------------------------
static void _PutCodedBitsGenCmd(void*       base,
                               mfe6_reg*   regs,
                               uchar*      code,
                               int         size,
                               void* cmd_buf,
                               int* cmd_len,
                               int cmd_len_max)
{
    int round = (size+15)>>4;
    ushort word = 0;

    regs->reg46 = regs->reg47 = regs->reg48 = 0;

    if (size == 0)
    {
        regs->reg47_s_fdc_bs_count = 0;
        GEN_REG_CMD(base, 0x47, regs->reg47, cmd_buf, cmd_len, cmd_len_max);
        regs->reg46_s_fdc_bs = 0;
        regs->reg47_s_fdc_bs_len = 0;
        regs->reg48_s_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x46, regs->reg46, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x47, regs->reg47, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x48, regs->reg48, cmd_buf, cmd_len, cmd_len_max);
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
        return;
    }

    // Fill the total round regs
    regs->reg47_s_fdc_bs_count = round - 1; // Count from 0
    GEN_REG_CMD(base, 0x47, regs->reg47, cmd_buf, cmd_len, cmd_len_max);

    while (size > 16)
    {
        word = (code[0]<<8) | code[1];
        code += 2;
        size -= 16;

        regs->reg46_s_fdc_bs = word;
        regs->reg47_s_fdc_bs_len = 15;
        regs->reg48_s_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x46, regs->reg46, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x47, regs->reg47, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x48, regs->reg48, cmd_buf, cmd_len, cmd_len_max);
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
    }

    if (size > 0)
    {
        word = code[0]<<8;
        if (size > 8)
            word += code[1];

        regs->reg46_s_fdc_bs = word;
        regs->reg47_s_fdc_bs_len = size;
        regs->reg48_s_fdc_bs_vld = 1;
        GEN_REG_CMD(base, 0x46, regs->reg46, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x47, regs->reg47, cmd_buf, cmd_len, cmd_len_max);
        GEN_REG_CMD(base, 0x48, regs->reg48, cmd_buf, cmd_len, cmd_len_max);
        regs->reg48_s_fdc_bs_vld = 0; // write-one-clear
    }
}

//------------------------------------------------------------------------------
//  Function    : _EncFireGenCmd
//  Description : Generate encode register command sequence.
//------------------------------------------------------------------------------
static int _EncFireGenCmd(mhve_ios* ios, mhve_job* job, void* cmd_buf, int* cmd_len)
{
    mfe6_ios* asic = (mfe6_ios*)ios;
    mfe6_reg* regs = (mfe6_reg*)job;
    void* base = asic->p_base;
    int cmd_len_max = 0;

    if (!cmd_buf || !cmd_len)
        return -1;

    cmd_len_max = *cmd_len;

    *cmd_len = 0;

    asic->p_regs = regs;

    /* Enable Clock */
    //REGWR_P(REG_CKG_MFE_BASE, 0x00, regs->regClk, "Set Clk");

    /* Flush regs to mfe-hw */
    GEN_REG_CMD(base, 0x1c, 0x00, cmd_buf, cmd_len, cmd_len_max); //MskIrq(base, 0x00);

    // Switch to sw mode
    regs->reg03_g_tbc_mode = 0;
    GEN_REG_CMD(base, 0x03, regs->reg03, cmd_buf, cmd_len, cmd_len_max);
    // Switch to hw mode
    regs->reg03_g_tbc_mode = 1;
    GEN_REG_CMD(base, 0x03, regs->reg03, cmd_buf, cmd_len, cmd_len_max);

    // SW reset
    regs->reg00_g_soft_rstz = 0;
    GEN_REG_CMD(base, 0x00, regs->reg00, cmd_buf, cmd_len, cmd_len_max);
    //_busywait(800);
    regs->reg00_g_soft_rstz = 1;
    GEN_REG_CMD(base, 0x00, regs->reg00, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x80, regs->reg80, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x01, regs->reg01, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x02, regs->reg02, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x90, regs->reg90, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x91, regs->reg91, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xf2, regs->regf2, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xf3, regs->regf3, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xf4, regs->regf4, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xfb, regs->regfb, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xfc, regs->regfc, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xfd, regs->regfd, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x03, regs->reg03, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x04, regs->reg04, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x05, regs->reg05, cmd_buf, cmd_len, cmd_len_max);

    // MB-pitch / packed422 / yuv-loader
    GEN_REG_CMD(base, 0x8b, regs->reg8b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xd2, regs->regd2, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xd6, regs->regd6, cmd_buf, cmd_len, cmd_len_max);

    // SAMBC
    GEN_REG_CMD(base, 0xdb, regs->regdb, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xdc, regs->regdc, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xdd, regs->regdd, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xde, regs->regde, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xdf, regs->regdf, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xe0, regs->rege0, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xe1, regs->rege1, cmd_buf, cmd_len, cmd_len_max);

    // Set picture-buffer address
    GEN_REG_CMD(base, 0x06, regs->reg06, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x07, regs->reg07, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x08, regs->reg08, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x09, regs->reg09, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0a, regs->reg0a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0b, regs->reg0b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0c, regs->reg0c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0d, regs->reg0d, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0e, regs->reg0e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x0f, regs->reg0f, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10, regs->reg10, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x11, regs->reg11, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x12, regs->reg12, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x13, regs->reg13, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x14, regs->reg14, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x15, regs->reg15, cmd_buf, cmd_len, cmd_len_max);

    // Clock gating
    GEN_REG_CMD(base, 0x16, regs->reg16, cmd_buf, cmd_len, cmd_len_max);

    // ME setting
    GEN_REG_CMD(base, 0x20, regs->reg20, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x21, regs->reg21, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x22, regs->reg22, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x23, regs->reg23, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x24, regs->reg24, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x25, regs->reg25, cmd_buf, cmd_len, cmd_len_max);

    // GN
    GEN_REG_CMD(base, 0x4c, regs->reg4c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x4d, regs->reg4d, cmd_buf, cmd_len, cmd_len_max);

    // MBR
    GEN_REG_CMD(base, 0x26, regs->reg26, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x27, regs->reg27, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x29, regs->reg29, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x2a, regs->reg2a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x6e, regs->reg6e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x6f, regs->reg6f, cmd_buf, cmd_len, cmd_len_max);

    // Perceptual MBR
#if !defined(MFE_DISABLE_PMBR)
    GEN_REG_CMD(base, 0x100, regs->reg100, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x101, regs->reg101, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x102, regs->reg102, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x103, regs->reg103, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x104, regs->reg104, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x105, regs->reg105, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x106, regs->reg106, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x107, regs->reg107, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x108, regs->reg108, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x109, regs->reg109, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10a, regs->reg10a, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10b, regs->reg10b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10c, regs->reg10c, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10e, regs->reg10e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x10f, regs->reg10f, cmd_buf, cmd_len, cmd_len_max);

    GEN_REG_CMD(base, 0x110, regs->reg110, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x111, regs->reg111, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x112, regs->reg112, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x113, regs->reg113, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x114, regs->reg114, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x115, regs->reg115, cmd_buf, cmd_len, cmd_len_max);
#endif

    // INTRA UPDATE
    GEN_REG_CMD(base, 0x2f, regs->reg2f, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x30, regs->reg30, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x31, regs->reg31, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x32, regs->reg32, cmd_buf, cmd_len, cmd_len_max);

    // MDC
    GEN_REG_CMD(base, 0x39, regs->reg39, cmd_buf, cmd_len, cmd_len_max);

    // DBF
    GEN_REG_CMD(base, 0x3a, regs->reg3a, cmd_buf, cmd_len, cmd_len_max);

    // CRC
    regs->reg73_g_crc_mode = 0xC;
    regs->reg73_g_debug_tcycle_chk_en = 0x1;
    regs->reg73_g_debug_tcycle_chk_sel = 0x1;
    regs->reg73_g_debug_en = 0; // TEST
    GEN_REG_CMD(base, 0x73, regs->reg73, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x2c, regs->reg2c, cmd_buf, cmd_len, cmd_len_max);

    // IEAP
    GEN_REG_CMD(base, 0x2b, regs->reg2b, cmd_buf, cmd_len, cmd_len_max);

    // Cross-format wrong regs setting prevention
    GEN_REG_CMD(base, 0x18, regs->reg18, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x1b, regs->reg1b, cmd_buf, cmd_len, cmd_len_max);

    // regs->reg19 MUST be zero
    GEN_REG_CMD(base, 0x19, 0, cmd_buf, cmd_len, cmd_len_max);

    // Prefetch & Low bandwidth mode
    GEN_REG_CMD(base, 0x68, regs->reg68, cmd_buf, cmd_len, cmd_len_max);

    // Prefetch
    GEN_REG_CMD(base, 0x6d, regs->reg6d, cmd_buf, cmd_len, cmd_len_max);

    // Low bandwidth
    GEN_REG_CMD(base, 0x6b, regs->reg6b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x6c, regs->reg6c, cmd_buf, cmd_len, cmd_len_max);

    // IMI DBF
    GEN_REG_CMD(base, 0x4e, regs->reg4e, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x4f, regs->reg4f, cmd_buf, cmd_len, cmd_len_max);

    // ZMV
    GEN_REG_CMD(base, 0x86, regs->reg86, cmd_buf, cmd_len, cmd_len_max);

    // Penalties
    GEN_REG_CMD(base, 0x87, regs->reg87, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x88, regs->reg88, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x89, regs->reg89, cmd_buf, cmd_len, cmd_len_max);

    // Reset any StopAndGo or StopAndDrop setting.
    regs->reg2d_s_txip_sng_mb = 0;
    GEN_REG_CMD(base, 0x2d, regs->reg2d, cmd_buf, cmd_len, cmd_len_max);

    // Enable eco item
    GEN_REG_CMD(base, 0x7d, regs->reg7d, cmd_buf, cmd_len, cmd_len_max);

    // POC
    GEN_REG_CMD(base, 0xf7, regs->regf7, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xf8, regs->regf8, cmd_buf, cmd_len, cmd_len_max);

    /* Set output address */
    _SetOutbsAddrGenCmd(base, regs, regs->outbs_addr, regs->outbs_size, cmd_buf, cmd_len, cmd_len_max);

    /* Multi-Slice */
    GEN_REG_CMD(base, 0xca, regs->regca, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xcb, regs->regcb, cmd_buf, cmd_len, cmd_len_max);

    /* Set dump register address */
    GEN_REG_CMD(base, 0xa7, regs->rega7, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xa8, regs->rega8, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0xa9, regs->rega9, cmd_buf, cmd_len, cmd_len_max);

    /* Clear and mask CmdQ IRQ */
    GEN_REG_CMD(base, 0x17b, regs->reg17b, cmd_buf, cmd_len, cmd_len_max);
    GEN_REG_CMD(base, 0x17c, regs->reg17c, cmd_buf, cmd_len, cmd_len_max);

    /* Enable HW */
    regs->reg00_g_frame_start_sw = 1;
    GEN_REG_CMD(base, 0x00, regs->reg00, cmd_buf, cmd_len, cmd_len_max);
    regs->reg00_g_frame_start_sw = 0;    // HW is write-one-clear

    /* Write SPS/PPS/User data/Slice header */
    _PutCodedBitsGenCmd(base, regs, regs->coded_data, regs->coded_bits, cmd_buf, cmd_len, cmd_len_max);

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : _EncPoll
//  Description : Polling encode done and update statistic data.
//------------------------------------------------------------------------------
static int _EncPoll(mhve_ios* ios)
{
    mfe6_ios* asic = (mfe6_ios*)ios;
    mfe6_reg* regs = asic->p_regs;
    void* base = asic->p_base;
    int i = 0;
    int err = -1;
    int bit = _BitIrq(base);

    if (bit & BIT_ENC_DONE)
        err = 0;
    else if (bit & BIT_BUF_FULL)
        err = 1;
    if (err != 0)
        return err;

    // feedback
    regs->reg28 = REGRD(base, 0x28, "sum-qstep:lo");
    regs->reg29 = REGRD(base, 0x29, "sum-qstep:hi");
    regs->regf5 = REGRD(base, 0xf5, "sum-qstep:lo");
    regs->regf6 = REGRD(base, 0xf6, "sum-qstep:hi");
    regs->reg42 = REGRD(base, 0x42, "bits size:lo");
    regs->reg43 = REGRD(base, 0x43, "bits size:hi");
    regs->reg76 = REGRD(base, 0x76, "cycle count0");
    regs->reg77 = REGRD(base, 0x77, "cycle count1");
    regs->reg129 = REGRD(base, 0x129,"Texture weight accumulation:lo");
    regs->reg12a = REGRD(base, 0x12a,"Texture weight accumulation:hi");

    regs->enc_cycles = ((uint)(regs->reg77 & 0xFF)<<16) + regs->reg76;
    regs->enc_bitcnt = ((uint)(regs->reg43_s_bsp_bit_cnt_hi)<<16) + regs->reg42_s_bsp_bit_cnt_lo;
    regs->enc_sumpqs = ((uint)(regs->regf6_s_mbr_last_frm_avg_qp_hi)<<16) + regs->regf5_s_mbr_last_frm_avg_qp_lo;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta); // Get the bit counts of pure bitstream and slice header.

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regs->pmbr_lut_hist[i] = REGRD(base, (0x116 + i), "LUT entry histogram entry");
    }

    for (i = 0; i < (1<<PMBR_LOG2_HIST_SIZE); i++) {
        regs->pmbr_tc_hist[i] = REGRD(base, (0x150 + i), "mbr tc hist"); // reg150_mbr_hist_tw_0
        regs->pmbr_pc_hist[i] = REGRD(base, (0x130 + i), "mbr pc hist"); // reg130_mbr_hist_pc_0
    }

    regs->pmbr_tc_accum = (uint)((regs->reg12a_mbr_tc_accum_high<<16) + regs->reg129_mbr_tc_accum_low);

    _ClrIrq(base, 0xFF);
    return err;
}

//------------------------------------------------------------------------------
//  Function    : _IsrFunc
//  Description : Polling encode done and update statistic data.
//------------------------------------------------------------------------------
static int _IsrFunc(mhve_ios* ios, int irq)
{
    mfe6_ios* asic = (mfe6_ios*)ios;
    mfe6_reg* regs = NULL;
    void* base = NULL;
    uint bit = 0;
    int i = 0;
    unsigned long encoded_size;
    int err = 0;
#ifdef SIMULATE_DUMP_REG
    mfe6_dump_reg* dump_reg;
#endif
    if (asic)
    {
        regs = asic->p_regs;
        base = asic->p_base;
    }
    else
    {
        MFE_MSG(MFE_MSG_ERR, "> _IsrFunc asic NULL\n");
        return -1;
    }

    bit = _BitIrq(base);
    _MskIrq(base, 0xFF);

#ifdef SIMULATE_DUMP_REG
    if (regs)
    {
        dump_reg = (mfe6_dump_reg *)regs->dump_reg_vaddr;
    }
    else
    {
        MFE_MSG(MFE_MSG_ERR, "> _IsrFunc regs NULL\n");
        return -1;
    }

    //CamOsPrintf("SIMULATE_DUMP_REG(ISR)  0x%08X\n", (u32)dump_reg);

    dump_reg->reg42 = REGRD(base, 0x42, "bits size:lo");
    dump_reg->reg43 = REGRD(base, 0x43, "bits size:hi");
    dump_reg->reg44 = REGRD(base, 0x44, "");
    dump_reg->reg45 = REGRD(base, 0x45, "");
    dump_reg->regb0 = REGRD(base, 0xb0, "");
    dump_reg->regb1 = REGRD(base, 0xb1, "");
    dump_reg->regb2 = REGRD(base, 0xb2, "");
    dump_reg->regb3 = REGRD(base, 0xb3, "");

    dump_reg->regd7 = REGRD(base, 0xd7, "");
    dump_reg->regd8 = REGRD(base, 0xd8, "");
    dump_reg->regd9 = REGRD(base, 0xd9, "");
    dump_reg->regda = REGRD(base, 0xda, "");

    for (i = 0; i < 15; i++) {
        dump_reg->reg116[i] = REGRD(base, 0x116 + i, "histogram");
    }

    for (i = 0; i < (1<<PMBR_LOG2_HIST_SIZE); i++) {
        dump_reg->reg150[i] = REGRD(base, 0x150 + i, "mbr tc hist"); // reg150_mbr_hist_tw_0
        dump_reg->reg130[i] = REGRD(base, 0x130 + i, "mbr pc hist"); // reg130_mbr_hist_pc_0
    }

    dump_reg->reg126 = REGRD(base, 0x126, "");
    dump_reg->reg127 = REGRD(base, 0x127, "");
    dump_reg->reg128 = REGRD(base, 0x128, "");

    dump_reg->reg1e  = bit & 0xFFFF;

    dump_reg->reg12a_hi = REGRD(base, 0x12a, "") >> 8;
    dump_reg->reg12b = REGRD(base, 0x12b, "");
    dump_reg->reg129 = REGRD(base, 0x129, "");
    dump_reg->reg12a_lo = REGRD(base, 0x12a, "") & 0xFF;

    dump_reg->regf5 = REGRD(base, 0xf5, "avg-qp:lo");
    dump_reg->regf6 = REGRD(base, 0xf6, "avg-qp:hi");
    encoded_size = (((uint)dump_reg->reg43<<16) + dump_reg->reg42)/8;
#else
    // TODO: handle enc_done/buf_full
    regs->reg28 = REGRD(base, 0x28, "avg-qp:lo");
    regs->reg29 = REGRD(base, 0x29, "avg-qp:hi");
    regs->regf5 = REGRD(base, 0xf5, "avg-qp:lo");
    regs->regf6 = REGRD(base, 0xf6, "avg-qp:hi");
    regs->reg42 = REGRD(base, 0x42, "bits size:lo");
    regs->reg43 = REGRD(base, 0x43, "bits size:hi");
    regs->reg76 = REGRD(base, 0x76, "cycle count0");
    regs->reg77 = REGRD(base, 0x77, "cycle count1");
    regs->reg129 = REGRD(base, 0x129,"Texture weight accumulation:lo");
    regs->reg12a = REGRD(base, 0x12a,"Texture weight accumulation:hi");

    regs->enc_cycles = ((uint)(regs->reg77&0xFF)<<16) + regs->reg76;
    regs->enc_bitcnt = ((uint)(regs->reg43_s_bsp_bit_cnt_hi)<<16) + regs->reg42_s_bsp_bit_cnt_lo;
    regs->enc_sumpqs = ((uint)(regs->regf6_s_mbr_last_frm_avg_qp_hi)<<16) + regs->regf5_s_mbr_last_frm_avg_qp_lo;
    regs->mjob.i_tick = (int)(regs->enc_cycles);
    regs->mjob.i_bits = (int)(regs->enc_bitcnt - regs->bits_delta);
    regs->irq_status  = bit;

    for(i = 0; i < PMBR_LUT_SIZE; i++)
    {
        regs->pmbr_lut_hist[i] = REGRD(base, (0x116 + i), "LUT entry histogram entry");
    }


    for (i = 0; i < (1<<PMBR_LOG2_HIST_SIZE); i++) {
        regs->pmbr_tc_hist[i] = REGRD(base, (0x150 + i), "mbr tc hist"); // reg150_mbr_hist_tw_0
        regs->pmbr_pc_hist[i] = REGRD(base, (0x130 + i), "mbr pc hist"); // reg130_mbr_hist_pc_0
        //CamOsPrintf("regs->pmbr_tc_hist[%d] = %x \n", i, regs->pmbr_tc_hist[i]);
        //CamOsPrintf("regs->pmbr_pc_hist[%d] = %x \n", i, regs->pmbr_pc_hist[i]);
    }

    regs->pmbr_tc_accum = (uint)((regs->reg12a_mbr_tc_accum_high<<16) + regs->reg129_mbr_tc_accum_low);

    if (bit & BIT_BUF_FULL)
    {
        if (!regs->notify_size)
            CamOsPrintf("mfe-enc buffer full(%d/%u)\n", regs->outbs_size, (((uint)(regs->reg43_s_bsp_bit_cnt_hi)<<16) + regs->reg42_s_bsp_bit_cnt_lo)/8);
    }

    encoded_size = (((uint)(regs->reg43_s_bsp_bit_cnt_hi)<<16) + regs->reg42_s_bsp_bit_cnt_lo) / 8;
#endif

    if (regs->notify_size)
    {
        //CamOsPrintf("%s: encoded_size = %u\n", __FUNCTION__, encoded_size);
        _ClrIrq(base, 0xFF);
        if (bit & BIT_BUF_FULL)
        {
            if (regs->notify_func)
                regs->notify_func(regs->outbs_addr, partial_offset, regs->notify_size, 0);

            if (regs->outbs_addr + regs->outbs_size >=
                partial_offset + regs->notify_size + regs->notify_size)
            {
                err = 1;
                partial_offset += regs->notify_size;
                _UpdateOutbsAddr(base, regs, regs->outbs_addr + partial_offset, regs->notify_size-8);
            }
        }
        else if (bit & BIT_ENC_DONE)
        {
            if (regs->notify_func)
                regs->notify_func(regs->outbs_addr, partial_offset, encoded_size, 1);
        }
    }
    else
        _ClrIrq(base, 0xFF);

    _MskIrq(base, 0x00);

    /* Trigger CmdQ IRQ */
    if (bit & BIT_ENC_DONE)
    {
        REGWR(base, 0x17c, 0xFFFF, "cmdq_irq_clr");
        REGWR(base, 0x17b, 0x807F, "cmdq_irq_mask and cmdq_irq_force");
        REGWR(base, 0x17b, 0x00FF, "cmdq_irq_mask and cmdq_irq_force");
        REGWR(base, 0x17c, 0xFFFF, "cmdq_irq_clr");
    }

    return err;
}

//------------------------------------------------------------------------------
//  Function    : _IrqMask
//  Description : Mask IRQ.
//------------------------------------------------------------------------------
static int _IrqMask(mhve_ios* ios, int msk)
{
    mfe6_ios* asic = (mfe6_ios*)ios;
    void* base = asic->p_base;

    _ClrIrq(base, 0xFF);
    _MskIrq(base, 0xFF);
    return 0;
}
