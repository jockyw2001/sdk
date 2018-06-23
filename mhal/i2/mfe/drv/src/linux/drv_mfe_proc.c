#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include "hal_mfe_def.h"
#include "drv_mfe_kernel.h"
#include "mdrv_rqct_st.h"
#include "cam_os_wrapper.h"

typedef struct MfeDevList_t
{
    struct CamOsListHead_t  tList;
    mmfe_dev                *mdev;
} MfeDevList_t;

static CamOsMutex_t _gtMfeProcMutex;
static MfeDevList_t _gtMfeDevList;
static u32 _gMfeDevCnt = 0;
struct proc_dir_entry *_gpRootMfeDir;
static s32 nShowCtxInfoDevId = -1;
static s32 nShowCtxInfoCtxId = -1;
static s32 nShowRegInfoDevId = -1;

static int MfeProcShowDevInfo(struct seq_file *m, void *v)
{
    mhve_ops* mops;
    rqct_ops* rqct;
    rqct_cfg  rqcf_fps;
    rqct_cfg  rqcf_seq;
    rqct_cfg  rqcf_qpr;
    struct CamOsListHead_t *ptPos, *ptQ;
    MfeDevList_t* ptTmp;
    int nDevCnt = 0;
    int nCtxCnt = 0;

    CamOsMutexLock(&_gtMfeProcMutex);

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMfeDevList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MfeDevList_t, tList);

        if(ptTmp->mdev)
        {
            seq_printf(m, "MFE Dev%d (bank 0x%04X)\n", nDevCnt, ((u32)ptTmp->mdev->p_reg_base & 0x00FFFF00)>>9);
            seq_printf(m, "  Ctx     W     H     FrmCnt   RC    FPS   GOP  RC-Params\n");
            seq_printf(m, " -------------------------------------------------------------------------------\n");
            for (nCtxCnt=0; nCtxCnt<MMFE_STREAM_NR; nCtxCnt++)
            {
                if (ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx)
                {
                    mops = ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->p_handle;
                    rqct = mops->rqct_ops(mops);

                    rqcf_fps.type = RQCT_CFG_FPS;
                    rqct->get_rqcf(rqct, &rqcf_fps);

                    rqcf_seq.type = RQCT_CONF_SEQ;
                    rqct->get_rqcf(rqct, &rqcf_seq);

                    rqcf_qpr.type = RQCT_CFG_QPR;
                    rqct->get_rqcf(rqct, &rqcf_qpr);

                    switch (rqcf_seq.seq.i_method)
                    {
                    case RQCT_MODE_CQP:
                        seq_printf(m, "   %2d  %4d  %4d %10d  CQP  %2d.%02d  %4d  QP: %d\n",
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_index,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_w,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_h,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_enccnt,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)/100,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)%100,
                                    rqcf_seq.seq.i_period-1,
                                    rqcf_seq.seq.i_leadqp);
                        break;
                    case RQCT_MODE_CBR:
                        seq_printf(m, "   %2d  %4d  %4d %10d  CBR  %2d.%02d  %4d  BitRate: %d\n",
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_index,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_w,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_h,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_enccnt,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)/100,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)%100,
                                    rqcf_seq.seq.i_period-1,
                                    rqcf_seq.seq.i_btrate);
                        break;
                    case RQCT_MODE_VBR:
                        seq_printf(m, "   %2d  %4d  %4d %10d  VBR  %2d.%02d  %4d  MaxRate: %d  QP(Max/Min): %d/%d\n",
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_index,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_w,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_h,
                                    ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_enccnt,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)/100,
                                    (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)%100,
                                    rqcf_seq.seq.i_period-1,
                                    rqcf_seq.seq.i_btrate,
                                    rqcf_qpr.qpr.i_iupperq,
                                    rqcf_qpr.qpr.i_ilowerq);
                        break;
                    }
                }
            }
        }

        nDevCnt++;
    }

    CamOsMutexUnlock(&_gtMfeProcMutex);

    return 0;
}

static int MfeProcOpenDevInfo(struct inode *inode, struct file *file)
{
    return single_open(file, MfeProcShowDevInfo, NULL);
}

