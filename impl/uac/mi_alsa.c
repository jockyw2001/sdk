//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
// By accessing, browsing and/or using MStar Software, you acknowledge that you
// have read, understood, and agree, to be bound by below terms ("Terms") and to
// comply with all applicable laws and regulations:
//
// 1. MStar shall retain any and all right, ownership and interest to MStar
//    Software and any modification/derivatives thereof.
//    No right, ownership, or interest to MStar Software and any
//    modification/derivatives thereof is transferred to you under Terms.
//
// 2. You understand that MStar Software might include, incorporate or be
//    supplied together with third party`s software and the use of MStar
//    Software may require additional licenses from third parties.
//    Therefore, you hereby agree it is your sole responsibility to separately
//    obtain any and all third party right and license necessary for your use of
//    such third party`s software.
//
// 3. MStar Software and any modification/derivatives thereof shall be deemed as
//    MStar`s confidential information and you agree to keep MStar`s
//    confidential information in strictest confidence and not disclose to any
//    third party.
//
// 4. MStar Software is provided on an "AS IS" basis without warranties of any
//    kind. Any warranties are hereby expressly disclaimed by MStar, including
//    without limitation, any warranties of merchantability, non-infringement of
//    intellectual property rights, fitness for a particular purpose, error free
//    and in conformity with any international standard.  You agree to waive any
//    claim against MStar for any loss, damage, cost or expense that you may
//    incur related to your use of MStar Software.
//    In no event shall MStar be liable for any direct, indirect, incidental or
//    consequential damages, including without limitation, lost of profit or
//    revenues, lost or damage of data, and unauthorized system use.
//    You agree that this Section 4 shall still apply without being affected
//    even if MStar Software has been modified by MStar in accordance with your
//    request or instruction for your use, except otherwise agreed by both
//    parties in writing.
//
// 5. If requested, MStar may from time to time provide technical supports or
//    services in relation with MStar Software to you for your use of
//    MStar Software in conjunction with your or your customer`s product
//    ("Services").
//    You understand and agree that, except otherwise agreed by both parties in
//    writing, Services are provided on an "AS IS" basis and the warranty
//    disclaimer set forth in Section 4 above shall apply.
//
// 6. Nothing contained herein shall be construed as by implication, estoppels
//    or otherwise:
//    (a) conferring any license or right to use MStar name, trademark, service
//        mark, symbol or any other identification;
//    (b) obligating MStar or any of its affiliates to furnish any person,
//        including without limitation, you and your customers, any assistance
//        of any kind whatsoever, or any information; or
//    (c) conferring any license or right under any intellectual property right.
//
// 7. These terms shall be governed by and construed in accordance with the laws
//    of Taiwan, R.O.C., excluding its conflict of law rules.
//    Any and all dispute arising out hereof or related hereto shall be finally
//    settled by arbitration referred to the Chinese Arbitration Association,
//    Taipei in accordance with the ROC Arbitration Law and the Arbitration
//    Rules of the Association by three (3) arbitrators appointed in accordance
//    with the said Rules.
//    The place of arbitration shall be in Taipei, Taiwan and the language shall
//    be English.
//    The arbitration award shall be final and binding to both parties.
//
//******************************************************************************
//<MStar Software>

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   mi_uac_impl.c
/// @brief  uac module impl
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/tlv.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/info.h>
#include <sound/initval.h>

MODULE_DESCRIPTION("UAC soundcard (/dev/null)");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("{{ALSA,UAC soundcard}}");

#define MAX_PCM_DEVICES		4
#define MAX_PCM_SUBSTREAMS	128
#define MAX_MIDI_DEVICES	2

/* defaults */
#define MAX_BUFFER_SIZE		(64*1024)
#define MIN_PERIOD_SIZE		64
#define MAX_PERIOD_SIZE		MAX_BUFFER_SIZE
#define USE_FORMATS 		(SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_S16_LE)
#define USE_RATE		SNDRV_PCM_RATE_CONTINUOUS | SNDRV_PCM_RATE_8000_48000
#define USE_RATE_MIN		5500
#define USE_RATE_MAX		48000
#define USE_CHANNELS_MIN 	1
#define USE_CHANNELS_MAX 	2
#define USE_PERIODS_MIN 	1
#define USE_PERIODS_MAX 	1024

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;	/* Index 0-MAX */
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;	/* ID for this card */
static bool enable[SNDRV_CARDS] = {1, [1 ... (SNDRV_CARDS - 1)] = 0};
static char *model[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = NULL};
static int pcm_devs[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 1};
static int pcm_substreams[SNDRV_CARDS] = {[0 ... (SNDRV_CARDS - 1)] = 8};
static bool fake_buffer = 1;

