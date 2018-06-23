
#include "drv_mfe.h"
#include "drv_mfe_kernel.h"
#include "drv_mfe_module.h"
#include "mdrv_mfe_io.h"
#include "mdrv_rqct_io.h"
#include "mhve_ios.h"
#include "drv_mfe_ctx.h"
#include "drv_mfe_dev.h"

/*=============================================================*/
// Global Macro definition
/*=============================================================*/
#define ALIGN256(x) (((x)+0xFF)&~0xFF)
#define MAX(a,b)    ((a)>(b)?(a):(b))
#define MIN(a,b)    ((a)<(b)?(a):(b))
#define CLP(a,b,x)  (MIN(MAX(x,a),b))

/*=============================================================*/
// Global Variable definition
/*=============================================================*/
mmfe_dev* mdev = NULL;

/*=============================================================*/
// Function Declaration
/*=============================================================*/
void MfeEnc(Mfe_param* pParam)
{
    /* validate params */
    int pictw = pParam->sBasic.nWidth;
    int picth = pParam->sBasic.nHeight;
    int length = pictw * picth;
    int picture = length + length / 2;
    if(picth % 16 != 0)picth = picth + picth % 16;
    int ysize = pictw * picth;
    int csize = ysize / 2;
    int picts = ysize + csize;
    int sqp = CLP(12, 48, pParam->sBasic.nQp);
    int bps = pParam->sBasic.nBitrate;
    int fps = pParam->sBasic.nFps;
    int gop = pParam->sBasic.nGop;
    int disposable = 0, cabac = 0, spl = 0;
    int pixfm = MMFE_PIXFMT_NV12;
    int dmvx = 32, dmvy = 16;
    mmfe_parm param;
    mmfe_ctrl cntrl;
    mmfe_buff* ibuff;
    mmfe_buff* obuff;
    mmfe_buff vbuff[2];
    rqct_conf rqcnf;
    int frame = 0;
    int count = 0;
    int total = 0;
    int fsize = 0;
    /* input & output BUFF */
    void* ibuff_kvirt;
    u64 ibuff_phys;
    u64 ibuff_base;
    void* obuff_kvirt;
    u64 obuff_phys;
    u64 obuff_base;
    /* input & output FILE */
    void* iFile_virt;
    u64 iFile_phys;
    u64 iFile_base;
    void* oFile_virt;
    u64 oFile_phys;
    u64 oFile_base;

    ibuff = &vbuff[0];
    obuff = &vbuff[1];

    int frm_num = 0;
    if(pictw == 1920) frm_num = 10;
    else frm_num = 50;
    CamOsDirectMemAlloc("INPUT-FILE", picts * frm_num, &iFile_virt, &iFile_base, &iFile_phys);
    CamOsDirectMemAlloc("OUTPUT-FILE", length * frm_num/2, &oFile_virt, &oFile_base, &oFile_phys);
    memset(iFile_virt, 0, picts * frm_num);
    memset(oFile_virt, 0, length * frm_num/2);
    UartSendTrace(" Input File Address : 0x%08x \n", (unsigned int)iFile_phys);
    UartSendTrace(" Output File Address : 0x%08x \n", (unsigned int)oFile_phys);

    UartSendTrace("Encode frame number : (FHD -> 10, VGA or CIF -> 50)\n");
    count = vm_atoi(wait_for_command());
    do
    {
        mdev = MfeProbe(mdev);
        //mdev->i_rctidx = pParam->sRC.eControlRate;
        mmfe_ctx* mctx = NULL;
        mctx = MfeOpen(mdev, mctx);
        if((mdev == NULL) || (mctx == NULL))
        {
            UartSendTrace("> Create Drv Fail \n");
            break;
        }
        do
        {
            /* now, check arguments */
            if ((pictw % 16) != 0 || (picth % 8) != 0)
            {
                UartSendTrace("> Resolution Setting Fail \n");
                break;
            }
            /* request direct-memory */
            CamOsDirectMemAlloc("INPUT-BUF", picts, &ibuff_kvirt, &ibuff_base, &ibuff_phys);
            CamOsDirectMemAlloc("OUTPUT-BUF", length, &obuff_kvirt, &obuff_base, &obuff_phys);

            memset(ibuff_kvirt, 128, picts);
            memset(obuff_kvirt, 0, length);
            memset(&vbuff, 0, sizeof(vbuff));

            /* buffer setting */
            ibuff->i_timecode = 0;
            ibuff->i_width = pictw;
            ibuff->i_height = picth;
            ibuff->i_memory = MMFE_MEMORY_MMAP;
            ibuff->i_planes = 2;
            ibuff->planes[0].mem.phys = ibuff_phys;
            ibuff->planes[1].mem.phys = ibuff_phys + ysize;
            ibuff->planes[0].i_size = ysize;
            ibuff->planes[1].i_size = csize;

            obuff->i_memory = MMFE_MEMORY_MMAP;
            obuff->i_planes = 1;
            obuff->planes[0].mem.phys = obuff_phys;
            obuff->planes[0].i_size = length;
            obuff->planes[0].i_used = 0;
            obuff->i_flags = 0;

            /* set parameters */
            param.type = MMFE_PARM_RES;
            param.res.i_pict_w = pictw;
            param.res.i_pict_h = picth;
            param.res.i_pixfmt = pixfm;
            param.res.i_outlen = -1;
            param.res.i_flags = 0;
            param.res.i_ClkEn = pParam->sClock.nClkEn;
            param.res.i_ClkSor = pParam->sClock.nClkSor;
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_RES Set Fail \n");
                break;
            }
            param.type = MMFE_PARM_FPS;
            param.fps.i_num = fps;
            param.fps.i_den = 1;
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_FPS Set Fail \n");
                break;
            }
            param.type = MMFE_PARM_MOT;
            if (MfeCtxActions(mctx, IOCTL_MMFE_G_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_MOT Get Fail \n");
                break;
            }
            param.mot.i_dmv_x = dmvx;
            param.mot.i_dmv_y = dmvy;
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_MOT Set Fail \n");
                break;
            }
            param.type = MMFE_PARM_AVC;
            if (MfeCtxActions(mctx, IOCTL_MMFE_G_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_AVC Get Fail \n");
                break;
            }
            if (disposable > 0)
                param.avc.i_poc_type = 0;
            param.avc.b_cabac_i = 0;
            param.avc.b_cabac_p = 0;
            param.avc.i_profile = MMFE_AVC_PROFILE_BP;
            if (cabac)
            {
                param.avc.b_cabac_i = 1;
                param.avc.b_cabac_p = 1;
                param.avc.i_profile = MMFE_AVC_PROFILE_MP;
            }
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_PARM, &param))
            {
                UartSendTrace("> MMFE_PARM_AVC Set Fail \n");
                break;
            }
            /* streaming loop */
            if (MfeCtxActions(mctx, IOCTL_MMFE_STREAMON, &param))
            {
                UartSendTrace("> IOCTL_MMFE_STREAMON Fail \n");
                break;
            }
            rqcnf.type = RQCT_CONF_SEQ;
            rqcnf.seq.i_method = !bps ? RQCT_METHOD_CQP : RQCT_METHOD_CBR;
            rqcnf.seq.i_period = gop;
            rqcnf.seq.i_leadqp = sqp;
            rqcnf.seq.i_btrate = bps;
            if (MfeCtxActions(mctx, IOCTL_RQCT_S_CONF, &rqcnf))
            {
                UartSendTrace("> RQCT_CONF_SEQ Set Fail \n");
                break;
            }

            cntrl.type = MMFE_CTRL_SPL;
            cntrl.spl.i_rows = spl;
            cntrl.spl.i_bits = 0;
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_CTRL, &cntrl))
            {
                UartSendTrace("> MMFE_CTRL_SPL Set Fail \n");
                break;
            }
