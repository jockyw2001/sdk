#include "hal_ceva.h"
#include "hal_debug.h"

#include "ms_platform.h"
#include <linux/delay.h>

// #define ENABLE_JTAG

#ifdef ENABLE_JTAG
#define RIU_BASE_ADDR   (0x1F000000)
#define BANK_CAL(addr)  ((addr<<9) + (RIU_BASE_ADDR))
#define BANK_GOP  (BANK_CAL(0x1026))
#endif // ENABLE_JTAG

#define LOW_U16(value)   (((u32)(value))&0x0000FFFF)
#define HIGH_U16(value)  ((((u32)(value))&0xFFFF0000)>>16)

#define MAKE_U32(high, low) ((((u32)high)<<16) | low)

#if (HAL_MSG_LEVL < HAL_MSG_DBG)
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  ms_writew(val,((uint)base+(idx)*4))
#else
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  do{HAL_MSG(HAL_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)*4), val); ms_writew(val,((uint)base+(idx)*4));} while(0)
#endif

void ceva_hal_init(ceva_hal_handle *handle, phys_addr_t base_sys, phys_addr_t base_axi2miu0, phys_addr_t base_axi2miu1, phys_addr_t base_axi2miu2, phys_addr_t base_axi2miu3)
{
    HAL_MSG(HAL_MSG_DBG, "init 0x%p, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X\n", handle, base_sys, base_axi2miu0, base_axi2miu1, base_axi2miu2, base_axi2miu3);

    memset(handle, 0, sizeof(ceva_hal_handle));
    handle->base_sys = base_sys;
    handle->base_axi2miu0 = base_axi2miu0;
    handle->base_axi2miu1 = base_axi2miu1;
    handle->base_axi2miu2 = base_axi2miu2;
    handle->base_axi2miu3 = base_axi2miu3;
}

void ceva_hal_disable_irq(ceva_hal_handle *handle, CEVA_HAL_IRQ_TARGET target, CEVA_HAL_IRQ irq)
{
    u16 irq0 = LOW_U16(irq);
    u16 irq1 = HIGH_U16(irq);

    switch(target)
    {
      case CEVA_HAL_IRQ_TARGET_ARM:
        handle->reg_sys.reg03 = REGR(handle->base_sys, 0x03);
        handle->reg_sys.reg_ceva2riu_int_en &= ~irq0;
        REGW(handle->base_sys, 0x03, handle->reg_sys.reg03);

        handle->reg_sys.reg04 = REGR(handle->base_sys, 0x04);
        handle->reg_sys.reg_ceva2riu_int_en2 &= ~irq1;
        REGW(handle->base_sys, 0x04, handle->reg_sys.reg04);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT0:
        handle->reg_sys.reg05 = REGR(handle->base_sys, 0x05);
        handle->reg_sys.reg_ceva_int0_en &= ~irq0;
        REGW(handle->base_sys, 0x05, handle->reg_sys.reg05);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT1:
        handle->reg_sys.reg06 = REGR(handle->base_sys, 0x06);
        handle->reg_sys.reg_ceva_int1_en &= ~irq0;
        REGW(handle->base_sys, 0x06, handle->reg_sys.reg06);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT2:
        handle->reg_sys.reg07 = REGR(handle->base_sys, 0x07);
        handle->reg_sys.reg_ceva_int2_en &= ~irq0;
        REGW(handle->base_sys, 0x07, handle->reg_sys.reg07);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_NMI:
        handle->reg_sys.reg08 = REGR(handle->base_sys, 0x08);
        handle->reg_sys.reg_ceva_nmi_en &= ~irq0;
        REGW(handle->base_sys, 0x08, handle->reg_sys.reg08);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_VINT:
        handle->reg_sys.reg09 = REGR(handle->base_sys, 0x09);
        handle->reg_sys.reg_ceva_vint_en &= ~irq0;
        REGW(handle->base_sys, 0x09, handle->reg_sys.reg09);
        break;

      default:
        break;
    }
}

