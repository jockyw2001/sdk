#ifndef _CMDQ_UTIL_H_
#define _CMDQ_UTIL_H_

#define WRITE_BYTE(_reg, _val)      (*((volatile u8*)(_reg)))  = (u8)(_val)
#define WRITE_WORD(_reg, _val)      (*((volatile u16*)(_reg))) = (u16)(_val)
#define WRITE_LONG(_reg, _val)      (*((volatile u32*)(_reg))) = (u32)(_val)
#define READ_BYTE(_reg)             (*(volatile u8*)(_reg))
#define READ_WORD(_reg)             (*(volatile u16*)(_reg))
#define READ_LONG(_reg)             (*(volatile u32*)(_reg))

// Address bus of RIU is 16 bits.
#define RIU_READ_BYTE(riubas,addr)         ( READ_BYTE( riubas+ (addr) ) )
#define RIU_READ_2BYTE(riubas,addr)        ( READ_WORD( riubas + (addr) ) )
#define RIU_WRITE_BYTE(riubas,addr, val)    WRITE_BYTE( (riubas + (addr)),val)
#define RIU_WRITE_2BYTE(riubas,addr, val)   WRITE_WORD( riubas + (addr), val)


//---------------------------------------------------------------------------
//  Function and Variable
//---------------------------------------------------------------------------


#define R2BYTE( riubas,u32Reg ) RIU_READ_2BYTE( riubas,(u32Reg) << 1)

#define R2BYTEMSK( riubas,u32Reg, u16mask)\
    ( riubas,( RIU_READ_2BYTE( (u32Reg)<< 1) & u16mask  ) )
#define R4BYTE( riubas,u32Reg )\
    ( { ((RIU_READ_2BYTE( riubas,(u32Reg) << 1)) | ((u32)(RIU_READ_2BYTE(riubas,( (u32Reg) + 2 ) << 1) ) << 16)) ; } )

#define W2BYTE(riubas, u32Reg, u16Val) RIU_WRITE_2BYTE( riubas,(u32Reg) << 1 , u16Val )

#define W2BYTEMSK(riubas, u32Reg, u16Val, u16Mask)\
    RIU_WRITE_2BYTE( riubas,(u32Reg)<< 1 , (RIU_READ_2BYTE(riubas,(u32Reg) << 1) & ~(u16Mask)) | ((u16Val) & (u16Mask)) )

#define W4BYTE( riubas,u32Reg, u32Val)\
    ( { RIU_WRITE_2BYTE( riubas,(u32Reg) << 1, ((u32Val) & 0x0000FFFF) ); \
        RIU_WRITE_2BYTE( riubas,( (u32Reg) + 2) << 1 , (((u32Val) >> 16) & 0x0000FFFF)) ; } )

#define W3BYTE( riubas,u32Reg, u32Val)\
    ( { RIU_WRITE_2BYTE( riubas,(u32Reg) << 1,  u32Val); \
        RIU_WRITE_BYTE( riubas,(u32Reg + 2) << 1 ,  ((u32Val) >> 16)); } )

#define CMDQ_DO_SIZE_ALIGN(size,align)   (((size+(align-1))/align)*align)
#endif
