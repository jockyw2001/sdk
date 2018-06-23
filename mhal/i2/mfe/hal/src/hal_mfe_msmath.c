#include <linux/limits.h>   //derek check
#include <linux/module.h>
#include "hal_mfe_def.h"
#include "hal_mfe_msmath.h"
#include "hal_mfe_global.h"

// For debug
//#include <assert.h>

//int64 remainder;
//uint64 uiremainder;
//CamOsMathDivU64(, , &uiremainder);
//CamOsMathDivS64(, , &remainder);

// ------------------------------------------------
static uint32 fExp_frac(uint32 x);
static uint64 fPow64_int(uint32 base, int exponment);
static uint64 fPow64_int_high_order(uint32 base, int exponment);
// ------------------------------------------------
uint32 get_pow_of_two(int x)
{
    //int val = x;
    int p = -1;//, bit_pos = 0;
    while(x > 0)
    {
        p++;
        x >>= 1;
    }
    return (p < 0) ? 0 : p;
}

// -------------------------------------------------------------------------------------------
// fixed-point implementation Q16.16
// -------------------------------------------------------------------------------------------

/**
Babylonian Method with 2^(0.5n) initial guess
*/
#define SQRT_EXTEND_BIT (16)
uint32 fSqrt(uint32 x)
{
    uint64 x_org, x_update, x_incr;
    int bit_num = get_pow_of_two(x >> 16);
    int sqrt_bit_num = bit_num >> 1;
    int guess_int = 1 << sqrt_bit_num;
    uint64 uiremainder;

    x_org = ((uint64)x) << SQRT_EXTEND_BIT;
    x_update = ((uint64)guess_int) << 16;

    x_incr = (CamOsMathDivU64(x_org, x_update, &uiremainder)) << (16 - SQRT_EXTEND_BIT);
    x_update = (x_update + x_incr) >> 1;
    x_incr = (CamOsMathDivU64(x_org, x_update, &uiremainder)) << (16 - SQRT_EXTEND_BIT);
    x_update = (x_update + x_incr) >> 1;
    x_incr = (CamOsMathDivU64(x_org, x_update, &uiremainder)) << (16 - SQRT_EXTEND_BIT);
    x_update = (x_update + x_incr) >> 1;
    x_incr = (CamOsMathDivU64(x_org, x_update, &uiremainder)) << (16 - SQRT_EXTEND_BIT);
    x_update = (x_update + x_incr) >> 1;

    return (uint32) x_update;
}

uint32 fExp(int exponment)
{
    uint32 Euler = 0x2B7E1; // 2.7183*(1<<16)
    uint32 n = (exponment >= 0) ? exponment : -exponment;
    uint32 integer = n >> 16;
    uint32 fraction = n & 0xFFFF;

    int64 exp_frac = fExp_frac(fraction);
    int64 exp_int = fPow64_int(Euler, integer);
    int64 rst = exp_frac * exp_int;
    int64 remainder;

    rst = (exponment >= 0) ? rst >> 16 : CamOsMathDivS64(((int64)1 << 48), rst, &remainder);

     //Overflow checking  //derek check
    //assert(rst<=UINT_MAX,"rst<=UINT_MAX");
    if (rst>UINT_MAX){
        //MFE_MSG(MFE_MSG_WARNING, "rst(%lld), UINT_MAX(%u)\n", rst, UINT_MAX);
        rst = UINT_MAX;
    }

    return (uint32)rst;
}

/**
input range: 0 <= x < 1
*/
#define EXP_FRAC_EXTEND_BIT (8)
#define DIV6_SCALE_E_Q1DOT16 ((int64)0x2AAB)
#define DIV10_SCALE_E_Q1DOT16 ((int64)0x1999)

