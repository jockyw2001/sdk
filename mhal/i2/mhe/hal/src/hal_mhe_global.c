#include "hal_mhe_msmath.h"
#include "hal_mhe_global.h"


// NEED_CHANGE_BY_PREC_SCALE_BITS
// Q16.16
static const uint32 g_auiLambdaByQp[54] =
{
    2506, 3179, 4033, 5117, 6493, 8238, 10452, 13261, 16825, 21347,
    27084, 34363, 43599, 55316, 70183, 89046, 112978, 143342, 181867, 230746,
    292762, 371446, 471277, 597939, 758642, 962537, 1221231, 1549453, 1965888, 2494245,
    3164605, 4015133, 5094251, 6463396, 8200516, 10404510, 13200855, 16748753, 21250193, 26961454,
    34207688, 43401440, 55066131, 69865856, 88643195, 112467184, 142694174, 181045052, 229703219, 291438887,
    369766804, 469146347, 595235410, 755212515
};

uint32 QpToLambdaScaled(const int Qp)
{
    // pow(2.718,  (Qp - 13.7122)/4.2005)

    //int QpTerm = (((Qp<<PREC_SCALE_BITS) - (int64)QP_LAMBMA_FORMULA_ADD)<<PREC_SCALE_BITS)/QP_LAMBMA_FORMULA_MUL;
    //return (uint32) fExp(QpTerm);
    int QpChecked = Qp;
    if(QpChecked < 0)
        QpChecked = 0;
    else if(QpChecked > 53)
        QpChecked = 53;
    return g_auiLambdaByQp[Qp];
}

//uint32 LambdaToQp(double lambda)
//{
//  return (uint32)( 4.2005 * log( lambda ) + 13.7122 + 0.5 );
//}
uint32 LambdaScaledToQp(const uint32 lambda_SCALED)
{
    int64 RoundAdd = 1 << (PREC_SCALE_BITS - 1);
    int64 qp_lambda_add = QP_LAMBMA_FORMULA_ADD;
    int64 forula_mul_log_term = (((int64)QP_LAMBMA_FORMULA_MUL) * MhefLog(lambda_SCALED)) >> PREC_SCALE_BITS;
    return (uint32)((forula_mul_log_term + qp_lambda_add + RoundAdd) >> PREC_SCALE_BITS);
}