void ceva_hal_enable_irq(ceva_hal_handle *handle, CEVA_HAL_IRQ_TARGET target, CEVA_HAL_IRQ irq)
{
    u16 irq0 = LOW_U16(irq);
    u16 irq1 = HIGH_U16(irq);

    switch(target)
    {
      case CEVA_HAL_IRQ_TARGET_ARM:
        handle->reg_sys.reg03 = REGR(handle->base_sys, 0x03);
        handle->reg_sys.reg_ceva2riu_int_en |= irq0;
        REGW(handle->base_sys, 0x03, handle->reg_sys.reg03);

        handle->reg_sys.reg04 = REGR(handle->base_sys, 0x04);
        handle->reg_sys.reg_ceva2riu_int_en2 |= irq1;
        REGW(handle->base_sys, 0x04, handle->reg_sys.reg04);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT0:
        handle->reg_sys.reg05 = REGR(handle->base_sys, 0x05);
        handle->reg_sys.reg_ceva_int0_en |= irq0;
        REGW(handle->base_sys, 0x05, handle->reg_sys.reg05);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT1:
        handle->reg_sys.reg06 = REGR(handle->base_sys, 0x06);
        handle->reg_sys.reg_ceva_int1_en |= irq0;
        REGW(handle->base_sys, 0x06, handle->reg_sys.reg06);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT2:
        handle->reg_sys.reg07 = REGR(handle->base_sys, 0x07);
        handle->reg_sys.reg_ceva_int2_en |= irq0;
        REGW(handle->base_sys, 0x07, handle->reg_sys.reg07);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_NMI:
        handle->reg_sys.reg08 = REGR(handle->base_sys, 0x08);
        handle->reg_sys.reg_ceva_nmi_en |= irq0;
        REGW(handle->base_sys, 0x08, handle->reg_sys.reg08);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_VINT:
        handle->reg_sys.reg09 = REGR(handle->base_sys, 0x09);
        handle->reg_sys.reg_ceva_vint_en |= irq0;
        REGW(handle->base_sys, 0x09, handle->reg_sys.reg09);
        break;

      default:
        break;
    }
}

CEVA_HAL_IRQ ceva_hal_get_irq_mask(ceva_hal_handle *handle, CEVA_HAL_IRQ_TARGET target)
{
    u32 enable = 0;

    switch(target)
    {
      case CEVA_HAL_IRQ_TARGET_ARM:
        handle->reg_sys.reg03 = REGR(handle->base_sys, 0x03);
        handle->reg_sys.reg04 = REGR(handle->base_sys, 0x04);
        enable = MAKE_U32(handle->reg_sys.reg_ceva2riu_int_en2, handle->reg_sys.reg_ceva2riu_int_en);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT0:
        handle->reg_sys.reg05 = REGR(handle->base_sys, 0x05);
        enable = MAKE_U32(0, handle->reg_sys.reg_ceva_int0_en);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT1:
        handle->reg_sys.reg06 = REGR(handle->base_sys, 0x06);
        enable = MAKE_U32(0, handle->reg_sys.reg_ceva_int1_en);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_INT2:
        handle->reg_sys.reg07 = REGR(handle->base_sys, 0x07);
        enable = MAKE_U32(0, handle->reg_sys.reg_ceva_int2_en);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_NMI:
        handle->reg_sys.reg08 = REGR(handle->base_sys, 0x08);
        enable = MAKE_U32(0, handle->reg_sys.reg_ceva_nmi_en);
        break;

      case CEVA_HAL_IRQ_TARGET_XM6_VINT:
        handle->reg_sys.reg09 = REGR(handle->base_sys, 0x09);
        enable = MAKE_U32(0, handle->reg_sys.reg_ceva_vint_en);
        break;

      default:
        break;
    }

    return enable;
}


CEVA_HAL_IRQ ceva_hal_get_irq_status(ceva_hal_handle *handle)
{
    u32 irq;

    handle->reg_sys.reg18 = REGR(handle->base_sys, 0x18);
    handle->reg_sys.reg19 = REGR(handle->base_sys, 0x19);
    irq = MAKE_U32(handle->reg_sys.reg_ceva_is2, handle->reg_sys.reg_ceva_is);

    return irq;
}

void ceva_hal_reset_xm6(ceva_hal_handle *handle)
{
    handle->reg_sys.reg00 = REGR(handle->base_sys, 0x00);
    handle->reg_sys.reg02 = REGR(handle->base_sys, 0x02);

    handle->reg_sys.reg_ceva_en = 0;
    REGW(handle->base_sys, 0x00, handle->reg_sys.reg00);

    udelay(1);

    handle->reg_sys.reg_rstz_ceva_core = 0;
    handle->reg_sys.reg_rstz_ceva_sys = 0;
    handle->reg_sys.reg_rstz_ceva_ocem = 0;
    handle->reg_sys.reg_rstz_ceva_global = 0;
    handle->reg_sys.reg_rstz_miu = 0;
    handle->reg_sys.reg_rstz_mcu = 0;
    handle->reg_sys.reg_rstz_mcu2ceva = 0;
    handle->reg_sys.reg_rstz_isp2ceva = 0;
    REGW(handle->base_sys, 0x02, handle->reg_sys.reg02);

    udelay(1);
}

