
#ifndef _HAL_MHE_INIT_H_
#define _HAL_MHE_INIT_H_

#include "hal_mfe_def.h"

#define POWERSAVING_ENB(base) do{REGW(base,0x16)=0xFFFF;}while(0)
#define POWERSAVING_DIS(base) do{REGW(base,0x16)=0x0000;}while(0)

#endif /*_HAL_MHE_INIT_H_*/