module_param_array(index, int, NULL, 0444);
MODULE_PARM_DESC(index, "Index value for uac soundcard.");
module_param_array(id, charp, NULL, 0444);
MODULE_PARM_DESC(id, "ID string for uac soundcard.");
module_param_array(enable, bool, NULL, 0444);
MODULE_PARM_DESC(enable, "Enable this uac soundcard.");
module_param_array(model, charp, NULL, 0444);
MODULE_PARM_DESC(model, "Soundcard model.");
module_param_array(pcm_devs, int, NULL, 0444);
MODULE_PARM_DESC(pcm_devs, "PCM devices # (0-4) for uac driver.");
module_param_array(pcm_substreams, int, NULL, 0444);
MODULE_PARM_DESC(pcm_substreams, "PCM substreams # (1-128) for uac driver.");
module_param(fake_buffer, bool, 0444);
MODULE_PARM_DESC(fake_buffer, "Fake buffer allocations.");

static struct platform_device *devices[SNDRV_CARDS];

#define MIXER_ADDR_MASTER	0
#define MIXER_ADDR_LINE		1
#define MIXER_ADDR_MIC		2
#define MIXER_ADDR_SYNTH	3
#define MIXER_ADDR_CD		4
#define MIXER_ADDR_LAST		4

struct uac_timer_ops {
	int (*create)(struct snd_pcm_substream *);
	void (*free)(struct snd_pcm_substream *);
	int (*prepare)(struct snd_pcm_substream *);
	int (*start)(struct snd_pcm_substream *);
	int (*stop)(struct snd_pcm_substream *);
	snd_pcm_uframes_t (*pointer)(struct snd_pcm_substream *);
};

#define get_uac_ops(substream) \
	(*(const struct uac_timer_ops **)(substream)->runtime->private_data)

struct uac_model {
	const char *name;
	int (*playback_constraints)(struct snd_pcm_runtime *runtime);
	int (*capture_constraints)(struct snd_pcm_runtime *runtime);
	u64 formats;
	size_t buffer_bytes_max;
	size_t period_bytes_min;
	size_t period_bytes_max;
	unsigned int periods_min;
	unsigned int periods_max;
	unsigned int rates;
	unsigned int rate_min;
	unsigned int rate_max;
	unsigned int channels_min;
	unsigned int channels_max;
};

struct snd_uac {
	struct snd_card *card;
	struct uac_model *model;
	struct snd_pcm *pcm;
	struct snd_pcm_hardware pcm_hw;
	spinlock_t mixer_lock;
	int mixer_volume[MIXER_ADDR_LAST+1][2];
	int capture_source[MIXER_ADDR_LAST+1][2];
	int iobox;
	struct snd_kcontrol *cd_volume_ctl;
	struct snd_kcontrol *cd_switch_ctl;
};

/*
 * card models
 */

static int emu10k1_playback_constraints(struct snd_pcm_runtime *runtime)
{
	int err;
	err = snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	if (err < 0)
		return err;
	err = snd_pcm_hw_constraint_minmax(runtime, SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 256, UINT_MAX);
	if (err < 0)
		return err;
	return 0;
}

struct uac_model model_emu10k1 = {
	.name = "emu10k1",
	.playback_constraints = emu10k1_playback_constraints,
	.buffer_bytes_max = 128 * 1024,
};

struct uac_model model_rme9652 = {
	.name = "rme9652",
	.buffer_bytes_max = 26 * 64 * 1024,
	.formats = SNDRV_PCM_FMTBIT_S32_LE,
	.channels_min = 26,
	.channels_max = 26,
	.periods_min = 2,
	.periods_max = 2,
};

struct uac_model model_ice1712 = {
	.name = "ice1712",
	.buffer_bytes_max = 256 * 1024,
	.formats = SNDRV_PCM_FMTBIT_S32_LE,
	.channels_min = 10,
	.channels_max = 10,
	.periods_min = 1,
	.periods_max = 1024,
};

struct uac_model model_uda1341 = {
	.name = "uda1341",
	.buffer_bytes_max = 16380,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.channels_min = 2,
	.channels_max = 2,
	.periods_min = 2,
	.periods_max = 255,
};

struct uac_model model_ac97 = {
	.name = "ac97",
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.channels_min = 2,
	.channels_max = 2,
	.rates = SNDRV_PCM_RATE_48000,
	.rate_min = 48000,
	.rate_max = 48000,
};

struct uac_model model_ca0106 = {
	.name = "ca0106",
	.formats = SNDRV_PCM_FMTBIT_S16_LE,
	.buffer_bytes_max = ((65536-64)*8),
	.period_bytes_max = (65536-64),
	.periods_min = 2,
	.periods_max = 8,
	.channels_min = 2,
	.channels_max = 2,
	.rates = SNDRV_PCM_RATE_48000|SNDRV_PCM_RATE_96000|SNDRV_PCM_RATE_192000,
	.rate_min = 48000,
	.rate_max = 192000,
};

struct uac_model *uac_models[] = {
	&model_emu10k1,
	&model_rme9652,
	&model_ice1712,
	&model_uda1341,
	&model_ac97,
	&model_ca0106,
	NULL
};