static const struct file_operations MfeProcOpsDevInfo = {
    .owner      = THIS_MODULE,
    .open       = MfeProcOpenDevInfo,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int MfeProcShowCtxInfo(struct seq_file *m, void *v)
{
    mhve_ops* mops;
    rqct_ops* rqct;
    rqct_cfg  rqcf_fps;
    rqct_cfg  rqcf_seq;
    rqct_cfg  rqcf_qpr;
    struct CamOsListHead_t *ptPos, *ptQ;
    MfeDevList_t* ptTmp;
    int nDevCnt = 0;
    int nCtxCnt = 0;

    CamOsMutexLock(&_gtMfeProcMutex);

    if (nShowCtxInfoDevId < 0 || nShowCtxInfoCtxId < 0)
    {
        seq_printf(m, "Please Specify Dev and Ctx by Following Command:\n  echo [DevId] [CtxId] > /proc/mfe/ctx_info\n");
    }
    else
    {
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMfeDevList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MfeDevList_t, tList);

            if(ptTmp->mdev)
            {
                for (nCtxCnt=0; nCtxCnt<MMFE_STREAM_NR; nCtxCnt++)
                {
                    if (ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx)
                    {
                        if (nShowCtxInfoDevId == nDevCnt && nShowCtxInfoCtxId == nCtxCnt)
                        {
                            mops = ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->p_handle;
                            rqct = mops->rqct_ops(mops);

                            rqcf_fps.type = RQCT_CFG_FPS;
                            rqct->get_rqcf(rqct, &rqcf_fps);

                            rqcf_seq.type = RQCT_CONF_SEQ;
                            rqct->get_rqcf(rqct, &rqcf_seq);

                            rqcf_qpr.type = RQCT_CFG_QPR;
                            rqct->get_rqcf(rqct, &rqcf_qpr);

                            seq_printf(m, "MFE Dev%d-Ctx%d Info\n", nShowCtxInfoDevId, nShowCtxInfoCtxId);
                            seq_printf(m, "    Resolution  :  %d x %d\n", ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_w, ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_picture_h);
                            seq_printf(m, "    FPS         :  %2d.%02d\n", (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)/100, (rqcf_fps.fps.n_fps*100/rqcf_fps.fps.d_fps)%100);
                            seq_printf(m, "    GOP         :  %d\n", rqcf_seq.seq.i_period-1);
                            seq_printf(m, "    Frame Count :  %d\n", ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_enccnt);
                            seq_printf(m, "    Dev/CtxState:  %d ~ %d\n", ptTmp->mdev->i_state, ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->i_state);
                            seq_printf(m, "    Real kbps   : %6d %6d %6d %6d %6d (1/3/5/10/60 Sec)\n",
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[0].kbps,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[1].kbps,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[2].kbps,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[3].kbps,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[4].kbps);
                            seq_printf(m, "    Real FPS    :  %2d.%02d  %2d.%02d  %2d.%02d  %2d.%02d  %2d.%02d (1/3/5/10/60 Sec)\n",
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[0].fps_x100 / 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[0].fps_x100 % 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[1].fps_x100 / 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[1].fps_x100 % 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[2].fps_x100 / 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[2].fps_x100 % 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[3].fps_x100 / 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[3].fps_x100 % 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[4].fps_x100 / 100,
                                       ptTmp->mdev->m_regrqc[nCtxCnt].p_mctx->rc_stat[4].fps_x100 % 100);
                            break;
                        }
                    }
                }
            }

            if (nShowCtxInfoDevId == nDevCnt && nShowCtxInfoCtxId == nCtxCnt)
            {
                break;
            }

            nDevCnt++;
        }

        if (nShowCtxInfoDevId != nDevCnt || nShowCtxInfoCtxId != nCtxCnt)
        {
            seq_printf(m, "MFE Dev%d-Ctx%d is Invalid Instance\n", nShowCtxInfoDevId, nShowCtxInfoCtxId);
        }
    }

    CamOsMutexUnlock(&_gtMfeProcMutex);

    return 0;
}

static int MfeProcOpenCtxInfo(struct inode *inode, struct file *file)
{
    return single_open(file, MfeProcShowCtxInfo, NULL);
}

static ssize_t MfeProcWriteCtxInfo (struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[] = "0x00000000\n";
    size_t len = min(sizeof(buf) - 1, count);
    unsigned long val[2];

    if (CamOsCopyFromUpperLayer(buf, buffer, len))
        return count;
    buf[len] = 0;

    if (sscanf(buf, "%li %li", &val[0], &val[1]) == 2)
    {
        CamOsMutexLock(&_gtMfeProcMutex);
        nShowCtxInfoDevId = val[0];
        nShowCtxInfoCtxId = val[1];
        CamOsMutexUnlock(&_gtMfeProcMutex);
    }

    return strnlen(buf, len);
}

static const struct file_operations MfeProcOpsCtxInfo = {
    .owner      = THIS_MODULE,
    .open       = MfeProcOpenCtxInfo,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
    .write      = MfeProcWriteCtxInfo,
};

static int MfeProcShowUtilization(struct seq_file *m, void *v)
{
    struct CamOsListHead_t *ptPos, *ptQ;
    MfeDevList_t* ptTmp;

    CamOsMutexLock(&_gtMfeProcMutex);

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMfeDevList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MfeDevList_t, tList);

        if(ptTmp->mdev)
        {
            seq_printf(m, "MFE Dev (bank 0x%04X) utilization:  %2d\n", ((u32)ptTmp->mdev->p_reg_base & 0x00FFFF00)>>9, ptTmp->mdev->i_utilization_percentage);
        }
    }

    CamOsMutexUnlock(&_gtMfeProcMutex);

    return 0;
}

static int MfeProcOpenUtilization(struct inode *inode, struct file *file)
{
    return single_open(file, MfeProcShowUtilization, NULL);
}

