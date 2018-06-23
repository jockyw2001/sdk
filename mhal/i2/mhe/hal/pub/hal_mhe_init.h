
#ifndef _HAL_MHE_INIT_H_
#define _HAL_MHE_INIT_H_

#include "hal_mhe_def.h"

#define POWERSAVING_ENB(base) do{REGW(base,0x12)=0xFFFF;REGW(base,0x13)=0x3;}while(0)
#define POWERSAVING_DIS(base) do{REGW(base,0x12)=0x0000;REGW(base,0x13)=0x0;}while(0)

#endif /*_HAL_MHE_INIT_H_*/