/*
 * system timer interface
 */

struct uac_systimer_pcm {
	/* ops must be the first item */
	const struct uac_timer_ops *timer_ops;
	spinlock_t lock;
	struct timer_list timer;
	unsigned long base_time;
	unsigned int frac_pos;	/* fractional sample position (based HZ) */
	unsigned int frac_period_rest;
	unsigned int frac_buffer_size;	/* buffer_size * HZ */
	unsigned int frac_period_size;	/* period_size * HZ */
	unsigned int rate;
	int elapsed;
	struct snd_pcm_substream *substream;
};

static void uac_systimer_rearm(struct uac_systimer_pcm *dpcm)
{
	dpcm->timer.expires = jiffies +
		(dpcm->frac_period_rest + dpcm->rate - 1) / dpcm->rate;
	add_timer(&dpcm->timer);
}

static void uac_systimer_update(struct uac_systimer_pcm *dpcm)
{
	unsigned long delta;

	delta = jiffies - dpcm->base_time;
	if (!delta)
		return;
	dpcm->base_time += delta;
	delta *= dpcm->rate;
	dpcm->frac_pos += delta;
	while (dpcm->frac_pos >= dpcm->frac_buffer_size)
		dpcm->frac_pos -= dpcm->frac_buffer_size;
	while (dpcm->frac_period_rest <= delta) {
		dpcm->elapsed++;
		dpcm->frac_period_rest += dpcm->frac_period_size;
	}
	dpcm->frac_period_rest -= delta;
}

static int uac_systimer_start(struct snd_pcm_substream *substream)
{
	struct uac_systimer_pcm *dpcm = substream->runtime->private_data;
	spin_lock(&dpcm->lock);
	dpcm->base_time = jiffies;
	uac_systimer_rearm(dpcm);
	spin_unlock(&dpcm->lock);
	return 0;
}

static int uac_systimer_stop(struct snd_pcm_substream *substream)
{
	struct uac_systimer_pcm *dpcm = substream->runtime->private_data;
	spin_lock(&dpcm->lock);
	del_timer(&dpcm->timer);
	spin_unlock(&dpcm->lock);
	return 0;
}

static int uac_systimer_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct uac_systimer_pcm *dpcm = runtime->private_data;

	dpcm->frac_pos = 0;
	dpcm->rate = runtime->rate;
	dpcm->frac_buffer_size = runtime->buffer_size * HZ;
	dpcm->frac_period_size = runtime->period_size * HZ;
	dpcm->frac_period_rest = dpcm->frac_period_size;
	dpcm->elapsed = 0;

	return 0;
}

static void uac_systimer_callback(unsigned long data)
{
	struct uac_systimer_pcm *dpcm = (struct uac_systimer_pcm *)data;
	unsigned long flags;
	int elapsed = 0;

	spin_lock_irqsave(&dpcm->lock, flags);
	uac_systimer_update(dpcm);
	uac_systimer_rearm(dpcm);
	elapsed = dpcm->elapsed;
	dpcm->elapsed = 0;
	spin_unlock_irqrestore(&dpcm->lock, flags);
	if (elapsed)
		snd_pcm_period_elapsed(dpcm->substream);
}

static snd_pcm_uframes_t
uac_systimer_pointer(struct snd_pcm_substream *substream)
{
	struct uac_systimer_pcm *dpcm = substream->runtime->private_data;
	snd_pcm_uframes_t pos;

	spin_lock(&dpcm->lock);
	uac_systimer_update(dpcm);
	pos = dpcm->frac_pos / HZ;
	spin_unlock(&dpcm->lock);
	return pos;
}

static int uac_systimer_create(struct snd_pcm_substream *substream)
{
	struct uac_systimer_pcm *dpcm;

	dpcm = kzalloc(sizeof(*dpcm), GFP_KERNEL);
	if (!dpcm)
		return -ENOMEM;
	substream->runtime->private_data = dpcm;
	init_timer(&dpcm->timer);
	dpcm->timer.data = (unsigned long) dpcm;
	dpcm->timer.function = uac_systimer_callback;
	spin_lock_init(&dpcm->lock);
	dpcm->substream = substream;
	return 0;
}

static void uac_systimer_free(struct snd_pcm_substream *substream)
{
	kfree(substream->runtime->private_data);
}

static struct uac_timer_ops uac_systimer_ops = {
	.create =	uac_systimer_create,
	.free =		uac_systimer_free,
	.prepare =	uac_systimer_prepare,
	.start =	uac_systimer_start,
	.stop =		uac_systimer_stop,
	.pointer =	uac_systimer_pointer,
};


/*
 * PCM interface
 */

static int uac_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		return get_uac_ops(substream)->start(substream);
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		return get_uac_ops(substream)->stop(substream);
	}
	return -EINVAL;
}

static int uac_pcm_prepare(struct snd_pcm_substream *substream)
{
	return get_uac_ops(substream)->prepare(substream);
}