static uint32 fExp_frac(uint32 x)
{
    int PrecisionBit = 16 + EXP_FRAC_EXTEND_BIT;
    int64 denom, nom, common_c_0_2_4_term, common_c_1_3_5_term;
    int64 x_org = ((int64)x) << EXP_FRAC_EXTEND_BIT;
    int64 ret;

    int64 c_0 = ((int64)1) << PrecisionBit;
    int64 c_1 = x_org >> 1; // div 2
    int64 c_2 = (c_1 * x_org) >> (2 + PrecisionBit); // div 4
    int64 c_3 = ((c_2 * x_org) * DIV6_SCALE_E_Q1DOT16) >> (16 + PrecisionBit); // div 6
    int64 c_4 = (c_3 * x_org) >> (3 + PrecisionBit); // div 8
    int64 c_5 = ((c_4 * x_org) * DIV10_SCALE_E_Q1DOT16) >> (16 + PrecisionBit); // div 10
    int64 remainder;

    common_c_0_2_4_term = c_0 + c_2 + c_4;
    common_c_1_3_5_term = c_1 + c_3 + c_5;
    nom = common_c_0_2_4_term + common_c_1_3_5_term; //c_0 + c_1 + c_2 + c_3 + c_4 + c_5;
    denom = common_c_0_2_4_term - common_c_1_3_5_term; //c_0 - c_1 + c_2 - c_3 + c_4 - c_5 ;

    CDBZ(denom,"denom");
    ret = CamOsMathDivS64((nom << 16), denom, &remainder);

    return (uint32)ret;
}

/**
input range: 1 <= x < 2
*/
//[1] C. S. Turner,  "A Fast Binary Logarithm Algorithm", IEEE Signal Processing Mag., pp. 124,140, Sep. 2010.

static uint32 fLog2_range1_to_2(uint32 x)
{
    int precision = 16, iter;
    uint32 b = 1U << (precision - 1);
    uint32 y = 0;
    int64 z;
    if(x == 0)
    {
        //printf("Error input x in fLog()");
        assert(0,"x=0"); // represents negative infinity
    }

    while(x < ((uint32)1 << precision))
    {
        x <<= 1;
        y -= (1 << precision);
    }

    while(x >= ((uint32)2 << precision))
    {
        x >>= 1;
        y += (1 << precision);
    }

    z = x;

    for(iter = 0; iter < precision; iter++)
    {
        z = (z * z) >> precision;
        if(z >= (2 << precision))
        {
            z >>= 1;
            y += b;
        }
        b >>= 1;
    }

    return y;
}

/**
// divide to x = A * 2^B
log2(x) = log2(A) + B
*/
static uint32 fLog2(uint32 x)
{
    int intOfx = x >> 16;
    int B = get_pow_of_two(intOfx);
    int64 A = x >> B;
    int64 log2_A = fLog2_range1_to_2((uint32)A);
    int64 log2_x = log2_A + (B << 16);
    return (uint32) log2_x;
}

/**
  x must >= 0
*/
#define INV_LOG2_E_Q1DOT31  ((int64)(0x58b90bfc)) // Inverse log base 2 of e
int fLog(uint32 x)
{
    int intOfx = x >> 16;
    int norm_x;
    int64 log_x;
    int64 remainder;

    if(x == 0)
      return (int)0x80000000;
    if(x == 1)
      return -726817;

    norm_x = (int)((intOfx != 0) ? x : CamOsMathDivS64(((int64)1) << 32, x, &remainder));

    log_x = (fLog2(norm_x) * INV_LOG2_E_Q1DOT31) >> 31;
    return (int)((intOfx != 0) ? log_x : -log_x);
}

