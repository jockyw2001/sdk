/*
 * mhe_control.c
 *
 *  Created on: Nov 29, 2017
 *      Author: derek.lee
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <mdrv_msys_io.h>
#include <mdrv_verchk.h>
#include <mdrv_msys_io_st.h>

#include <mdrv_rqct_io.h>
#include <mdrv_mhe_io.h>
#include <mdrv_pmbr_io.h>

#include "cam_os_wrapper.h"

#include "mhe_utility.h"
#include "mhe_control.h"

#define CBR_UPPER_QP        51  //48
#define CBR_LOWER_QP        12

MHE_ERROR _SetMheVecBasicSettings(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_parm param;

    /*resolution*/
    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_RES;

    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param))
        return MheSetError;

    param.res.i_pict_w = pParam->basic.w;
    param.res.i_pict_h = pParam->basic.h;
    param.res.i_pixfmt = MMHE_PIXFMT_NV12;
    param.res.i_outlen = -1;
    param.res.i_flags = 0;

    /*clock*/
    if(pParam->sClock.bEn)
    {
        param.res.i_ClkEn = pParam->sClock.nClkEn;
        param.res.i_ClkSor = pParam->sClock.nClkSor;
    }

    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    /*fps*/
    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_FPS;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param))
        return MheSetError;

    param.fps.i_num = pParam->basic.fps;
    param.fps.i_den = 1;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    /*gop*/
    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_GOP;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param))
        return MheSetError;

    param.gop.i_pframes = pParam->basic.Gop;
    param.gop.i_bframes = 0;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    /*bps*/
    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_BPS;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param))
        return MheSetError;

    param.bps.i_method = pParam->sRC.eControlRate;
    param.bps.i_bps  = pParam->basic.bitrate;
    param.bps.i_ref_qp = pParam->basic.nQp;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    return err;
}

MHE_ERROR _SetMheVecH265Dblk(Mheparam* pParam)
{

    MHE_ERROR err = MheErrorNone;
#if 1
    mmhe_parm param;

    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_HEVC;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_HEVC Error = %x\n", __func__, err);
    }

    //param.avc.i_poc_type = pParam->sDisposable.bDisposable ? 0 : pParam->sPoc.nPocType;
    //param.avc.b_cabac = pParam->sEntropy.bEntropyCodingCABAC;
    //param.avc.i_profile = MMHE_AVC_PROFILE_MP;
    //param.avc.b_constrained_intra_pred = pParam->sIntraP.bconstIpred;
    param.hevc.b_deblocking_cross_slice_enable = pParam->sDeblk.b_deblocking_cross_slice_enable;
    param.hevc.b_deblocking_disable = pParam->sDeblk.b_deblocking_disable;
    param.hevc.i_tc_offset_div2 = pParam->sDeblk.slice_tc_offset_div2;
    param.hevc.i_beta_offset_div2 = pParam->sDeblk.slice_beta_offset_div2;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;
#endif
    return err;
}

MHE_ERROR _SetMheVecH265Disposable(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    return err;
}

MHE_ERROR _SetMheVecH265PocType(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    printf("%s not support\n", __func__);
#if 0
    mmhe_parm param;

    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_AVC;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_AVC Error = %x\n", __func__, err);
    }

    param.avc.i_poc_type = pParam->sPoc.nPocType;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;
#endif
    return err;
}

MHE_ERROR _SetMheVecH265Trans(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    printf("%s not support\n", __func__);


    mmhe_parm param;
    memset(&param, 0, sizeof(mmhe_parm));

    param.type = MMHE_PARM_HEVC;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_HEVC Error = %x\n", __func__, err);
    }

    param.hevc.i_cqp_offset = pParam->sTrans.nQpOffset;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    return err;
}

MHE_ERROR _SetMheVecH265Gop(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_parm param;

    param.type = MMHE_PARM_GOP;
    param.gop.i_pframes = pParam->basic.Gop;
    param.gop.i_bframes = 0;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    return err;
}