static snd_pcm_uframes_t uac_pcm_pointer(struct snd_pcm_substream *substream)
{
	return get_uac_ops(substream)->pointer(substream);
}

static struct snd_pcm_hardware uac_pcm_hardware = {
	.info =			(SNDRV_PCM_INFO_MMAP |
				 SNDRV_PCM_INFO_INTERLEAVED |
				 SNDRV_PCM_INFO_RESUME |
				 SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		USE_FORMATS,
	.rates =		USE_RATE,
	.rate_min =		USE_RATE_MIN,
	.rate_max =		USE_RATE_MAX,
	.channels_min =		USE_CHANNELS_MIN,
	.channels_max =		USE_CHANNELS_MAX,
	.buffer_bytes_max =	MAX_BUFFER_SIZE,
	.period_bytes_min =	MIN_PERIOD_SIZE,
	.period_bytes_max =	MAX_PERIOD_SIZE,
	.periods_min =		USE_PERIODS_MIN,
	.periods_max =		USE_PERIODS_MAX,
	.fifo_size =		0,
};

extern int mi_uac_playback_hw_params(struct snd_pcm_hw_params *hw_params);
extern int mi_uac_capture_hw_params(struct snd_pcm_hw_params *hw_params);
static int uac_pcm_hw_params(struct snd_pcm_substream *substream,
			       struct snd_pcm_hw_params *hw_params)
{
    if(SNDRV_PCM_STREAM_PLAYBACK==substream->stream){
       if(0 > mi_uac_playback_hw_params(hw_params))
			printk(KERN_INFO
				"%s Playback hw params Failed\n",__func__);
    }
    else
    if(SNDRV_PCM_STREAM_CAPTURE==substream->stream){
       if(0 > mi_uac_capture_hw_params(hw_params))
			printk(KERN_INFO
				"%s Capture hw params Failed\n",__func__);
    }

	if (fake_buffer) {
		/* runtime->dma_bytes has to be set manually to allow mmap */
		substream->runtime->dma_bytes = params_buffer_bytes(hw_params);
		return 0;
	}
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

static int uac_pcm_hw_free(struct snd_pcm_substream *substream)
{
	if (fake_buffer)
		return 0;
	return snd_pcm_lib_free_pages(substream);
}

static int uac_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_uac *uac = snd_pcm_substream_chip(substream);
	struct uac_model *model = uac->model;
	struct snd_pcm_runtime *runtime = substream->runtime;
	const struct uac_timer_ops *ops;
	int err;

	ops = &uac_systimer_ops;

	err = ops->create(substream);
	if (err < 0)
		return err;
	get_uac_ops(substream) = ops;

	runtime->hw = uac->pcm_hw;
	if (substream->pcm->device & 1) {
		runtime->hw.info &= ~SNDRV_PCM_INFO_INTERLEAVED;
		runtime->hw.info |= SNDRV_PCM_INFO_NONINTERLEAVED;
	}
	if (substream->pcm->device & 2)
		runtime->hw.info &= ~(SNDRV_PCM_INFO_MMAP |
				      SNDRV_PCM_INFO_MMAP_VALID);

	if (model == NULL)
		return 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (model->playback_constraints)
			err = model->playback_constraints(substream->runtime);
	} else {
		if (model->capture_constraints)
			err = model->capture_constraints(substream->runtime);
	}
	if (err < 0) {
		get_uac_ops(substream)->free(substream);
		return err;
	}
	return 0;
}

static int uac_pcm_close(struct snd_pcm_substream *substream)
{
	get_uac_ops(substream)->free(substream);
	return 0;
}

/*
 * uac buffer handling
 */

static void *uac_page[2];

static void free_fake_buffer(void)
{
	if (fake_buffer) {
		int i;
		for (i = 0; i < 2; i++)
			if (uac_page[i]) {
				free_page((unsigned long)uac_page[i]);
				uac_page[i] = NULL;
			}
	}
}

static int alloc_fake_buffer(void)
{
	int i;

	if (!fake_buffer)
		return 0;
	for (i = 0; i < 2; i++) {
		uac_page[i] = (void *)get_zeroed_page(GFP_KERNEL);
		if (!uac_page[i]) {
			free_fake_buffer();
			return -ENOMEM;
		}
	}
	return 0;
}

extern s32 mi_uac_capture(void *from, u32 length);
extern s32 mi_uac_playback(void *to, u32 length);
static int uac_pcm_copy(struct snd_pcm_substream *substream,
			  int channel, snd_pcm_uframes_t pos,
			  void __user *dst, snd_pcm_uframes_t count)
{
	static unsigned int ecount = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	u32 bytes = frames_to_bytes(runtime,count);
	if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream)
	{
		if(0 > mi_uac_playback(dst,bytes)){
			if(ecount++/100){
				ecount = 0;
				printk(KERN_INFO
					"%s Put MI buf Failed\n",__func__);
            }
		}
	}
	else if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
	{
		if(0 > mi_uac_capture(dst,bytes)){
			if(ecount++/100){
				ecount = 0;
				printk(KERN_INFO
					"%s Get MI Buf Failed\n",__func__);
            }
			memset(dst,0xff,bytes);
		}
	}
	return 0; /* do nothing */
}