void ceva_hal_enable_xm6(ceva_hal_handle *handle)
{
    handle->reg_sys.reg00 = REGR(handle->base_sys, 0x00);
    handle->reg_sys.reg02 = REGR(handle->base_sys, 0x02);
    handle->reg_sys.reg15 = REGR(handle->base_sys, 0x15);
#ifdef ENABLE_JTAG
    // set JTAG pin out
    REGW(BANK_GOP, 0x04, 0x0300);
#endif // ENABLE_JTAG

    handle->reg_sys.reg_ceva_en = 1;
    REGW(handle->base_sys, 0x00, handle->reg_sys.reg00);

    handle->reg_sys.reg_ceva_boot = 1;
    // handle->reg_sys.reg_ceva_csysreq = 0;  // not sure...
    REGW(handle->base_sys, 0x15, handle->reg_sys.reg15);

    handle->reg_sys.reg_rstz_ceva_sys = 1;
    handle->reg_sys.reg_rstz_ceva_ocem = 1;
    handle->reg_sys.reg_rstz_ceva_global = 1;
    handle->reg_sys.reg_rstz_miu = 1;
    handle->reg_sys.reg_rstz_mcu = 1;
    handle->reg_sys.reg_rstz_mcu2ceva = 1;
    handle->reg_sys.reg_rstz_isp2ceva = 1;
    REGW(handle->base_sys, 0x02, handle->reg_sys.reg02);

    udelay(1);
}

void ceva_hal_bootup_xm6(ceva_hal_handle *handle, u32 boot_addr)
{
    handle->reg_sys.reg02 = REGR(handle->base_sys, 0x02);

    // Set boot up address
    handle->reg_sys.reg_ceva_vector_low  = LOW_U16(boot_addr);
    handle->reg_sys.reg_ceva_vector_high = HIGH_U16(boot_addr);
    REGW(handle->base_sys, 0x16, handle->reg_sys.reg16);
    REGW(handle->base_sys, 0x17, handle->reg_sys.reg17);

    handle->reg_sys.reg_rstz_ceva_core = 1;
    REGW(handle->base_sys, 0x02, handle->reg_sys.reg02);

    udelay(1);

    // Set reset vector as default (0) because ICE may reset again
    handle->reg_sys.reg_ceva_vector_low  = 0;
    handle->reg_sys.reg_ceva_vector_high = 0;
    REGW(handle->base_sys, 0x16, handle->reg_sys.reg16);
    REGW(handle->base_sys, 0x17, handle->reg_sys.reg17);
}

void ceva_hal_reset_warp(ceva_hal_handle *handle)
{
    handle->reg_sys.reg_warp_en = 0;
    REGW(handle->base_sys, 0x40, handle->reg_sys.reg40);

    handle->reg_sys.reg_rstz_warp = 0; // ~CEVA_HAL_RESET_WARP_ALL
    REGW(handle->base_sys, 0x42, handle->reg_sys.reg42);

    // delay 1us
    udelay(1);
}

void ceva_hal_enable_warp(ceva_hal_handle *handle)
{
    handle->reg_sys.reg_warp_en = 0;
    REGW(handle->base_sys, 0x40, handle->reg_sys.reg40);

    handle->reg_sys.reg_rstz_warp = 0;
    REGW(handle->base_sys, 0x42, handle->reg_sys.reg42);

    // delay 1us
    udelay(1);

    handle->reg_sys.reg_warp_en = 1;
    REGW(handle->base_sys, 0x40, handle->reg_sys.reg40);

    // handle->reg_sys.reg_rstz_warp = 0xFFFF;
    handle->reg_sys.reg42 = CEVA_HAL_RESET_WARP_ALL;
    REGW(handle->base_sys, 0x42, handle->reg_sys.reg42);

    // delay 1us
    udelay(1);
}

void ceva_hal_set_axi2miu(ceva_hal_handle *handle)
{
    unsigned short axi2miu2_data = 0;
    unsigned short axi2miu3_data = 0;

    axi2miu2_data = REGR(handle->base_axi2miu2, 0x02);
    axi2miu3_data = REGR(handle->base_axi2miu3, 0x02);

    axi2miu2_data = axi2miu2_data | 0x0001;
    axi2miu3_data = axi2miu3_data | 0x0001;

    REGW(handle->base_axi2miu2, 0x02, axi2miu2_data);
    REGW(handle->base_axi2miu3, 0x02, axi2miu2_data);

    REGW(handle->base_axi2miu2, 0x01, 0x1010);
    REGW(handle->base_axi2miu3, 0x01, 0x1010);
}

void ceva_hal_write_dummy_data(ceva_hal_handle *handle, u32 index, u32 data)
{
    switch (index)
    {
        case 0:
            handle->reg_sys.reg_dummy_0 = LOW_U16(data);
            handle->reg_sys.reg_dummy_1 = HIGH_U16(data);
            REGW(handle->base_sys, 0x60, handle->reg_sys.reg60);
            REGW(handle->base_sys, 0x61, handle->reg_sys.reg61);
            break;

        case 1:
            handle->reg_sys.reg_dummy_2 = LOW_U16(data);
            handle->reg_sys.reg_dummy_3 = HIGH_U16(data);
            REGW(handle->base_sys, 0x62, handle->reg_sys.reg62);
            REGW(handle->base_sys, 0x63, handle->reg_sys.reg63);
            break;

        default:
            HAL_MSG(HAL_MSG_ERR, "Only index 0 ~3 are available\n");

    }
}