MHE_ERROR _SetMheVecH265RateControl(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_parm param;
    rqct_conf rqcnf;

    memset(&param, 0, sizeof(mmhe_parm));
    memset(&rqcnf, 0, sizeof(rqct_conf));

    param.type = MMHE_PARM_BPS;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param))
        return MheSetError;


    param.bps.i_method = pParam->sRC.eControlRate;
    param.bps.i_bps  = pParam->basic.bitrate;
    param.bps.i_ref_qp = pParam->basic.nQp;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    //Max/Min QP setting for VBR
    if(param.bps.i_method == Mhe_RQCT_METHOD_VBR)
    {
        rqcnf.type = RQCT_CONF_QPR;
        rqcnf.qpr.i_iupperq = pParam->sRC.nImaxqp;
        rqcnf.qpr.i_ilowerq = pParam->sRC.nIminqp;
        rqcnf.qpr.i_pupperq = pParam->sRC.nPmaxqp;
        rqcnf.qpr.i_plowerq = pParam->sRC.nPminqp;
        err = ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf);
        return MheSetError;

        rqcnf.type = RQCT_CONF_SEQ;
        rqcnf.seq.i_method = Mhe_RQCT_METHOD_VBR;
        rqcnf.seq.i_period = pParam->basic.Gop;
        rqcnf.seq.i_leadqp = -1;
        rqcnf.seq.i_btrate = pParam->basic.bitrate;
        err = ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf);
        return MheSetError;
    }
    //Max/Min QP setting for CBR
    else if(param.bps.i_method == Mhe_RQCT_METHOD_CBR)
    {


        rqcnf.type = RQCT_CONF_QPR;
        rqcnf.qpr.i_iupperq = CBR_UPPER_QP;
        rqcnf.qpr.i_ilowerq = CBR_LOWER_QP;
        rqcnf.qpr.i_pupperq = CBR_UPPER_QP;
        rqcnf.qpr.i_plowerq = CBR_LOWER_QP;
        err = ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf);
        return MheSetError;

        rqcnf.type = RQCT_CONF_SEQ;
        rqcnf.seq.i_method = Mhe_RQCT_METHOD_CBR;
        rqcnf.seq.i_period = pParam->basic.Gop;
        rqcnf.seq.i_leadqp = -1;
        rqcnf.seq.i_btrate = pParam->basic.bitrate;
        err = ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf);
        return MheSetError;
    }

    rqcnf.type = RQCT_CONF_LOG;
    if(ioctl(pParam->mhefd, IOCTL_RQCT_G_CONF, &rqcnf))
        return MheSetError;
    rqcnf.log.b_logm = 1;
    if(ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf))
        return MheSetError;
    /*
       rqcnf.type = RQCT_CONF_SEQ;
        rqcnf.seq.i_method = pParam->sRC.eControlRate;
        rqcnf.seq.i_period = 25;
        rqcnf.seq.i_leadqp = pParam->sRC.nRefQp;
        rqcnf.seq.i_btrate = pParam->sRC.nTargetBitrate;
        if (ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf))
            return MheSetError;*/

    return err;
}

MHE_ERROR _SetMheVecH265IntraPred(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    mmhe_parm param;
    rqct_conf rqcnf;

    memset(&param, 0, sizeof(mmhe_parm));
    memset(&rqcnf, 0, sizeof(rqct_conf));

    param.type = MMHE_PARM_HEVC;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_HEVC Error = %x\n", __func__, err);
    }

    param.hevc.b_constrained_intra_pred = pParam->sIntraP.bconstIpred;

    if(ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param))
        return MheSetError;

    rqcnf.type = RQCT_CONF_PEN;
    err = ioctl(pParam->mhefd, IOCTL_RQCT_G_CONF, &rqcnf);
    if(err)
    {
        printf("%s Get-RQCT_CONF_PEN Error = %x\n", __func__, err);
    }

    rqcnf.pen.b_ia8xlose = pParam->sIntraP.bIntraCu8Lose;
    rqcnf.pen.b_ir8xlose = pParam->sIntraP.bInterCu8Lose;
    rqcnf.pen.b_ia16lose = pParam->sIntraP.bIntraCu16Lose;
    rqcnf.pen.b_ir16lose = pParam->sIntraP.bInterCu16Lose;
    rqcnf.pen.b_ir16mlos = pParam->sIntraP.bInterCu16MergeLose;
    rqcnf.pen.b_ir16slos = pParam->sIntraP.bInterCu16SkipLose;
    rqcnf.pen.b_ir16mslos = pParam->sIntraP.bInterCu16MergeSkipLose;
    rqcnf.pen.b_ia32lose = pParam->sIntraP.bIntraCu32Lose;
    rqcnf.pen.b_ir32mlos = pParam->sIntraP.bInterCu32MergeLose;
    rqcnf.pen.b_ir32mslos = pParam->sIntraP.bInterCu32MergeSkipLose;

    rqcnf.pen.u_ia8xpen = pParam->sIntraP.nIntraCu8Pen;
    rqcnf.pen.u_ir8xpen = pParam->sIntraP.nInterCu8Pen;
    rqcnf.pen.u_ia16pen = pParam->sIntraP.nIntraCu16Pen;
    rqcnf.pen.u_ir16pen = pParam->sIntraP.nInterCu16Pen;
    rqcnf.pen.u_ir16mpen = pParam->sIntraP.nInterCu16MergePen;
    rqcnf.pen.u_ir16spen = pParam->sIntraP.nInterCu16SkipPen;
    rqcnf.pen.u_ir16mspen = pParam->sIntraP.nInterCu16MergeSkipPen;
    rqcnf.pen.u_ia32pen = pParam->sIntraP.nIntra32Pen;
    rqcnf.pen.u_ir32mpen = pParam->sIntraP.nIntra32MergePen;
    rqcnf.pen.u_ir32mspen = pParam->sIntraP.nIntra32MergeSkipPen;

    if(ioctl(pParam->mhefd, IOCTL_RQCT_S_CONF, &rqcnf))
        return MheSetError;

    return err;
}

