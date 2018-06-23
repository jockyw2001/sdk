#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdec_framing.h"

#define TRUE (1)
#define FALSE (0)
#define SCAN_BUFFER_LEN (512 * 1024)
#define FRM_INF

typedef enum
{
    VIDEO_STREAM_TYPE_H264 = 0,
    VIDEO_STREAM_TYPE_H265,
    VIDEO_STREAM_TYPE_JPEG,
    VIDEO_STREAM_TYPE_MAX,
} VIDEO_STREAM_TYPE_e;

///hevc mode:vps + sps + pps + sei + idr + p + p + p ..... vps + sps + pps + sei + idr + p + p + p
typedef enum
{
  HEVC_NAL_TRAIL_N    = 0,
  HEVC_NAL_TRAIL_R    = 1,///P
  HEVC_NAL_TSA_N      = 2,
  HEVC_NAL_TSA_R      = 3,
  HEVC_NAL_STSA_N     = 4,
  HEVC_NAL_STSA_R     = 5,
  HEVC_NAL_RADL_N     = 6,
  HEVC_NAL_RADL_R     = 7,
  HEVC_NAL_RASL_N     = 8,
  HEVC_NAL_RASL_R     = 9,
  HEVC_NAL_BLA_W_LP   = 16,
  HEVC_NAL_BLA_W_RADL = 17,
  HEVC_NAL_BLA_N_LP   = 18,
  HEVC_NAL_IDR_W_RADL = 19,///IDR
  HEVC_NAL_IDR_N_LP   = 20,
  HEVC_NAL_CRA_NUT    = 21,
  HEVC_NAL_VPS        = 32,
  HEVC_NAL_SPS        = 33,
  HEVC_NAL_PPS        = 34,
  HEVC_NAL_AUD        = 35,
  HEVC_NAL_EOS_NUT    = 36,
  HEVC_NAL_EOB_NUT    = 37,
  HEVC_NAL_FD_NUT     = 38,
  HEVC_NAL_SEI_PREFIX = 39,
  HEVC_NAL_SEI_SUFFIX = 40,
  HEVC_NAL_UNIT_TYPE_MAX,
} HEVC_NAL_UNIT_TYPE_e;

///avc mode:sps + pps + idr + p + p + p + p....sps + pps + idr + p + p + p + p
typedef enum
{
    AVC_NAL_PB = 1,
    AVC_NAL_IDR = 5,
    AVC_NAL_SPS = 7,
    AVC_NAL_PPS = 8,
    AVC_NAL_AUD = 9,
    AVC_NAL_MAX,
} AVC_NAL_UNIT_TYPE_e;

#define IsNaluBound(pu8CurPos) ((pu8CurPos[0] == 0x00)&&(pu8CurPos[1] == 0x00)&&(pu8CurPos[2] == 0x00)&&(pu8CurPos[3] == 0x01))
#define IsJpgBound(pu8CurPos)  ((pu8CurPos[0] == 0xFF)&&(pu8CurPos[1] == 0xD8))
static long int _frm_number = 0;

