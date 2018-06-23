#include "hal_mfe_msmath.h"
#include "hal_mfe_global.h"



int QP2Qstep_SCALED( int QP )
{
    int iter;
    int Qstep_SCALED;
    const int QP2QSTEP_SCALED[6] = { 20, 22, 26, 28, 32, 36 };

    Qstep_SCALED = QP2QSTEP_SCALED[QP % 6];
    for (iter = 0; iter < (QP / 6); iter++)
    {
        Qstep_SCALED <<= 1;
    }
    return Qstep_SCALED;
}
/*!
 *************************************************************************************
 * \brief
 *    map Qstep to QP
 *
 *************************************************************************************
*/

// * input 5bit frac --> intermidiante 12bit
int Qstep_12bitSCALED2QP( int Qstep_SCALED )
{
  int q_per = 0, q_rem = 0;

  if(Qstep_SCALED < 2560) // QP2Qstep(0) = 0.625
    return 0;
  else if (Qstep_SCALED > 917504) // QP2Qstep(51) = 224
    return 51;


  while(Qstep_SCALED > 4608) // QP2Qstep(5) = 1.125
  {
    Qstep_SCALED >>= 1;
    q_per += 1;
  }

  if (Qstep_SCALED <= 2688)
  {
    Qstep_SCALED = 2560;
    q_rem = 0;
  }
  else if (Qstep_SCALED <= 3072)
  {
    Qstep_SCALED = 2816;
    q_rem = 1;
  }
  else if (Qstep_SCALED <= 3456)
  {
    Qstep_SCALED = 3328;
    q_rem = 2;
  }
  else if (Qstep_SCALED <= 3840)
  {
    Qstep_SCALED = 3584;
    q_rem = 3;
  }
  else if (Qstep_SCALED <= 4352)
  {
    Qstep_SCALED = 4096;
    q_rem = 4;
  }
  else
  {
    Qstep_SCALED = 4608;
    q_rem = 5;
  }

  return (q_per * 6 + q_rem);
}