static int uac_pcm_silence(struct snd_pcm_substream *substream,
			     int channel, snd_pcm_uframes_t pos,
			     snd_pcm_uframes_t count)
{
	return 0; /* do nothing */
}

static struct page *uac_pcm_page(struct snd_pcm_substream *substream,
				   unsigned long offset)
{
	return virt_to_page(uac_page[substream->stream]); /* the same page */
}

static struct snd_pcm_ops uac_pcm_ops = {
	.open =		uac_pcm_open,
	.close =	uac_pcm_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	uac_pcm_hw_params,
	.hw_free =	uac_pcm_hw_free,
	.prepare =	uac_pcm_prepare,
	.trigger =	uac_pcm_trigger,
	.pointer =	uac_pcm_pointer,
};

static struct snd_pcm_ops uac_pcm_ops_no_buf = {
	.open =		uac_pcm_open,
	.close =	uac_pcm_close,
	.ioctl =	snd_pcm_lib_ioctl,
	.hw_params =	uac_pcm_hw_params,
	.hw_free =	uac_pcm_hw_free,
	.prepare =	uac_pcm_prepare,
	.trigger =	uac_pcm_trigger,
	.pointer =	uac_pcm_pointer,
	.copy =		uac_pcm_copy,
	.silence =	uac_pcm_silence,
	.page =		uac_pcm_page,
};

static int snd_card_uac_pcm(struct snd_uac *uac, int device,
			      int substreams)
{
	struct snd_pcm *pcm;
	struct snd_pcm_ops *ops;
	int err;

	err = snd_pcm_new(uac->card, "Dummy Uac PCM", device,
			       substreams, substreams, &pcm);
	if (err < 0)
		return err;
	uac->pcm = pcm;
	if (fake_buffer)
		ops = &uac_pcm_ops_no_buf;
	else
		ops = &uac_pcm_ops;
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, ops);
	pcm->private_data = uac;
	pcm->info_flags = 0;
	strcpy(pcm->name, "Dummy Uac PCM");
	if (!fake_buffer) {
		snd_pcm_lib_preallocate_pages_for_all(pcm,
			SNDRV_DMA_TYPE_CONTINUOUS,
			snd_dma_continuous_data(GFP_KERNEL),
			0, 64*1024);
	}
	return 0;
}

/*
 * mixer interface
 */

#define UAC_VOLUME(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, \
  .access = SNDRV_CTL_ELEM_ACCESS_READWRITE | SNDRV_CTL_ELEM_ACCESS_TLV_READ, \
  .name = xname, .index = xindex, \
  .info = snd_uac_volume_info, \
  .get = snd_uac_volume_get, .put = snd_uac_volume_put, \
  .private_value = addr, \
  .tlv = { .p = db_scale_uac } }

static int snd_uac_volume_info(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 2;
	uinfo->value.integer.min = -50;
	uinfo->value.integer.max = 100;
	return 0;
}

static int snd_uac_volume_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);
	int addr = kcontrol->private_value;

	spin_lock_irq(&uac->mixer_lock);
	ucontrol->value.integer.value[0] = uac->mixer_volume[addr][0];
	ucontrol->value.integer.value[1] = uac->mixer_volume[addr][1];
	spin_unlock_irq(&uac->mixer_lock);
	return 0;
}

static int snd_uac_volume_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);
	int change, addr = kcontrol->private_value;
	int left, right;

	left = ucontrol->value.integer.value[0];
	if (left < -50)
		left = -50;
	if (left > 100)
		left = 100;
	right = ucontrol->value.integer.value[1];
	if (right < -50)
		right = -50;
	if (right > 100)
		right = 100;
	spin_lock_irq(&uac->mixer_lock);
	change = uac->mixer_volume[addr][0] != left ||
	         uac->mixer_volume[addr][1] != right;
	uac->mixer_volume[addr][0] = left;
	uac->mixer_volume[addr][1] = right;
	spin_unlock_irq(&uac->mixer_lock);
	return change;
}

static const DECLARE_TLV_DB_SCALE(db_scale_uac, -4500, 30, 0);

#define UAC_CAPSRC(xname, xindex, addr) \
{ .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, .index = xindex, \
  .info = snd_uac_capsrc_info, \
  .get = snd_uac_capsrc_get, .put = snd_uac_capsrc_put, \
  .private_value = addr }

#define snd_uac_capsrc_info	snd_ctl_boolean_stereo_info