static const struct file_operations MfeProcOpsUtilization = {
    .owner      = THIS_MODULE,
    .open       = MfeProcOpenUtilization,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int MfeProcShowDumpReg(struct seq_file *m, void *v)
{
    struct CamOsListHead_t *ptPos, *ptQ;
    MfeDevList_t* ptTmp;
    int nDevCnt = 0;
    int regaddr;
    int bankidx = 0;
    unsigned short regvalue;
    void* mfe_base;

    CamOsMutexLock(&_gtMfeProcMutex);

    if (nShowRegInfoDevId < 0)
    {
        seq_printf(m, "Please Specify Dev by Following Command:\n  echo [DevId] > /proc/mfe/dump_reg\n");
    }
    else
    {
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMfeDevList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MfeDevList_t, tList);

            if(ptTmp->mdev && (nDevCnt == nShowRegInfoDevId))
            {
                for(bankidx = 0; bankidx < 3; bankidx++)
                {
                    mfe_base = ptTmp->mdev->p_reg_base + 0x100 * bankidx;
                    seq_printf(m, "MFE Dev%d (bank 0x%04X)\n", nDevCnt, ((u32)mfe_base & 0x00FFFF00)>>9);
                    seq_printf(m, "  Offset(h)  00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F\n");
                    seq_printf(m, "  ----------------------------------------------------------------------------------------\n");
                    for (regaddr=0; regaddr < 0x80; regaddr++)
                    {
                        regvalue = REGRD(mfe_base, regaddr, "");
                        if(regaddr % 16 == 0)
                            seq_printf(m, "\n%08X   ", regaddr);
                        seq_printf(m, "%04X ", regvalue);
                    }
                    seq_printf(m, "\n");
                }

                break;
            }

            nDevCnt++;
        }
    }

    CamOsMutexUnlock(&_gtMfeProcMutex);

    return 0;
}

static ssize_t MfeProcWriteRegInfo (struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char buf[] = "0x00000000\n";
    size_t len = min(sizeof(buf) - 1, count);
    unsigned long val[2];

    if (CamOsCopyFromUpperLayer(buf, buffer, len))
        return count;
    buf[len] = 0;

    if (sscanf(buf, "%li", &val[0]) == 1)
    {
        CamOsMutexLock(&_gtMfeProcMutex);
        nShowRegInfoDevId = val[0];
        CamOsMutexUnlock(&_gtMfeProcMutex);
    }

    return strnlen(buf, len);
}

static int MfeProcOpenDumpReg(struct inode *inode, struct file *file)
{
    return single_open(file, MfeProcShowDumpReg, NULL);
}

static const struct file_operations MfeProcOpsDumpReg = {
    .owner      = THIS_MODULE,
    .open       = MfeProcOpenDumpReg,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
    .write      = MfeProcWriteRegInfo,
};

void MfeProcInit(mmfe_dev* mdev)
{
    struct proc_dir_entry *pde;
    MfeDevList_t* ptNewEntry;

    CamOsMutexLock(&_gtMfeProcMutex);

    if (!_gMfeDevCnt)
    {
        _gpRootMfeDir = proc_mkdir("mfe", NULL);
        if (!_gpRootMfeDir)
        {
            CamOsPrintf("[MFE]can not create proc\n");
            goto MfeProcInit_exit;
        }

        pde = proc_create("dev_info", S_IRUGO, _gpRootMfeDir, &MfeProcOpsDevInfo);
        pde = proc_create("ctx_info", S_IRUGO, _gpRootMfeDir, &MfeProcOpsCtxInfo);
        pde = proc_create("utilization", S_IRUGO, _gpRootMfeDir, &MfeProcOpsUtilization);
        pde = proc_create("dump_reg", S_IRUGO, _gpRootMfeDir, &MfeProcOpsDumpReg);

        CAM_OS_INIT_LIST_HEAD(&_gtMfeDevList.tList);
    }

    ptNewEntry = (MfeDevList_t*)CamOsMemCalloc(1, sizeof(MfeDevList_t));
    ptNewEntry->mdev = mdev;
    CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMfeDevList.tList);

    _gMfeDevCnt++;

MfeProcInit_exit:
    CamOsMutexUnlock(&_gtMfeProcMutex);
    return ;
}

void MfeProcDeInit(mmfe_dev* mdev)
{
    struct CamOsListHead_t *ptPos, *ptQ;
    MfeDevList_t* ptTmp;

    CamOsMutexLock(&_gtMfeProcMutex);

    if (_gMfeDevCnt <= 0)
        goto MfeProcDeInit_exit;


    if (_gMfeDevCnt == 1)
    {
        remove_proc_entry("dev_info", _gpRootMfeDir);
        remove_proc_entry("ctx_info", _gpRootMfeDir);
        remove_proc_entry("utilization", _gpRootMfeDir);
        remove_proc_entry("dump_reg", _gpRootMfeDir);
        remove_proc_entry("mfe", NULL);

        _gpRootMfeDir = NULL;
    }

    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMfeDevList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MfeDevList_t, tList);

        if(ptTmp->mdev == mdev)
        {
            CAM_OS_LIST_DEL(ptPos);
            CamOsMemRelease(ptTmp);
        }
    }

    _gMfeDevCnt--;

MfeProcDeInit_exit:
    CamOsMutexUnlock(&_gtMfeProcMutex);
    return;
}