#if 0
            cntrl.type = MMFE_CTRL_ROI;
            cntrl.roi.i_index = 0;
            cntrl.roi.i_dqp = -13;
            cntrl.roi.i_mbx = 0;
            cntrl.roi.i_mby = 0;
            cntrl.roi.i_mbw = 160/16;
            cntrl.roi.i_mbh = 160/16;
            if (MfeCtxActions(mctx, IOCTL_MMFE_S_CTRL, &cntrl))
            {
                UartSendTrace("> MMFE_CTRL_ROI Set Fail \n");
                break;
            }
#endif
            while (frame < count)
            {
                memcpy(ibuff_kvirt, iFile_virt + (frame * picture), length);
                memcpy(ibuff_kvirt + ysize, iFile_virt + (frame * picture) + length, length / 2);

                ibuff->i_flags = disposable > 0 ? MMFE_FLAGS_DISPOSABLE : 0;
                ibuff->planes[0].i_used = ibuff->planes[0].i_size;
                ibuff->planes[1].i_used = ibuff->planes[1].i_size;
                obuff->planes[0].i_used = 0;
                MfeCtxActions(mctx, IOCTL_MMFE_ENCODE, vbuff);

                if (disposable < 2 || !(obuff->i_flags & MMFE_FLAGS_DISPOSABLE))
                    memcpy(oFile_virt + fsize, obuff_kvirt, obuff->planes[0].i_used);
                fsize += ALIGN256(obuff->planes[0].i_used);
                total += obuff->planes[0].i_used;

                UartSendTrace("frame:%d, size:%d \n", frame, obuff->planes[0].i_used);
                frame++;
            }
            MfeCtxActions(mctx, IOCTL_MMFE_STREAMOFF, &param);
            UartSendTrace("total-size:%8d\n", total);

            UartSendTrace(" Encode END (press any key to continue)\n");
            wait_for_command();

            if (ibuff_kvirt) CamOsDirectMemRelease(ibuff_kvirt, picts);
            if (obuff_kvirt) CamOsDirectMemRelease(obuff_kvirt, length);
            if (iFile_virt) CamOsDirectMemRelease(iFile_virt, picts * frm_num);
            if (oFile_virt) CamOsDirectMemRelease(oFile_virt, length * frm_num/2);
        }
        while (0);
        MfeRelease(mdev, mctx);
        MfeRemove(mdev);
    }
    while (0);
}