static int snd_uac_capsrc_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);
	int addr = kcontrol->private_value;

	spin_lock_irq(&uac->mixer_lock);
	ucontrol->value.integer.value[0] = uac->capture_source[addr][0];
	ucontrol->value.integer.value[1] = uac->capture_source[addr][1];
	spin_unlock_irq(&uac->mixer_lock);
	return 0;
}

static int snd_uac_capsrc_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);
	int change, addr = kcontrol->private_value;
	int left, right;

	left = ucontrol->value.integer.value[0] & 1;
	right = ucontrol->value.integer.value[1] & 1;
	spin_lock_irq(&uac->mixer_lock);
	change = uac->capture_source[addr][0] != left &&
	         uac->capture_source[addr][1] != right;
	uac->capture_source[addr][0] = left;
	uac->capture_source[addr][1] = right;
	spin_unlock_irq(&uac->mixer_lock);
	return change;
}

static int snd_uac_iobox_info(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_info *info)
{
	const char *const names[] = { "None", "CD Player" };

	return snd_ctl_enum_info(info, 1, 2, names);
}

static int snd_uac_iobox_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *value)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);

	value->value.enumerated.item[0] = uac->iobox;
	return 0;
}

static int snd_uac_iobox_put(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *value)
{
	struct snd_uac *uac = snd_kcontrol_chip(kcontrol);
	int changed;

	if (value->value.enumerated.item[0] > 1)
		return -EINVAL;

	changed = value->value.enumerated.item[0] != uac->iobox;
	if (changed) {
		uac->iobox = value->value.enumerated.item[0];

		if (uac->iobox) {
			uac->cd_volume_ctl->vd[0].access &=
				~SNDRV_CTL_ELEM_ACCESS_INACTIVE;
			uac->cd_switch_ctl->vd[0].access &=
				~SNDRV_CTL_ELEM_ACCESS_INACTIVE;
		} else {
			uac->cd_volume_ctl->vd[0].access |=
				SNDRV_CTL_ELEM_ACCESS_INACTIVE;
			uac->cd_switch_ctl->vd[0].access |=
				SNDRV_CTL_ELEM_ACCESS_INACTIVE;
		}

		snd_ctl_notify(uac->card, SNDRV_CTL_EVENT_MASK_INFO,
			       &uac->cd_volume_ctl->id);
		snd_ctl_notify(uac->card, SNDRV_CTL_EVENT_MASK_INFO,
			       &uac->cd_switch_ctl->id);
	}

	return changed;
}

static struct snd_kcontrol_new snd_uac_controls[] = {
UAC_VOLUME("Master Volume", 0, MIXER_ADDR_MASTER),
UAC_CAPSRC("Master Capture Switch", 0, MIXER_ADDR_MASTER),
UAC_VOLUME("Synth Volume", 0, MIXER_ADDR_SYNTH),
UAC_CAPSRC("Synth Capture Switch", 0, MIXER_ADDR_SYNTH),
UAC_VOLUME("Line Volume", 0, MIXER_ADDR_LINE),
UAC_CAPSRC("Line Capture Switch", 0, MIXER_ADDR_LINE),
UAC_VOLUME("Mic Volume", 0, MIXER_ADDR_MIC),
UAC_CAPSRC("Mic Capture Switch", 0, MIXER_ADDR_MIC),
UAC_VOLUME("CD Volume", 0, MIXER_ADDR_CD),
UAC_CAPSRC("CD Capture Switch", 0, MIXER_ADDR_CD),
{
	.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
	.name  = "External I/O Box",
	.info  = snd_uac_iobox_info,
	.get   = snd_uac_iobox_get,
	.put   = snd_uac_iobox_put,
},
};

static int snd_card_uac_new_mixer(struct snd_uac *uac)
{
	struct snd_card *card = uac->card;
	struct snd_kcontrol *kcontrol;
	unsigned int idx;
	int err;

	spin_lock_init(&uac->mixer_lock);
	strcpy(card->mixername, "Dummy Uac Mixer");
	uac->iobox = 1;

	for (idx = 0; idx < ARRAY_SIZE(snd_uac_controls); idx++) {
		kcontrol = snd_ctl_new1(&snd_uac_controls[idx], uac);
		err = snd_ctl_add(card, kcontrol);
		if (err < 0)
			return err;
		if (!strcmp(kcontrol->id.name, "CD Volume"))
			uac->cd_volume_ctl = kcontrol;
		else if (!strcmp(kcontrol->id.name, "CD Capture Switch"))
			uac->cd_switch_ctl = kcontrol;

	}
	return 0;
}

#if defined(CONFIG_SND_DEBUG) && defined(CONFIG_PROC_FS)
/*
 * proc interface
 */
static void print_formats(struct snd_uac *uac,
			  struct snd_info_buffer *buffer)
{
	int i;

	for (i = 0; i < SNDRV_PCM_FORMAT_LAST; i++) {
		if (uac->pcm_hw.formats & (1ULL << i))
			snd_iprintf(buffer, " %s", snd_pcm_format_name(i));
	}
}