MHE_ERROR _SetMheVecH265InterPred(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    mmhe_parm param;

    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_MOT;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_MOT Error = %x\n", __func__, err);
    }
    param.mot.i_dmv_x = pParam->sInterP.nDmv_X;
    param.mot.i_dmv_y = pParam->sInterP.nDmv_Y;
    param.mot.i_subpel = pParam->sInterP.nSubpel;
    param.mot.i_mvblks[0] = 0;
    param.mot.i_mvblks[1] = 0;
    if(TRUE == pParam->sInterP.bInter4x4PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_4x4;
    }
    if(TRUE == pParam->sInterP.bInter8x4PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_8x4;
    }
    if(TRUE == pParam->sInterP.bInter4x8PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_4x8;
    }
    if(TRUE == pParam->sInterP.bInter8x8PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_8x8;
    }
    if(TRUE == pParam->sInterP.bInter16x8PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_16x8;
    }
    if(TRUE == pParam->sInterP.bInter8x16PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_8x16;
    }
    if(TRUE == pParam->sInterP.bInter16x16PredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_16x16;
    }
    if(TRUE == pParam->sInterP.bInterSKIPPredEn)
    {
        param.mot.i_mvblks[0] |= MMHE_MVBLK_SKIP;
    }
    err = ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param);
    if(err)
    {
        printf("%s Set-MMHE_PARM_MOT Error = %x\n", __func__, err);
    }

    return err;
}

MHE_ERROR _SetMheVecH265SliceSplit(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_ctrl vctrl;

    memset(&vctrl, 0, sizeof(mmhe_ctrl));

    vctrl.type = MMHE_CTRL_SPL;
    if(ioctl(pParam->mhefd, IOCTL_MMHE_G_CTRL, &vctrl))
        return MheSetError;

    vctrl.spl.i_rows =  pParam->sMSlice.nRows;
    vctrl.spl.i_bits = pParam->sMSlice.nBits;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_S_CTRL, &vctrl);
    if(err)
    {
        printf("%s -MMHE_CTRL_SPL Error = %x\n", __func__, err);
    }

    return err;
}