unsigned int get_next_frame_start_position(unsigned char *pu8FrmBuf, unsigned int u32FrmLen, unsigned char u8StreamType, unsigned char *pu8StartAlign)
{
    unsigned char *pu8CurPos = NULL;
    unsigned char *pu8EndPos = NULL;
    unsigned char u8Len = 0;

    VIDEO_STREAM_TYPE_e eStreamType = (VIDEO_STREAM_TYPE_e)u8StreamType;
    if (VIDEO_STREAM_TYPE_H265 == eStreamType)
    {
        pu8EndPos = pu8FrmBuf + u32FrmLen - 5;
        *pu8StartAlign = IsNaluBound(pu8FrmBuf);
    }
    else
    if (VIDEO_STREAM_TYPE_H264 == eStreamType)
    {
        pu8EndPos = pu8FrmBuf + u32FrmLen - 5;
        *pu8StartAlign = IsNaluBound(pu8FrmBuf);
    }
    else
    if (VIDEO_STREAM_TYPE_JPEG == eStreamType)
    {
        pu8EndPos = pu8FrmBuf + u32FrmLen - 3;
        *pu8StartAlign = IsJpgBound(pu8FrmBuf);
    }

    if (!(*pu8StartAlign))
    {
        FRM_INF("Start Not Align, Return False\n");
    }

    pu8CurPos = pu8FrmBuf + 1;
    while (pu8CurPos < pu8EndPos)
    {
        ///0 0 0 1
        if (VIDEO_STREAM_TYPE_H265 == eStreamType)
        {
            HEVC_NAL_UNIT_TYPE_e eType = HEVC_NAL_UNIT_TYPE_MAX;
            if (!IsNaluBound(pu8CurPos))
            {
                pu8CurPos++;
                continue;
            }

            eType = (pu8CurPos[4] >> 1) & 0x3F;
            if (   (eType == HEVC_NAL_IDR_W_RADL)
                || (eType == HEVC_NAL_TRAIL_R))
            {
                _frm_number++;
                FRM_INF("HEVC Frm Find %d, Off:0x%lx\n", eType, pu8CurPos - pu8FrmBuf);
                return pu8CurPos - pu8FrmBuf;
            }
            else
            if (   (eType == HEVC_NAL_SPS)
                || (eType == HEVC_NAL_PPS)
                || (eType == HEVC_NAL_AUD)
                || (eType == HEVC_NAL_VPS))
            {
                FRM_INF("HEVC Header Find %d, Off:0x%lx\n", eType, pu8CurPos - pu8FrmBuf);
                return pu8CurPos - pu8FrmBuf;
            }

            pu8CurPos++;
        }
        else
        if (VIDEO_STREAM_TYPE_H264 == eStreamType)
        {
            AVC_NAL_UNIT_TYPE_e eType = AVC_NAL_MAX;
            if (!IsNaluBound(pu8CurPos))
            {
                pu8CurPos++;
                continue;
            }

            eType = pu8CurPos[4] & 0x1F;
            if (   (eType == AVC_NAL_PB)
                || (eType == AVC_NAL_IDR))
            {
                _frm_number++;
                FRM_INF("AVC Frm Find %d Off:0x%lx\n", eType, pu8CurPos - pu8FrmBuf);
                return pu8CurPos - pu8FrmBuf;
            }
            else
            if (   (eType == AVC_NAL_SPS)
                || (eType == AVC_NAL_PPS)
                || (eType == AVC_NAL_AUD))
            {
                FRM_INF("AVC Header Find %d Off:0x%lx\n", eType, pu8CurPos - pu8FrmBuf);
                return pu8CurPos - pu8FrmBuf;
            }

            pu8CurPos++;
        }
        else
        if (VIDEO_STREAM_TYPE_JPEG == eStreamType)
        {
            if (!IsJpgBound(pu8CurPos))
            {
                pu8CurPos++;
                continue;
            }

            FRM_INF("JPG Find, Off:0x%lx\n", pu8CurPos - pu8FrmBuf);
            return pu8CurPos - pu8FrmBuf;
        }
        else
        {
            FRM_INF("unkonw type:%d", eStreamType);
            return 0;
        }
    }

    FRM_INF("Not Found\n");
    return 0;
}

#if (0)
int main(int argc, char *argv[])
{
    FILE *rfp = fopen("./test.es", "rb");

    if (!rfp)
    {
        printf("open err\n");
        return -1;
    }

    int frm = 0;
    char name[256];
    char *buf = (char *)malloc(SCAN_BUFFER_LEN);
    if (!buf)
    {
        return -1;
    }

    int len = 0;
    long int total_off = 0;
    int rlen = 0;
    while (1)
    {
        rlen = fread(buf + len, 1, SCAN_BUFFER_LEN - len, rfp);
        if (rlen <= 0)
        {
            printf("file end, done\n");
            break;
        }

        len += rlen;
        int frm_bound = 0;
        char *cur_pos = buf;
        while (len > 0)
        {
            frm_bound = get_next_frame_start_position(cur_pos, len, VIDEO_STREAM_TYPE_H265);
            if (!frm_bound)
            {
                if (len > (SCAN_BUFFER_LEN >> 1))
                {
                    char *p = (char *)malloc(len);
                    memcpy(p, cur_pos, len);
                    memcpy(buf, p, len);
                    free(p);
                    p = NULL;
                }
                else
                {
                    memcpy(buf, cur_pos, len);
                }

                break;
            }

            total_off += frm_bound;
            FRM_INF("found frm(%ld), off:0x%lx, len:%d, total_off:0x%lx\n", _frm_number, cur_pos - buf, len, total_off);
            if (0)///(frm < 20)
            {
                sprintf(name, "./frm_%d.raw", frm++);
                FILE *fpw = fopen(name, "w+");
                fwrite(cur_pos, 1, frm_bound, fpw);
                fflush(fpw);
                fclose(fpw);
            }
            len -= frm_bound;
            cur_pos += frm_bound;
        }
    }
    return 0;
}
#endif
