#ifndef __ST_RTSP_H__
#define __ST_RTSP_H__

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include "mi_vif.h"
#include "st_common.h"

//原来的"结构体初始化"顺序，在c++编译时会报错。
ST_CaseDesc_t g_stVifCaseDesc[2]=
{
	{
		.stDesc =
		{
			.u32CaseIndex = 0,
			.szDesc = {'v','e','n','c','-','h','2','6','4'},
			.u32WndNum = 1,
			.eDispoutTiming = E_ST_TIMING_MAX,
			.u32VencNum = 0,
			.bNeedVdisp = 0,
			.u32VideoChnNum = 0,
		},
		.eDispoutTiming = E_ST_TIMING_1080P_60,
		.s32SplitMode = 0,
		.u32SubCaseNum = 1,
		.u32ShowWndNum = 1,
		.stSubDesc =
		{
			{
				.u32CaseIndex = 0,
				.szDesc = {'e','x','i','t'},
				.u32WndNum = 0,
				.eDispoutTiming = E_ST_TIMING_MAX,
				.u32VencNum = 0,
				.bNeedVdisp = 0,
				.u32VideoChnNum = 0,
			},
		},
		.stCaseArgs =
		{
			{
				.eVideoChnType = E_ST_VIF_CHN,
				.uChnArg =
				{
					.stVifChnArg =
					{
						.u32Chn = 0,
						.u16CapWidth = 3840,
						.u16CapHeight = 2160,
						.s32FrmRate = E_MI_VIF_FRAMERATE_FULL,
						.eType = E_MI_VENC_MODTYPE_H264E,
                        .u16VdfInWidth = 0,
                        .u16VdfInHeight = 0,
                        .u16OdNum = 0,
                        .stOdArea =
                        {
                            {
                                .s32X = 0,
                                .s32Y = 0,
                                .u16PicW = 0,
                                .u16PicH = 0,
                            },
                        },
                        .u16MdNum = 0,
                        .stMdArea =
                        {
                            {
                                .s32X = 0,
                                .s32Y = 0,
                                .u16PicW = 0,
                                .u16PicH = 0,
                            }
                        },
					}
				}
			},
		},
	},
	{
		.stDesc =
		{
			.u32CaseIndex = 1,
			.szDesc = {'v','e','n','c','-','h','2','6','5'},
			.u32WndNum = 1,
			.eDispoutTiming = E_ST_TIMING_MAX,
			.u32VencNum = 0,
			.bNeedVdisp = 0,
			.u32VideoChnNum = 0,
		},
		.eDispoutTiming = E_ST_TIMING_1080P_60,
		.s32SplitMode = 0,
		.u32SubCaseNum = 1,
		.u32ShowWndNum = 1,
		.stSubDesc =
		{
			{
				.u32CaseIndex = 0,
				.szDesc = {'e','x','i','t'},
				.u32WndNum = 0,
				.eDispoutTiming = E_ST_TIMING_MAX,
				.u32VencNum = 0,
				.bNeedVdisp = 0,
				.u32VideoChnNum = 0,
			},
		},
		.stCaseArgs =
		{
			{
				.eVideoChnType = E_ST_VIF_CHN,
				.uChnArg =
				{
					.stVifChnArg =
					{
						.u32Chn = 0,
						.u16CapWidth = 3840,
						.u16CapHeight = 2160,
						.s32FrmRate = E_MI_VIF_FRAMERATE_FULL,
						.eType = E_MI_VENC_MODTYPE_H265E,
						.u16VdfInWidth = 0,
						.u16VdfInHeight = 0,
						.u16OdNum = 0,
						.stOdArea =
                        {
                            {
                                .s32X = 0,
                                .s32Y = 0,
                                .u16PicW = 0,
                                .u16PicH = 0,
                            },
                        },
						.u16MdNum = 0,
						.stMdArea =
                        {
                            {
                                .s32X = 0,
                                .s32Y = 0,
                                .u16PicW = 0,
                                .u16PicH = 0,
                            }
                        },
					}
				}
			},
		},
	}
};

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif  // __ST_RTSP_H__