u32 ceva_hal_read_dummy_data(ceva_hal_handle *handle, u32 index)
{
    switch (index)
    {
        case 0:
            handle->reg_sys.reg60 = REGR(handle->base_sys, 0x60);
            handle->reg_sys.reg61 = REGR(handle->base_sys, 0x61);
            return MAKE_U32(handle->reg_sys.reg_dummy_1, handle->reg_sys.reg_dummy_0);

        case 1:
            handle->reg_sys.reg62 = REGR(handle->base_sys, 0x62);
            handle->reg_sys.reg63 = REGR(handle->base_sys, 0x63);
            return MAKE_U32(handle->reg_sys.reg_dummy_3, handle->reg_sys.reg_dummy_2);

        default:
            HAL_MSG(HAL_MSG_ERR, "Only index 0 ~3 are available\n");
    }

    return 0;
}

u32 ceva_hal_get_mcci_irq(ceva_hal_handle *handle)
{
    handle->reg_sys.reg0d = REGR(handle->base_sys, 0x0d);
    handle->reg_sys.reg0e = REGR(handle->base_sys, 0x0e);
    HAL_MSG(HAL_MSG_DBG, "status %04x %04x\n", handle->reg_sys.reg0e, handle->reg_sys.reg0d);

    return MAKE_U32(handle->reg_sys.reg0e, handle->reg_sys.reg0d);
}

u32 ceva_hal_check_mcci_irq(ceva_hal_handle *handle, u32 index)
{
    handle->reg_sys.reg0d = REGR(handle->base_sys, 0x0d);
    handle->reg_sys.reg0e = REGR(handle->base_sys, 0x0e);
    HAL_MSG(HAL_MSG_DBG, "mcci state %04x-%04x\n", handle->reg_sys.reg0e, handle->reg_sys.reg0d);

    HAL_MSG(HAL_MSG_DBG, "int state  %04x-%04x\n", REGR(handle->base_sys, 0x19), REGR(handle->base_sys, 0x18));
    HAL_MSG(HAL_MSG_DBG, "int enable %04x-%04x, %04x, %04x, %04x\n", REGR(handle->base_sys, 0x04), REGR(handle->base_sys, 0x03), REGR(handle->base_sys, 0x05), REGR(handle->base_sys, 0x06), REGR(handle->base_sys, 0x07) );

    if (index < 16) {
        return handle->reg_sys.reg_mcci_rd_ind_low  & (1 <<  index);
    } else if (index < 32) {
        return (handle->reg_sys.reg_mcci_rd_ind_high & (1 <<  index)) << 16;
    } else {
        HAL_MSG(HAL_MSG_ERR, "Incorrect index %d\n", index);
    }

    return 0;
}

void ceva_hal_clear_mcci_irq(ceva_hal_handle *handle, u32 index)
{
    u16 value;

    handle->reg_sys.reg0d = REGR(handle->base_sys, 0x0d);
    handle->reg_sys.reg0e = REGR(handle->base_sys, 0x0e);
    HAL_MSG(HAL_MSG_DBG, "status %04x %04x\n", handle->reg_sys.reg0e, handle->reg_sys.reg0d);

    if (index < 16) {
        value = 1 <<  index;
        handle->reg_sys.reg_mcci_rd_wc_low = value;
        REGW(handle->base_sys, 0x0b, handle->reg_sys.reg0b);
    } else if (index < 32) {
        value = 1 << (index-16);
        handle->reg_sys.reg_mcci_rd_wc_high = value;
        REGW(handle->base_sys, 0x0c, handle->reg_sys.reg0c);
    } else {
        HAL_MSG(HAL_MSG_ERR, "Incorrect index %d\n", index);
    }

    handle->reg_sys.reg0d = REGR(handle->base_sys, 0x0d);
    handle->reg_sys.reg0e = REGR(handle->base_sys, 0x0e);
    HAL_MSG(HAL_MSG_DBG, "status %04x %04x\n", handle->reg_sys.reg0e, handle->reg_sys.reg0d);
}

void ceva_hal_clear_mcci_irq_ex(ceva_hal_handle *handle, u32 mask)
{
    handle->reg_sys.reg_mcci_rd_wc_low = LOW_U16(mask);
    REGW(handle->base_sys, 0x0b, handle->reg_sys.reg0b);

    handle->reg_sys.reg_mcci_rd_wc_high = HIGH_U16(mask);
    REGW(handle->base_sys, 0x0c, handle->reg_sys.reg0c);
}