static void print_rates(struct snd_uac *uac,
			struct snd_info_buffer *buffer)
{
	static int rates[] = {
		5512, 8000, 11025, 16000, 22050, 32000, 44100, 48000,
		64000, 88200, 96000, 176400, 192000,
	};
	int i;

	if (uac->pcm_hw.rates & SNDRV_PCM_RATE_CONTINUOUS)
		snd_iprintf(buffer, " continuous");
	if (uac->pcm_hw.rates & SNDRV_PCM_RATE_KNOT)
		snd_iprintf(buffer, " knot");
	for (i = 0; i < ARRAY_SIZE(rates); i++)
		if (uac->pcm_hw.rates & (1 << i))
			snd_iprintf(buffer, " %d", rates[i]);
}

#define get_uac_int_ptr(uac, ofs) \
	(unsigned int *)((char *)&((uac)->pcm_hw) + (ofs))
#define get_uac_ll_ptr(uac, ofs) \
	(unsigned long long *)((char *)&((uac)->pcm_hw) + (ofs))

struct uac_hw_field {
	const char *name;
	const char *format;
	unsigned int offset;
	unsigned int size;
};
#define FIELD_ENTRY(item, fmt) {		   \
	.name = #item,				   \
	.format = fmt,				   \
	.offset = offsetof(struct snd_pcm_hardware, item), \
	.size = sizeof(uac_pcm_hardware.item) }

static struct uac_hw_field fields[] = {
	FIELD_ENTRY(formats, "%#llx"),
	FIELD_ENTRY(rates, "%#x"),
	FIELD_ENTRY(rate_min, "%d"),
	FIELD_ENTRY(rate_max, "%d"),
	FIELD_ENTRY(channels_min, "%d"),
	FIELD_ENTRY(channels_max, "%d"),
	FIELD_ENTRY(buffer_bytes_max, "%ld"),
	FIELD_ENTRY(period_bytes_min, "%ld"),
	FIELD_ENTRY(period_bytes_max, "%ld"),
	FIELD_ENTRY(periods_min, "%d"),
	FIELD_ENTRY(periods_max, "%d"),
};

static void uac_proc_read(struct snd_info_entry *entry,
			    struct snd_info_buffer *buffer)
{
	struct snd_uac *uac = entry->private_data;
	int i;

	for (i = 0; i < ARRAY_SIZE(fields); i++) {
		snd_iprintf(buffer, "%s ", fields[i].name);
		if (fields[i].size == sizeof(int))
			snd_iprintf(buffer, fields[i].format,
				*get_uac_int_ptr(uac, fields[i].offset));
		else
			snd_iprintf(buffer, fields[i].format,
				*get_uac_ll_ptr(uac, fields[i].offset));
		if (!strcmp(fields[i].name, "formats"))
			print_formats(uac, buffer);
		else if (!strcmp(fields[i].name, "rates"))
			print_rates(uac, buffer);
		snd_iprintf(buffer, "\n");
	}
}

static void uac_proc_write(struct snd_info_entry *entry,
			     struct snd_info_buffer *buffer)
{
	struct snd_uac *uac = entry->private_data;
	char line[64];

	while (!snd_info_get_line(buffer, line, sizeof(line))) {
		char item[20];
		const char *ptr;
		unsigned long long val;
		int i;

		ptr = snd_info_get_str(item, line, sizeof(item));
		for (i = 0; i < ARRAY_SIZE(fields); i++) {
			if (!strcmp(item, fields[i].name))
				break;
		}
		if (i >= ARRAY_SIZE(fields))
			continue;
		snd_info_get_str(item, ptr, sizeof(item));
		if (kstrtoull(item, 0, &val))
			continue;
		if (fields[i].size == sizeof(int))
			*get_uac_int_ptr(uac, fields[i].offset) = val;
		else
			*get_uac_ll_ptr(uac, fields[i].offset) = val;
	}
}

static void uac_proc_init(struct snd_uac *chip)
{
	struct snd_info_entry *entry;

	if (!snd_card_proc_new(chip->card, "uac_pcm", &entry)) {
		snd_info_set_text_ops(entry, chip, uac_proc_read);
		entry->c.text.write = uac_proc_write;
		entry->mode |= S_IWUSR;
		entry->private_data = chip;
	}
}
#else
#define uac_proc_init(x)
#endif /* CONFIG_SND_DEBUG && CONFIG_PROC_FS */