MHE_ERROR _SetMheVecH265ROI(Mheparam* pParam, int idx)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_ctrl vctrl;

    u32 nRoiRegion = pParam->sRoi[idx].nMbW * pParam->sRoi[idx].nMbH;
    u32 nFramePixel = pParam->sNewSeq.nWidth * pParam->sNewSeq.nHeight;


    //if(TRUE == pParam->bChangedROIConfig[idx])
    {
        memset(&vctrl, 0, sizeof(mmhe_ctrl));
        vctrl.type = MMHE_CTRL_ROI;
        vctrl.roi.i_index = pParam->sRoi[idx].nIdx;
        if(TRUE == pParam->sRoi[idx].bEnable)
        {
            vctrl.roi.i_roiqp = pParam->sRoi[idx].nMbqp;
        }
        else
        {
            vctrl.roi.i_roiqp = 0;
        }
        if(pParam->sRoi[idx].nMbX > pParam->sNewSeq.nWidth ||
                pParam->sRoi[idx].nMbY > pParam->sNewSeq.nHeight ||
                pParam->sRoi[idx].nMbW == 0 ||
                pParam->sRoi[idx].nMbH == 0 )//||
                //(pParam->sRoi[idx].nMbqp + 15) > 30)
        {
            vctrl.roi.i_roiqp = pParam->sRoi[idx].nMbqp = 0;
            vctrl.roi.i_cbx = pParam->sRoi[idx].nMbX = 0;
            vctrl.roi.i_cby = pParam->sRoi[idx].nMbY = 0;
            vctrl.roi.i_cbw = pParam->sRoi[idx].nMbW = 0;
            vctrl.roi.i_cbh = pParam->sRoi[idx].nMbH = 0;
            err = ioctl(pParam->mhefd, IOCTL_MMHE_S_CTRL, &vctrl);
            if(err)
            {
                printf("%s -MMHE_CTRL_ROI Error = %x\n", __func__, err);
            }
            //pParam->bChangedROIConfig[i] = OMX_FALSE;
        }
        else
        {
            printf("- (%d , %d) \n", pParam->sNewSeq.nWidth, pParam->sNewSeq.nHeight);
            printf("-Idx: %u Enable: %d (X,Y)->(%u,%u) (W,H)->(%u,%u) Qp:%d Abs:%d\n", pParam->sRoi[idx].nIdx, pParam->sRoi[idx].bEnable,
                   pParam->sRoi[idx].nMbX, pParam->sRoi[idx].nMbY, pParam->sRoi[idx].nMbW, pParam->sRoi[idx].nMbH, pParam->sRoi[idx].nMbqp, pParam->sRoi[idx].bAbsQp);


            if(pParam->sNewSeq.nWidth < (pParam->sRoi[idx].nMbX + pParam->sRoi[idx].nMbW))
                //pParam->sRoi[idx].nMbW = pParam->sNewSeq.nWidth - pParam->sRoi[idx].nMbX;
                pParam->sRoi[idx].nMbW = pParam->basic.w - pParam->sRoi[idx].nMbX;
            if(pParam->sNewSeq.nHeight < (pParam->sRoi[idx].nMbY + pParam->sRoi[idx].nMbH))
                pParam->sRoi[idx].nMbH = pParam->basic.h - pParam->sRoi[idx].nMbY;
            vctrl.roi.i_cbx = pParam->sRoi[idx].nMbX / 16;
            vctrl.roi.i_cby = pParam->sRoi[idx].nMbY / 16;
            vctrl.roi.i_cbw = (pParam->sRoi[idx].nMbW + 15) / 16;
            vctrl.roi.i_cbh = (pParam->sRoi[idx].nMbH + 15) / 16;
            vctrl.roi.i_roiqp -= (((double)nRoiRegion) / ((double)nFramePixel)) * vctrl.roi.i_roiqp;
            //vctrl.roi.i_dqp = 2;
            printf("--> index %d, (%d %d %d %d), roiqp %d\n", vctrl.roi.i_index, vctrl.roi.i_cbx, vctrl.roi.i_cby, vctrl.roi.i_cbw, vctrl.roi.i_cbh, vctrl.roi.i_roiqp);
            err = ioctl(pParam->mhefd, IOCTL_MMHE_S_CTRL, &vctrl);
            if(err)
            {
                printf("%s -MMHE_CTRL_ROI Error = %x\n", __func__, err);
            }

            //pParam->bChangedROIConfig[i] = FALSE;
        }
    }

    return err;
}


MHE_ERROR _SetMheVecH265InsertUserData(Mheparam* pParam, int idx)
{
    MHE_ERROR err = MheErrorNone;

    mmhe_buff *pubuf;
    mmhe_buff vbuff;

    pubuf = &vbuff;

    memset(pubuf, 0, sizeof(mmhe_buff));

    pubuf->i_memory = MMHE_MEMORY_USER;
    pubuf->i_planes = 1;
    pubuf->planes[0].i_size = 1024;

    if(pParam->sSEI[idx].u32Len > 0)
    {
        pubuf->planes[0].mem.uptr = pParam->sSEI[idx].U8data;
        pubuf->planes[0].i_used = pParam->sSEI[idx].u32Len;
        ioctl(pParam->mhefd, IOCTL_MMHE_S_DATA, pubuf);
        if(err)
        {
            printf("%s -IOCTL_MMHE_S_DATA Error = %x\n", __func__, err);
        }
    }


    return err;
}

