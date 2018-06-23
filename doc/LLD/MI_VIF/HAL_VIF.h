

/** @brief ARM to 8051 ring buffer binding.
@param[in] pstRingBuf physical address of ring buffer
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
@remark MI call this function to bind 8051 ring buffer
*/

u32 HalVifInit();
u32 HalVifDeinit();

u32 MHalVifDevSetConfig(u32 u32VifDev, HalVifDevCfg_t *pstDevAttr);
u32 MHalVifDevEnable(u32 u32VifDev);
u32 MHalVifDevDisable(u32 u32VifDev);

u32 MHalVifChnSetConfig(u32 u32VifChn, HalVifChnCfg_t *pstAttr);
u32 MHalVifChnEnable(u32 u32VifChn);
u32 MHalVifChnDisable(u32 u32VifChn);
u32 MHalVifChnQuery(u32 u32VifChn, HalVifChnStat_t *pstStat);

u32 MHalVifSubChnSetConfig(u32 u32VifChn, HalVifSubChnCfg_t *pstAttr);
u32 MHalVifSubChnEnable(u32 u32VifChn);
u32 MHalVifSubChnDisable(u32 u32VifChn);
u32 MHalVifSubChnQuery(u32 u32VifChn, HalVifChnStat_t *pstStat);

/** @brief Push new frame buffer into VIF wait queue
@param[in] u32VifChn VIF channel
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
u32 MHalVifQueueFrameBuffer(u32 u32VifChn, const VifRingBufElm_t *ptFbInfo);

/** @brief Query VIF for ready frame
@param[in] u32VifChn VIF channel
@param[out] pNumBuf number of frame ready
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
u32 MHalVifQueryFrames(u32 u32VifChn, U32 *pNumBuf);

/** @brief Receive ready frame from VIF quque
@param[in] u32VifChn VIF channel
@param[out] ptFbInfo output frame buffer
@retval 0 if successed or E_HAL_VIF_ERROR if error occurs.
*/
u32 MHalVifDequeueFrameBuffer(u32 u32VifChn, VifRingBufElm_t *ptFbInfo);