static int snd_uac_probe(struct platform_device *devptr)
{
	struct snd_card *card;
	struct snd_uac *uac;
	struct uac_model *m = NULL, **mdl;
	int idx, err;
	int dev = devptr->id;

	err = snd_card_new(&devptr->dev, index[dev], id[dev], THIS_MODULE,
			   sizeof(struct snd_uac), &card);
	if (err < 0)
		return err;
	uac = card->private_data;
	uac->card = card;
	for (mdl = uac_models; *mdl && model[dev]; mdl++) {
		if (strcmp(model[dev], (*mdl)->name) == 0) {
			printk(KERN_INFO
				"snd-uac: Using model '%s' for card %i\n",
				(*mdl)->name, card->number);
			m = uac->model = *mdl;
			break;
		}
	}
	for (idx = 0; idx < MAX_PCM_DEVICES && idx < pcm_devs[dev]; idx++) {
		if (pcm_substreams[dev] < 1)
			pcm_substreams[dev] = 1;
		if (pcm_substreams[dev] > MAX_PCM_SUBSTREAMS)
			pcm_substreams[dev] = MAX_PCM_SUBSTREAMS;
		err = snd_card_uac_pcm(uac, idx, pcm_substreams[dev]);
		if (err < 0)
			goto __nodev;
	}

	uac->pcm_hw = uac_pcm_hardware;
	if (m) {
		if (m->formats)
			uac->pcm_hw.formats = m->formats;
		if (m->buffer_bytes_max)
			uac->pcm_hw.buffer_bytes_max = m->buffer_bytes_max;
		if (m->period_bytes_min)
			uac->pcm_hw.period_bytes_min = m->period_bytes_min;
		if (m->period_bytes_max)
			uac->pcm_hw.period_bytes_max = m->period_bytes_max;
		if (m->periods_min)
			uac->pcm_hw.periods_min = m->periods_min;
		if (m->periods_max)
			uac->pcm_hw.periods_max = m->periods_max;
		if (m->rates)
			uac->pcm_hw.rates = m->rates;
		if (m->rate_min)
			uac->pcm_hw.rate_min = m->rate_min;
		if (m->rate_max)
			uac->pcm_hw.rate_max = m->rate_max;
		if (m->channels_min)
			uac->pcm_hw.channels_min = m->channels_min;
		if (m->channels_max)
			uac->pcm_hw.channels_max = m->channels_max;
	}

	err = snd_card_uac_new_mixer(uac);
	if (err < 0)
		goto __nodev;
	strcpy(card->driver, "Dummy Uac");
	strcpy(card->shortname, "Dummy Uac");
	sprintf(card->longname, "Dummy Uac %i", dev + 1);

	uac_proc_init(uac);

	err = snd_card_register(card);
	if (err == 0) {
		platform_set_drvdata(devptr, card);
		return 0;
	}
      __nodev:
	snd_card_free(card);
	return err;
}

static int snd_uac_remove(struct platform_device *devptr)
{
	snd_card_free(platform_get_drvdata(devptr));
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int snd_uac_suspend(struct device *pdev)
{
	struct snd_card *card = dev_get_drvdata(pdev);
	struct snd_uac *uac = card->private_data;

	snd_power_change_state(card, SNDRV_CTL_POWER_D3hot);
	snd_pcm_suspend_all(uac->pcm);
	return 0;
}

static int snd_uac_resume(struct device *pdev)
{
	struct snd_card *card = dev_get_drvdata(pdev);

	snd_power_change_state(card, SNDRV_CTL_POWER_D0);
	return 0;
}

static SIMPLE_DEV_PM_OPS(snd_uac_pm, snd_uac_suspend, snd_uac_resume);
#define SND_UAC_PM_OPS	&snd_uac_pm
#else
#define SND_UAC_PM_OPS	NULL
#endif

#define SND_UAC_DRIVER	"snd_uac"

static struct platform_driver snd_uac_driver = {
	.probe		= snd_uac_probe,
	.remove		= snd_uac_remove,
	.driver		= {
		.name	= SND_UAC_DRIVER,
		.owner	= THIS_MODULE,
		.pm	= SND_UAC_PM_OPS,
	},
};

static void snd_uac_unregister_all(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(devices); ++i)
		platform_device_unregister(devices[i]);
	platform_driver_unregister(&snd_uac_driver);
	free_fake_buffer();
}

int alsa_card_uac_init(void)
{
	int i, cards, err;
	err = platform_driver_register(&snd_uac_driver);
	if (err < 0)
		return err;

	err = alloc_fake_buffer();
	if (err < 0) {
		platform_driver_unregister(&snd_uac_driver);
		return err;
	}

	cards = 0;
	for (i = 0; i < SNDRV_CARDS; i++) {
		struct platform_device *device;
		if (! enable[i])
			continue;
		device = platform_device_register_simple(SND_UAC_DRIVER,
							 i, NULL, 0);
		if (IS_ERR(device))
			continue;
		if (!platform_get_drvdata(device)) {
			platform_device_unregister(device);
			continue;
		}
		devices[i] = device;
		cards++;
	}
	if (!cards) {
#ifdef MODULE
		printk(KERN_ERR "Dummy Uac soundcard not found or device busy\n");
#endif
		snd_uac_unregister_all();
		return -ENODEV;
	}
	return 0;
}

int alsa_card_uac_exit(void)
{
	snd_uac_unregister_all();
	return 0;
}