MHE_ERROR _SetMheVecH265NewSeq(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

    mmhe_ctrl vctrl;
    //u32 i =0;
    memset(&vctrl, 0, sizeof(mmhe_ctrl));
    vctrl.type = MMHE_CTRL_SEQ;
    vctrl.seq.i_pixfmt = MMHE_PIXFMT_NV12;
    vctrl.seq.i_pixelw = (int)pParam->sNewSeq.nWidth;
    vctrl.seq.i_pixelh = (int)pParam->sNewSeq.nHeight;
    vctrl.seq.n_fps = pParam->sNewSeq.xFramerate >> 6;
    vctrl.seq.d_fps = 1024;
    if(vctrl.seq.n_fps > 0 && vctrl.seq.d_fps > 0)
    {
        err = ioctl(pParam->mhefd, IOCTL_MMHE_S_CTRL, &vctrl);
        if(err)
        {
            printf("%s -MMHE_CTRL_SEQ Error = %x\n", __func__, err);
        }
    }
    else
    {
        printf("%s -MMHE_CTRL_SEQ Error = %x\n", __func__, err);
    }

    return err;
}

MHE_ERROR _SetMheVecH265VUI(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;
    mmhe_parm param;

    memset(&param, 0, sizeof(mmhe_parm));
    param.type = MMHE_PARM_VUI;
    err = ioctl(pParam->mhefd, IOCTL_MMHE_G_PARM, &param);
    if(err)
    {
        printf("%s Get-MMHE_PARM_VUI Error = %x\n", __func__, err);
    }

    param.vui.i_sar_w = pParam->sVUI.u16SarWidth;
    param.vui.i_sar_h = pParam->sVUI.u16SarHeight;
    param.vui.b_video_full_range = pParam->sVUI.u8VideoFullRangeFlag;
    param.vui.b_video_signal_pres = pParam->sVUI.u8VideoSignalTypePresentFlag;
    param.vui.i_video_format = pParam->sVUI.u8VideoFormat;
    param.vui.b_colour_desc_pres = pParam->sVUI.u8ColourDescriptionPresentFlag;
    param.vui.b_timing_info_pres = pParam->sVUI.u8TimingInfoPresentFlag;

    err = ioctl(pParam->mhefd, IOCTL_MMHE_S_PARM, &param);
    if(err)
    {
        printf("%s -MMHE_PARM_VUI Error = %x\n", __func__, err);
    }

    return err;
}

MHE_ERROR _SetMheVecH265PMBR(Mheparam* pParam)
{
    MHE_ERROR err = MheErrorNone;

#if 1   //for PMBR, not ready
    H265RcRMBR pmbrcfg;


    if(pParam->sPMBR.tc.bEn)
    {
        memset(&pmbrcfg, 0 , sizeof(pmbrcfg));
        pmbrcfg.type = DRV_PMBR_CFG_TC;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_G_CONF, &pmbrcfg))
            return MheSetError;

        pmbrcfg.tc.i_LutEntryClipRange = pParam->sPMBR.tc.i_LutEntryClipRange;
        pmbrcfg.tc.i_TextWeightType = pParam->sPMBR.tc.i_TextWeightType;
        pmbrcfg.tc.i_SkinLvShift = pParam->sPMBR.tc.i_SkinLvShift;
        pmbrcfg.tc.i_WeightOffset = pParam->sPMBR.tc.i_WeightOffset;
        pmbrcfg.tc.i_TcOffset = pParam->sPMBR.tc.i_TcOffset;
        pmbrcfg.tc.i_TcToSkinAlpha = pParam->sPMBR.tc.i_TcToSkinAlpha;
        pmbrcfg.tc.i_TcGradThr = pParam->sPMBR.tc.i_TcGradThr;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_S_CONF, &pmbrcfg))
            return MheSetError;
    }

    if(pParam->sPMBR.pc.bEn)
    {
        memset(&pmbrcfg, 0 , sizeof(pmbrcfg));
        pmbrcfg.type = DRV_PMBR_CFG_PC;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_G_CONF, &pmbrcfg))
            return MheSetError;

        pmbrcfg.pc.i_PwYMax = pParam->sPMBR.pc.i_PwYMax;
        pmbrcfg.pc.i_PwYMin = pParam->sPMBR.pc.i_PwYMin;
        pmbrcfg.pc.i_PwCbMax = pParam->sPMBR.pc.i_PwCbMax;
        pmbrcfg.pc.i_PwCbMin = pParam->sPMBR.pc.i_PwCbMin;
        pmbrcfg.pc.i_PwCrMax = pParam->sPMBR.pc.i_PwCrMax;
        pmbrcfg.pc.i_PwCrMin = pParam->sPMBR.pc.i_PwCrMin;
        pmbrcfg.pc.i_PwCbPlusCrMin = pParam->sPMBR.pc.i_PwCbPlusCrMin;
        pmbrcfg.pc.i_PwAddConditionEn = pParam->sPMBR.pc.i_PwAddConditionEn;
        pmbrcfg.pc.i_PwCrOffset = pParam->sPMBR.pc.i_PwCrOffset;
        pmbrcfg.pc.i_PwCbCrOffset = pParam->sPMBR.pc.i_PwCbCrOffset;
        pmbrcfg.pc.i_PcDeadZone = pParam->sPMBR.pc.i_PcDeadZone;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_S_CONF, &pmbrcfg))
            return MheSetError;
    }


    if(pParam->sPMBR.ss.bEn)
    {
        memset(&pmbrcfg, 0 , sizeof(pmbrcfg));
        pmbrcfg.type = DRV_PMBR_CFG_SS;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_G_CONF, &pmbrcfg))
            return MheSetError;

        pmbrcfg.ss.i_SmoothEn = pParam->sPMBR.ss.i_SmoothEn;
        pmbrcfg.ss.i_SmoothClipMax = pParam->sPMBR.ss.i_SmoothClipMax;
        pmbrcfg.ss.i_SecStgAlpha = pParam->sPMBR.ss.i_SecStgAlpha;
        pmbrcfg.ss.i_SecStgBitWghtOffset = pParam->sPMBR.ss.i_SecStgBitWghtOffset;
        if(ioctl(pParam->mhefd, IOCTL_PMBR_S_CONF, &pmbrcfg))
            return MheSetError;
    }