/**
a^Y = exp (ln(a^Y)) = exp(Y*ln(a))
*/
// input:  -1 < exponment < 1
static uint32 fPow_frac(uint32 A, int exponment)
{
    int64 Y_x_ln_A;
    uint32 ret;

    int ln_A = fLog(A);
    int sign_ln_A = (ln_A >= 0) ? 0 : 1;
    int sign_exponment = (exponment >= 0) ? 0 : 1;
    int sign_Y_x_ln_A = sign_ln_A ^ sign_exponment;
    int abs_ln_A = (sign_ln_A == 0) ? ln_A : -ln_A;
    int abs_exponment = (sign_exponment == 0) ? exponment : -exponment;
    int64 abs_Y_x_abs_ln_A = ((int64)abs_exponment * abs_ln_A) >> 16;

    // Overflow checking  //derek check
    //assert(abs_Y_x_abs_ln_A<=INT_MAX,"abs_Y_x_abs_ln_A>INT_MAX");
    if (abs_Y_x_abs_ln_A > INT_MAX){
        //MFE_MSG(MFE_MSG_WARNING, "abs_Y_x_abs_ln_A(%lld), INT_MAX(%d)\n", abs_Y_x_abs_ln_A, INT_MAX);
        abs_Y_x_abs_ln_A = INT_MAX;
    }

    Y_x_ln_A = (sign_Y_x_ln_A) ? -abs_Y_x_abs_ln_A : abs_Y_x_abs_ln_A;

    ret = fExp((int)Y_x_ln_A);
    return ret;
}

uint64 fPow64(uint32 base, int exponment)
{
    int abs_exp = (exponment >= 0) ? exponment : -exponment;
    int integer = (exponment >= 0) ? abs_exp >> 16 : -(abs_exp >> 16);
    int fraction = exponment - (integer << 16);
    uint32 pow_frac;
    uint64 pow_int;
    uint64 rst;

    if(exponment == 0)
    {
        return ((uint64)1<<16);
    }

    if(base == 0)
    {
        return 0;
    }

    pow_frac = fPow_frac(base, fraction);
    pow_int = fPow64_int(base, integer);

    rst = ((uint64)pow_frac) * pow_int;
    rst >>= 16;

    return rst;
}

#if 0
#define POW_INT_EXTEND_BIT (8)
static uint64 fPow64_int_high_order(uint32 base, int exponment)
{
    uint64 rst = ((uint64)1) << (16 + POW_INT_EXTEND_BIT);
    uint64 pwr = (uint64)base << POW_INT_EXTEND_BIT;
    int bit_pos = exponment;

    while(bit_pos)
    {
        if(bit_pos & 0x01)
        {
            rst *= pwr;
            rst >>= (16 + POW_INT_EXTEND_BIT);
        }
        pwr *= pwr;
        pwr >>= (16 + POW_INT_EXTEND_BIT);
        bit_pos >>= 1;
    }
    return rst >> POW_INT_EXTEND_BIT;
}
#else
static uint64 high_preci_AxB(uint64 A, uint64 B)
{
    uint64 int_A = A >> 16;
    uint64 int_B = B >> 16;
    uint64 frac_A = A & 0xFFFF;
    uint64 frac_B = B & 0xFFFF;

    uint64 int_mul = (int_A * int_B) << 16;
    uint64 frac_mul = (frac_A * frac_B) >> 16;
    uint64 cross_mul = int_A * frac_B + int_B * frac_A;
    return int_mul + frac_mul + cross_mul;
}

static uint64 fPow64_int_high_order(uint32 base, int exponment)
{
    uint64 rst = ((uint64)1)<<(16);
    uint64 pwr = (uint64)base;
    int bit_pos = exponment;
    while(bit_pos)
    {
        if(bit_pos&0x01)
        {
            rst = high_preci_AxB(rst, pwr);
        }

        pwr = high_preci_AxB(pwr, pwr);
        bit_pos>>=1;
    }
    return rst;
}
#endif

static uint64 fPow64_int(uint32 base, int exponment)
{
    uint64 rst;
    int n = (exponment > 0) ? (exponment) : (-exponment);
    int64 remainder;
    uint64 uiremainder;


    if(n == 0)
    {
        return 1 << 16;
    }
    else if(n == 1)
    {
        rst = (uint64)base;
    }
    else if(n == 2)
    {
        rst = (uint64)base * base;
        return (exponment > 0) ? rst >> 16 : CamOsMathDivS64(((int64)1) << 48, rst, &remainder);
    }
    else
    {
        rst = fPow64_int_high_order(base, n);
    }

    return (exponment > 0) ? rst : (rst==0) ? (uint64)1<<32 : CamOsMathDivU64(((uint64)1) << 32, rst, &uiremainder);
}
