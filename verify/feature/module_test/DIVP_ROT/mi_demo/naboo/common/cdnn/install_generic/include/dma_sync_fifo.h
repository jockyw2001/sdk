/*
 * dma_sync_fifo.h
 *
 *  Created on: Feb 2016
 *      Author: yurys
 */

#ifndef INCLUDE_DMA_SYNC_FIFO_H_
#define INCLUDE_DMA_SYNC_FIFO_H_

#include "dma_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

dma_status_e dma_sync_message_fifo_init(void);

#ifndef CEVAXM
extern int register_dma_isr(void(*cb)(void));
#endif //!defined CEVAXM

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DMA_SYNC_FIFO_H_ */