#endif

    return err;
}



int SetMheParameter(Mheparam* pParam)
{
    int i = 0;
    MHE_ERROR err = MheErrorNone;

    if(!pParam->mhefd)
        return MheFdError;

    //basick settings
    if(pParam->basic.bEn)
    {
        err = _SetMheVecBasicSettings(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 deblocking
    if(pParam->sDeblk.bEn)
    {
        err = _SetMheVecH265Dblk(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 disposable
    if(pParam->sDisposable.bEn)
    {
        err = _SetMheVecH265Disposable(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 Poc
    if(pParam->sPoc.bEn)
    {
        err = _SetMheVecH265PocType(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 Entropy
    if(pParam->sTrans.bEn)
    {
        err = _SetMheVecH265Trans(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 Rate Control
    if(pParam->sRC.bEn)
    {
        err = _SetMheVecH265RateControl(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 Intra pred
    if(pParam->sIntraP.bEn)
    {
        err = _SetMheVecH265IntraPred(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 Inter pred
    if(pParam->sInterP.bEn)
    {
        err = _SetMheVecH265InterPred(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 VUI
    if(pParam->sVUI.bEn)
    {
        err = _SetMheVecH265VUI(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 InsertUserData
    for(i = 0; i < MAX_SEI_NUM; i++)
    {
        if(pParam->sSEI[i].bEn)
        {
            err = _SetMheVecH265InsertUserData(pParam, i);
            if(MheErrorNone != err)
            {
                goto MheErr;
            }
        }
    }

    //h265 PMBR
    if(pParam->sPMBR.tc.bEn
            || pParam->sPMBR.pc.bEn
            || pParam->sPMBR.ss.bEn)
    {
        err = _SetMheVecH265PMBR(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

MheErr:
    return err;

}

int SetMheControl(Mheparam* pParam)
{
    int i = 0;
    MHE_ERROR err = MheErrorNone;

    //h265 M-Slice
    if(pParam->sMSlice.bEn)
    {
        err = _SetMheVecH265SliceSplit(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

    //h265 ROI
    for(i = 0; i < MAX_ROI_NUM; i++)
    {
        if(pParam->sRoi[i].bEn)
        {
            err = _SetMheVecH265ROI(pParam, i);
            if(MheErrorNone != err)
            {
                goto MheErr;
            }
        }
    }

    //h265 NewSeq
    if(pParam->sNewSeq.bEn)
    {
        err = _SetMheVecH265NewSeq(pParam);
        if(MheErrorNone != err)
        {
            goto MheErr;
        }
    }

MheErr:
    return err;
}
