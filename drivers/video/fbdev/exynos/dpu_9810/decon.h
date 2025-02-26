/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Header file for Exynos DECON driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef ___SAMSUNG_DECON_H__
#define ___SAMSUNG_DECON_H__

#include <linux/fb.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/pm_qos.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/platform_device.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-core.h>
#include <soc/samsung/bts.h>
#if defined(CONFIG_EXYNOS_ITMON)
#include <soc/samsung/exynos-itmon.h>
#endif
#if defined(CONFIG_ION_EXYNOS)
#include <linux/exynos_ion.h>
#include <linux/ion.h>
#include <linux/exynos_iovmm.h>
#endif
#ifdef CONFIG_SEC_ABC
#include <linux/sti/abc_common.h>
#endif

#ifdef CONFIG_EXYNOS_COMMON_PANEL
#include "../panel/panel_drv.h"
#endif

#include "regs-decon.h"

#include "./panels/decon_lcd.h"
#include "dsim.h"
#include "displayport.h"
#include "../../../../dma-buf/sync_debug.h"
#include "hdr_metadata.h"
#if defined(CONFIG_EXYNOS_COMMON_PANEL)
#include "disp_err.h"
#endif

#ifdef CONFIG_SUPPORT_DSU
#include "dsu.h"
#endif
#define MAX_DECON_CNT		3
#define SUCCESS_EXYNOS_SMC	0

extern struct ion_device *ion_exynos;
extern struct decon_device *decon_drvdata[MAX_DECON_CNT];
extern int decon_log_level;
extern int dpu_bts_log_level;
extern int win_update_log_level;
extern int decon_systrace_enable;
extern struct decon_bts_ops decon_bts_control;

#define DECON_MODULE_NAME	"exynos-decon"
#define MAX_NAME_SIZE		32
#define MAX_PLANE_CNT		3
#define MAX_PLANE_ADDR_CNT	4
#define DECON_ENTER_HIBER_CNT	3
#define DECON_ENTER_LPD_CNT	3
#define MIN_BLK_MODE_WIDTH	144
#define MIN_BLK_MODE_HEIGHT	16
#define VSYNC_TIMEOUT_MSEC	200
#define DEFAULT_BPP		32
#define MAX_DECON_WIN		6
#define MAX_DPP_SUBDEV		6
#define MIN_WIN_BLOCK_WIDTH	8
#define MIN_WIN_BLOCK_HEIGHT	1
#define FD_TRY_CNT		3
#define VALID_FD_VAL		3
#define DECON_TRACE_BUF_SIZE	40
#define CHIP_VER	(9810)

#define DECON_WIN_UPDATE_IDX	(6)

#ifndef KHZ
#define KHZ (1000)
#endif
#ifndef MHZ
#define MHZ (1000*1000)
#endif
#ifndef MSEC
#define MSEC (1000)
#endif

#define FHD (1080 * 1920)

#define SHADOW_UPDATE_TIMEOUT	(300 * 1000) /* 300ms */
#define IDLE_WAIT_TIMEOUT	(50 * 1000) /* 50ms */
#define RUN_WAIT_TIMEOUT	IDLE_WAIT_TIMEOUT
#define CEIL(x)			((x-(u32)(x) > 0 ? (u32)(x+1) : (u32)(x)))
#define DSC_INIT_XMIT_DELAY	0x200

#define EINT_PEND(x)		((x == 0) ? 2 : ((x == 1) ? 4 : 1))

#define MAX_DSC_SLICE_CNT	4

void dpu_debug_printk(const char *function_name, const char *format, ...);

#define decon_err(fmt, ...)							\
	do {									\
		if (decon_log_level >= 3) {					\
			pr_err(pr_fmt(fmt), ##__VA_ARGS__);			\
		}								\
	} while (0)

#define decon_warn(fmt, ...)							\
	do {									\
		if (decon_log_level >= 4) {					\
			pr_warn(pr_fmt(fmt), ##__VA_ARGS__);			\
		}								\
	} while (0)

#define decon_info(fmt, ...)							\
	do {									\
		if (decon_log_level >= 6)					\
			pr_info(pr_fmt(fmt), ##__VA_ARGS__);			\
	} while (0)

#define decon_dbg(fmt, ...)							\
	do {									\
		if (decon_log_level >= 7)					\
			pr_info(pr_fmt(fmt), ##__VA_ARGS__);			\
	} while (0)

#define DPU_DEBUG_WIN(fmt, args...)						\
	do {									\
		if (win_update_log_level >= 7)					\
			dpu_debug_printk("WIN_UPDATE", fmt,  ##args);		\
	} while (0)

#define DPU_DEBUG_BTS(fmt, args...)						\
	do {									\
		if (dpu_bts_log_level >= 7)					\
			dpu_debug_printk("BTS", fmt,  ##args);			\
	} while (0)

#define DPU_INFO_BTS(fmt, args...)						\
	do {									\
		if (dpu_bts_log_level >= 6)					\
			dpu_debug_printk("BTS", fmt,  ##args);			\
	} while (0)

#define DPU_ERR_BTS(fmt, args...)						\
	do {									\
		if (dpu_bts_log_level >= 3)					\
			dpu_debug_printk("BTS", fmt, ##args);			\
	} while (0)


/* DECON systrace related */
void tracing_mark_write(struct decon_device *decon, char id, char *str1, int value);
#define decon_systrace(decon, id, str1, value)					\
	do {									\
		if (decon_systrace_enable)					\
			tracing_mark_write(decon, id, str1, value);		\
	} while (0)

enum decon_trig_mode {
	DECON_HW_TRIG = 0,
	DECON_SW_TRIG
};

enum decon_out_type {
	DECON_OUT_DSI = 0,
	DECON_OUT_EDP,
	DECON_OUT_DP,
};

enum decon_dsi_mode {
	DSI_MODE_SINGLE = 0,
	DSI_MODE_DUAL_DSI,
	DSI_MODE_DUAL_DISPLAY,
	DSI_MODE_NONE
};

enum decon_hold_scheme {
	/*  should be set to this value in case of DSIM video mode */
	DECON_VCLK_HOLD_ONLY		= 0x00,
	/*  should be set to this value in case of DSIM command mode */
	DECON_VCLK_RUNNING_VDEN_DISABLE = 0x01,
	DECON_VCLK_HOLD_VDEN_DISABLE	= 0x02,
	/*  should be set to this value in case of HDMI, eDP */
	DECON_VCLK_NOT_AFFECTED		= 0x03,
};

enum decon_rgb_order {
	DECON_RGB = 0x0,
	DECON_GBR = 0x1,
	DECON_BRG = 0x2,
	DECON_BGR = 0x4,
	DECON_RBG = 0x5,
	DECON_GRB = 0x6,
};

enum decon_win_func {
	PD_FUNC_CLEAR			= 0x0,
	PD_FUNC_COPY			= 0x1,
	PD_FUNC_DESTINATION		= 0x2,
	PD_FUNC_SOURCE_OVER		= 0x3,
	PD_FUNC_DESTINATION_OVER	= 0x4,
	PD_FUNC_SOURCE_IN		= 0x5,
	PD_FUNC_DESTINATION_IN		= 0x6,
	PD_FUNC_SOURCE_OUT		= 0x7,
	PD_FUNC_DESTINATION_OUT		= 0x8,
	PD_FUNC_SOURCE_A_TOP		= 0x9,
	PD_FUNC_DESTINATION_A_TOP	= 0xa,
	PD_FUNC_XOR			= 0xb,
	PD_FUNC_PLUS			= 0xc,
	PD_FUNC_USER_DEFINED		= 0xd,
};

enum decon_win_alpha_coef {
	BND_COEF_ZERO			= 0x0,
	BND_COEF_ONE			= 0x1,
	BND_COEF_AF			= 0x2,
	BND_COEF_1_M_AF		= 0x3,
	BND_COEF_AB			= 0x4,
	BND_COEF_1_M_AB		= 0x5,
	BND_COEF_PLNAE_ALPHA0		= 0x6,
	BND_COEF_1_M_PLNAE_ALPHA0	= 0x7,
	BND_COEF_PLNAE_ALPHA1		= 0x8,
	BND_COEF_1_M_PLNAE_ALPHA1	= 0x9,
	BND_COEF_ALPHA_MULT		= 0xA,
	BND_COEF_1_M_ALPHA_MULT	= 0xB,
};

enum decon_win_alpha_sel {
	ALPHA_MULT_SRC_SEL_ALPHA0 = 0,
	ALPHA_MULT_SRC_SEL_ALPHA1 = 1,
	ALPHA_MULT_SRC_SEL_AF = 2,
	ALPHA_MULT_SRC_SEL_AB = 3,
};

enum decon_fifo_mode {
	DECON_FIFO_00K = 0,
	DECON_FIFO_04K,
	DECON_FIFO_08K,
	DECON_FIFO_12K,
	DECON_FIFO_16K,
};

enum decon_merger_mode {
	DECON_LRM_NO		= 0x0,
	DECON_LRM_NOSWAP_RF	= 0x4,
	DECON_LRM_NOSWAP_LF	= 0x5,
	DECON_LRM_SWAP_RF	= 0x6,
	DECON_LRM_SWAP_LF	= 0x7,
};

enum decon_te_src {
	DECON_TE_FROM_DDI0 = 0,
	DECON_TE_FROM_DDI1,
	DECON_TE_FROM_DDI2,
	DECON_TE_FROM_USB,
};

enum decon_set_trig {
	DECON_TRIG_DISABLE = 0,
	DECON_TRIG_ENABLE
};

enum decon_idma_type {
	IDMA_G0 = 0,
	IDMA_G1,
	IDMA_VG0,
	IDMA_VG1,
	IDMA_VGF0,
	IDMA_VGF1, /* VGRF in case of Exynos9810 */
	MAX_DECON_DMA_TYPE,
};

/*
 * DECON_STATE_ON : disp power on, decon/dsim clock on & lcd on
 * DECON_HIBER : disp power off, decon/dsim clock off & lcd on
 * DECON_STATE_OFF : disp power off, decon/dsim clock off & lcd off
 */
enum decon_state {
	DECON_STATE_INIT = 0,
	DECON_STATE_ON,
#ifdef CONFIG_SUPPORT_DOZE
	DECON_STATE_DOZE,
#endif
	DECON_STATE_HIBER,
#ifdef CONFIG_SUPPORT_DOZE
	DECON_STATE_DOZE_SUSPEND,
#endif
	DECON_STATE_OFF,
	DECON_STATE_TUI,
};

/* To find a proper CLOCK ratio */
enum decon_clk_id {
	CLK_ID_VCLK = 0,
	CLK_ID_ECLK,
	CLK_ID_ACLK,
	CLK_ID_PCLK,
	CLK_ID_DPLL, /* DPU_PLL */
	CLK_ID_RESOLUTION,
	CLK_ID_MIC_RATIO,
	CLK_ID_DSC_RATIO,
	CLK_ID_MAX,
};

enum decon_path_cfg {
	PATH_CON_ID_DSIM_IF0 = 0,
	PATH_CON_ID_DSIM_IF1 = 1,
	PATH_CON_ID_DP = 3,
	PATH_CON_ID_DUAL_DSC = 4,
	PATH_CON_ID_DSCC_EN = 7,
};

enum decon_data_path {
	/* No comp - OUTFIFO0 DSIM_IF0 */
	DPATH_NOCOMP_OUTFIFO0_DSIMIF0			= 0x001,
	/* No comp - FF0 - FORMATTER1 - DSIM_IF1 */
	DPATH_NOCOMP_OUTFIFO0_DSIMIF1			= 0x002,
	/* No comp - SPLITTER - FF0/1 - FORMATTER0/1 - DSIM_IF0/1 */
	DPATH_NOCOMP_SPLITTER_OUTFIFO01_DSIMIF01	= 0x003,

	/* DSC_ENC0 - OUTFIFO0 - DSIM_IF0 */
	DPATH_DSCENC0_OUTFIFO0_DSIMIF0		= 0x011,
	/* DSC_ENC0 - OUTFIFO0 - DSIM_IF1 */
	DPATH_DSCENC0_OUTFIFO0_DSIMIF1		= 0x012,

	/* DSCC,DSC_ENC0/1 - OUTFIFO01 DSIM_IF0 */
	DPATH_DSCC_DSCENC01_OUTFIFO01_DSIMIF0	= 0x0B1,
	/* DSCC,DSC_ENC0/1 - OUTFIFO01 DSIM_IF1 */
	DPATH_DSCC_DSCENC01_OUTFIFO01_DSIMIF1	= 0x0B2,
	/* DSCC,DSC_ENC0/1 - OUTFIFO01 DSIM_IF0/1*/
	DPATH_DSCC_DSCENC01_OUTFIFO01_DSIMIF01	= 0x0B3,

	/* No comp - OUTFIFO0 DSIM_IF0 */
	DECON1_NOCOMP_OUTFIFO0_DSIMIF0	= 0x001,
	/* No comp - OUTFIFO0 DP_IF */
	DECON1_NOCOMP_OUTFIFO0_DPIF		= 0x008,
	/* DSC_ENC1 - OUTFIFO0 - DSIM_IF0 */
	DECON1_DSCENC1_OUTFIFO0_DSIMIF0	= 0x021,
	/* DSC_ENC1 - OUTFIFO0 - DP_IF */
	DECON1_DSCENC1_OUTFIFO0_DPIF	= 0x028,

	/* No comp - OUTFIFO0 DP_IF */
	DECON2_NOCOMP_OUTFIFO0_DPIF		= 0x008,
	/* DSC_ENC2 - OUTFIFO0 - DP_IF0 */
	DECON2_DSCENC2_OUTFIFO0_DPIF	= 0x048,
};

enum decon_dsc_id {
	DECON_DSC_ENC0 = 0x0,
	DECON_DSC_ENC1 = 0x1,
	DECON_DSC_ENC2 = 0x2,
};

enum decon_scaler_path {
	SCALERPATH_OFF	= 0x0,
	SCALERPATH_VGF	= 0x1,
	SCALERPATH_VGRF	= 0x2,
};

enum decon_share_path {
	SHAREPATH_DQE_USE		= 0x0,
	SHAREPATH_VG0_USE		= 0x1,
	SHAREPATH_VG1_USE		= 0x2,
	SHAREPATH_VGF1_USE		= 0x3,
	SHAREPATH_VGF0_USE		= 0x4,
};

enum decon_pixel_format {
	/* RGB 8bit display */
	/* 4byte */
	DECON_PIXEL_FORMAT_ARGB_8888 = 0,
	DECON_PIXEL_FORMAT_ABGR_8888,
	DECON_PIXEL_FORMAT_RGBA_8888,
	DECON_PIXEL_FORMAT_BGRA_8888,
	DECON_PIXEL_FORMAT_XRGB_8888,
	DECON_PIXEL_FORMAT_XBGR_8888,
	DECON_PIXEL_FORMAT_RGBX_8888,
	DECON_PIXEL_FORMAT_BGRX_8888,
	/* 2byte */
	DECON_PIXEL_FORMAT_RGBA_5551,
	DECON_PIXEL_FORMAT_BGRA_5551,
	DECON_PIXEL_FORMAT_ABGR_4444,
	DECON_PIXEL_FORMAT_RGBA_4444,
	DECON_PIXEL_FORMAT_BGRA_4444,
	DECON_PIXEL_FORMAT_RGB_565,
	DECON_PIXEL_FORMAT_BGR_565,

	/* RGB 10bit display */
	/* 4byte */
	DECON_PIXEL_FORMAT_ARGB_2101010,
	DECON_PIXEL_FORMAT_ABGR_2101010,
	DECON_PIXEL_FORMAT_RGBA_1010102,
	DECON_PIXEL_FORMAT_BGRA_1010102,

	/* YUV 8bit display */
	/* YUV422 2P */
	DECON_PIXEL_FORMAT_NV16,
	DECON_PIXEL_FORMAT_NV61,
	/* YUV422 3P */
	DECON_PIXEL_FORMAT_YVU422_3P,
	/* YUV420 2P */
	DECON_PIXEL_FORMAT_NV12,
	DECON_PIXEL_FORMAT_NV21,
	DECON_PIXEL_FORMAT_NV12M,
	DECON_PIXEL_FORMAT_NV21M,
	/* YUV420 3P */
	DECON_PIXEL_FORMAT_YUV420,
	DECON_PIXEL_FORMAT_YVU420,
	DECON_PIXEL_FORMAT_YUV420M,
	DECON_PIXEL_FORMAT_YVU420M,
	/* YUV - 2 planes but 1 buffer */
	DECON_PIXEL_FORMAT_NV12N,
	DECON_PIXEL_FORMAT_NV12N_10B,

	/* YUV 10bit display */
	/* YUV420 2P */
	DECON_PIXEL_FORMAT_NV12M_P010,
	DECON_PIXEL_FORMAT_NV21M_P010,

	/* YUV420(P8+2) 4P */
	DECON_PIXEL_FORMAT_NV12M_S10B,
	DECON_PIXEL_FORMAT_NV21M_S10B,

	DECON_PIXEL_FORMAT_MAX,
};

enum decon_blending {
	DECON_BLENDING_NONE = 0,
	DECON_BLENDING_PREMULT = 1,
	DECON_BLENDING_COVERAGE = 2,
	DECON_BLENDING_MAX = 3,
};

enum dpp_rotate {
	DPP_ROT_NORMAL = 0x0,
	DPP_ROT_XFLIP,
	DPP_ROT_YFLIP,
	DPP_ROT_180,
	DPP_ROT_90,
	DPP_ROT_90_XFLIP,
	DPP_ROT_90_YFLIP,
	DPP_ROT_270,
};

enum dpp_csc_eq {
	/* eq_mode : 6bits [5:0] */
	CSC_STANDARD_SHIFT = 0,
	CSC_BT_601 = 0,
	CSC_BT_709 = 1,
	CSC_BT_2020 = 2,
	CSC_DCI_P3 = 3,
	CSC_STANDARD_UNSPECIFIED = 63,
	/* eq_mode : 3bits [8:6] */
	CSC_RANGE_SHIFT = 6,
	CSC_RANGE_LIMITED = 0x0,
	CSC_RANGE_FULL = 0x1,
	CSC_RANGE_UNSPECIFIED = 7,
};

enum dpp_comp_src {
	DPP_COMP_SRC_NONE = 0,
	DPP_COMP_SRC_G2D,
	DPP_COMP_SRC_GPU
};

enum dpp_hdr_standard {
	DPP_HDR_OFF = 0,
	DPP_HDR_ST2084,
	DPP_HDR_HLG,
};

struct decon_clocks {
	unsigned long decon[CLK_ID_DPLL + 1];
};

struct decon_mode_info {
	enum decon_psr_mode psr_mode;
	enum decon_trig_mode trig_mode;
	enum decon_out_type out_type;
	enum decon_dsi_mode dsi_mode;
};

struct decon_param {
	struct decon_mode_info psr;
	struct decon_lcd *lcd_info;
	u32 nr_windows;
	void __iomem *disp_ss_regs;
};

struct decon_window_regs {
	u32 wincon;
	u32 start_pos;
	u32 end_pos;
	u32 colormap;
	u32 start_time;
	u32 pixel_count;
	u32 whole_w;
	u32 whole_h;
	u32 offset_x;
	u32 offset_y;
	u32 winmap_state;
	enum decon_idma_type type;
	int plane_alpha;
	enum decon_pixel_format format;
	enum decon_blending blend;
};

struct decon_dma_buf_data {
	struct ion_handle		*ion_handle;
	struct dma_buf			*dma_buf;
	struct dma_buf_attachment	*attachment;
	struct sg_table			*sg_table;
	dma_addr_t			dma_addr;
	struct sync_file		*fence;
};

struct decon_win_rect {
	int x;
	int y;
	u32 w;
	u32 h;
};

struct decon_rect {
	u32 left;
	u32 top;
	u32 right;
	u32 bottom;
};

struct dpp_params {
	dma_addr_t addr[MAX_PLANE_CNT];
	enum dpp_rotate rot;
	enum dpp_csc_eq eq_mode;
	enum dpp_comp_src comp_src;
	enum dpp_hdr_standard hdr_std;
	u32 min_luminance;
	u32 max_luminance;
};

struct decon_frame {
	int x;
	int y;
	u32 w;
	u32 h;
	u32 f_w;
	u32 f_h;
};

struct decon_win_config {
	enum {
		DECON_WIN_STATE_DISABLED = 0,
		DECON_WIN_STATE_COLOR,
		DECON_WIN_STATE_BUFFER,
		DECON_WIN_STATE_UPDATE,
		DECON_WIN_STATE_CURSOR,
	} state;

	/* Reusability:This struct is used for IDMA and ODMA */
	union {
		__u32 color;
		struct {
			int				fd_idma[3];
			int				acq_fence;
			int				rel_fence;
			int				plane_alpha;
			enum decon_blending		blending;
			enum decon_idma_type		idma_type;
			enum decon_pixel_format		format;
			struct dpp_params		dpp_parm;
			/* no read area of IDMA */
			struct decon_win_rect		block_area;
			struct decon_win_rect		transparent_area;
			struct decon_win_rect		opaque_area;
			/* source framebuffer coordinates */
			struct decon_frame		src;
		};
#ifdef CONFIG_SUPPORT_DSU
		struct {
			int left;
			int top;
			int right;
			int bottom;
			int mode;
		};
#endif
	};
	/* destination OSD coordinates */
	struct decon_frame dst;
	bool protection;
	bool compression;
};

struct decon_reg_data {
	u32 num_of_window;
	int plane_cnt[MAX_DECON_WIN + 1];
	struct list_head list;
	struct decon_rect blender_bg;
	struct decon_win_config dpp_config[MAX_DECON_WIN + 1];
	struct decon_win_rect block_rect[MAX_DECON_WIN];
	struct decon_window_regs win_regs[MAX_DECON_WIN];
	struct decon_dma_buf_data dma_buf_data[MAX_DECON_WIN + 1][MAX_PLANE_CNT];

	/*
	 * If window update size is changed, that size has to be applied to
	 * DECON, DSIM and panel in case of below
	 * - full size -> partial size
	 * - partial size -> different partial size
	 * - partial size -> full size
	 *
	 * need_update flag indicates whether changes are applied to hw or not
	 */
	bool need_update;
	/* current update region */
	struct decon_rect up_region;
	/* protected contents playback */
	bool protection[MAX_DECON_WIN + 1];
	/* cursor async */
	bool is_cursor_win[MAX_DECON_WIN];
	int cursor_win;
#ifdef CONFIG_SUPPORT_DSU
	struct dsu_info dsu;
#endif
};

struct decon_win_config_extra {
	int remained_frames;
	u32 reserved[7];
};

struct decon_win_config_data_old {
	int	retire_fence;
	int	fd_odma;
	struct decon_win_config config[MAX_DECON_WIN + 1];
};

struct decon_win_config_data {
	int	retire_fence;
	int	fd_odma;
#ifdef CONFIG_SUPPORT_DSU
	struct decon_win_config config[MAX_DECON_WIN + 2];
#else
	struct decon_win_config config[MAX_DECON_WIN + 1];
#endif
	struct decon_win_config_extra extra;
};

enum hwc_ver {
	HWC_INIT = 0,
	HWC_1_0 = 1,
	HWC_2_0 = 2,
};

struct decon_disp_info {
	enum hwc_ver ver;
	enum decon_psr_mode psr_mode;
	struct lcd_mres_info mres_info;
	u32 chip_ver;
	unsigned char reverved[128];
};

struct dpu_size_info {
	u32 w_in;
	u32 h_in;
	u32 w_out;
	u32 h_out;
};

#ifdef CONFIG_DECON_EVENT_LOG
/**
 * Display Subsystem event management status.
 *
 * These status labels are used internally by the DECON to indicate the
 * current status of a device with operations.
 */
typedef enum dpu_event_type {
	/* Related with FB interface */
	DPU_EVT_BLANK = 0,
	DPU_EVT_UNBLANK,
	DPU_EVT_ACT_VSYNC,
	DPU_EVT_DEACT_VSYNC,
	DPU_EVT_WIN_CONFIG,
	DPU_EVT_DISP_INFO,

	/* Related with interrupt */
	DPU_EVT_TE_INTERRUPT,
	DPU_EVT_UNDERRUN,
	DPU_EVT_DECON_FRAMEDONE,
	DPU_EVT_DSIM_FRAMEDONE,
	DPU_EVT_RSC_CONFLICT,

	/* Related with async event */
	DPU_EVT_UPDATE_HANDLER,
	DPU_EVT_DSIM_COMMAND,
	DPU_EVT_TRIG_MASK,
	DPU_EVT_TRIG_UNMASK,
	DPU_EVT_FENCE_RELEASE,
	DPU_EVT_DECON_FRAMEDONE_WAIT,
	DPU_EVT_DECON_SHUTDOWN,
	DPU_EVT_DSIM_SHUTDOWN,
	DPU_EVT_DECON_FRAMESTART,

	/* Related with DPP */
	DPU_EVT_DPP_WINCON,
	DPU_EVT_DPP_FRAMEDONE,
	DPU_EVT_DPP_STOP,
	DPU_EVT_DPP_UPDATE_DONE,
	DPU_EVT_DPP_SHADOW_UPDATE,
	DPU_EVT_DPP_SUSPEND,
	DPU_EVT_DPP_RESUME,

	/* Related with PM */
	DPU_EVT_DECON_SUSPEND,
	DPU_EVT_DECON_RESUME,
	DPU_EVT_ENTER_HIBER,
	DPU_EVT_EXIT_HIBER,
	DPU_EVT_DSIM_SUSPEND,
	DPU_EVT_DSIM_RESUME,
	DPU_EVT_ENTER_ULPS,
	DPU_EVT_EXIT_ULPS,
	DPU_EVT_DSIM_PHY_ON,
	DPU_EVT_DSIM_PHY_OFF,

	DPU_EVT_LINECNT_ZERO,

	/* write-back events */
	DPU_EVT_WB_SET_BUFFER,
	DPU_EVT_WB_SW_TRIGGER,

	DPU_EVT_DMA_FRAMEDONE,
	DPU_EVT_DMA_RECOVERY,

	DPU_EVT_DECON_SET_BUFFER,
	/* cursor async */
	DPU_EVT_CURSOR_POS,
	DPU_EVT_CURSOR_UPDATE,

	/* window update */
	DPU_EVT_WINUP_UPDATE_REGION,
	DPU_EVT_WINUP_FLAGS,
	DPU_EVT_WINUP_APPLY_REGION,
#ifdef CONFIG_SUPPORT_DOZE
	DPU_EVT_DOZE,
	DPU_EVT_DOZE_SUSPEND,
#endif
	DPU_EVT_MAX, /* End of EVENT */
} dpu_event_t;

/* Related with Cursor */
struct disp_log_cursor {
	u32 xpos;
	u32 ypos;
	ktime_t elapsed;	/* End time - Start time */
};

/* Related with Fence */
struct disp_log_fence {
	u32 timeline_value;
	int timeline_max;
};

/* Related with PM */
struct disp_log_pm {
	u32 pm_status;		/* ACTIVE(1) or SUSPENDED(0) */
	ktime_t elapsed;	/* End time - Start time */
};

/* Related with S3CFB_WIN_CONFIG */
struct decon_update_reg_data {
	struct decon_window_regs 	win_regs[MAX_DECON_WIN];
	struct decon_win_config 	win_config[MAX_DECON_WIN + 1];
	struct decon_win_rect 		win;
};

/* Related with MIPI COMMAND read/write */
#define DPU_CALLSTACK_MAX 10
struct dsim_log_cmd_buf {
	u32 id;
	u8 buf;
	void *caller[DPU_CALLSTACK_MAX];
};

/* Related with DPP */
struct disp_log_dpp {
	u32 id;
	u32 start_cnt;
	u32 done_cnt;
	u32 comp_src;
	struct decon_frame src;
	struct decon_frame dst;
};

/* Related with window update information */
struct disp_log_winup {
	struct decon_frame req_region;
	struct decon_frame adj_region;
	struct decon_frame apl_region;
	bool need_update;
	bool reconfigure;
};

/**
 * struct dpu_log - Display Subsystem Log
 * This struct includes DECON/DSIM/DPP
 */
struct dpu_log {
	ktime_t time;
	dpu_event_t type;
	union {
		struct disp_log_dpp dpp;
		struct decon_update_reg_data reg;
		struct dsim_log_cmd_buf cmd_buf;
		struct disp_log_pm pm;
		struct disp_log_fence fence;
		struct disp_log_cursor cursor;
		struct disp_log_winup winup;
	} data;
};

struct dpu_size_err_info {
	ktime_t time;
	struct dpu_size_info info;
};

/* Definitions below are used in the DECON */
#define	DPU_EVENT_LOG_MAX	SZ_512
#define	DPU_EVENT_PRINT_MAX	(DPU_EVENT_LOG_MAX >> 1)
typedef enum dpu_event_log_level_type {
	DPU_EVENT_LEVEL_LOW = 0,
	DPU_EVENT_LEVEL_HIGH,
} dpu_log_level_t;

/* APIs below are used in the DECON/DSIM/DPP driver */
#define DPU_EVENT_START() ktime_t start = ktime_get()
void DPU_EVENT_LOG(dpu_event_t type, struct v4l2_subdev *sd, ktime_t time);
void DPU_EVENT_LOG_WINCON(struct v4l2_subdev *sd, struct decon_reg_data *regs);
void DPU_EVENT_LOG_CMD(struct v4l2_subdev *sd, u32 cmd_id, unsigned long data);
void DPU_EVENT_LOG_CURSOR(struct v4l2_subdev *sd, struct decon_reg_data *regs); /* cursor async */
void DPU_EVENT_LOG_UPDATE_REGION(struct v4l2_subdev *sd,
		struct decon_frame *req_region, struct decon_frame *adj_region);
void DPU_EVENT_LOG_WINUP_FLAGS(struct v4l2_subdev *sd, bool need_update,
		bool reconfigure);
void DPU_EVENT_LOG_APPLY_REGION(struct v4l2_subdev *sd,
		struct decon_rect *apl_rect);
void DPU_EVENT_SHOW(struct seq_file *s, struct decon_device *decon);
int decon_create_debugfs(struct decon_device *decon);
void decon_destroy_debugfs(struct decon_device *decon);
#else /*!*/
#define DPU_EVENT_START(...) do { } while(0)
#define DPU_EVENT_LOG(...) do { } while(0)
#define DPU_EVENT_LOG_WINCON(...) do { } while(0)
#define DPU_EVENT_LOG_CMD(...) do { } while(0)
#define DPU_EVENT_LOG_CURSOR(...) do { } while (0)
#define DPU_EVENT_LOG_UPDATE_REGION(...) do { } while(0)
#define DPU_EVENT_LOG_WINUP_FLAGS(...) do { } while(0)
#define DPU_EVENT_LOG_APPLY_REGION(...) do { } while(0)
#define DPU_EVENT_SHOW(...) do { } while(0)
#define decon_create_debugfs(...) do { } while(0)
#define decon_destroy_debugfs(..) do { } while(0)
#endif


#define REQ_DSI_DUMP	1
#define IGN_DSI_DUMP	0

/* HDR information of panel */
enum decon_hdr_type {
	HDR_NONE = 0,
	HDR_DOLBY_VISION = 1,
	HDR_HDR10 = 2,
	HDR_HLG = 3,
};

#define HDR_CAPA_NUM		4

struct decon_hdr_capabilities {
	unsigned int out_types[HDR_CAPA_NUM];
};

struct decon_hdr_capabilities_info {
	int out_num;
	int max_luminance;
	int max_average_luminance;
	int min_luminance;
};

struct decon_resources {
	int irq;
	void __iomem *regs;
	void __iomem *ss_regs;
	struct clk *aclk;
	struct clk *dpll;
	struct clk *pclk;
	struct clk *eclk;
	struct clk *eclk_leaf;
	struct clk *vclk;
	struct clk *vclk_leaf;
	struct clk *busp;
	struct clk *busd;
	struct clk *busc;
	struct clk *core;
	struct pinctrl *pinctrl;
	struct pinctrl_state *hw_te_on;
	struct pinctrl_state *hw_te_off;
};

struct decon_dt_info {
	enum decon_psr_mode psr_mode;
	enum decon_trig_mode trig_mode;
	enum decon_dsi_mode dsi_mode;
	enum decon_out_type out_type;
	int out_idx[MAX_DSIM_CNT];
	int max_win;
	int dft_win;
	int dft_idma;
};

struct decon_win {
	struct decon_device *decon;
	struct fb_info *fbinfo;

	struct fb_videomode videomode;
	struct decon_dma_buf_data dma_buf_data[MAX_PLANE_CNT];
#if defined(CONFIG_FB_TEST)
	struct decon_dma_buf_data fb_buf_data;
#endif
	int plane_cnt;

	int idx;
	int dpp_id;
	u32 pseudo_palette[16];
};
/* cursor async */
struct decon_user_window {
	int x;
	int y;
};

struct dpu_afbc_info {
	dma_addr_t dma_addr[2];
	bool is_afbc[2];
	void *v_addr[2];
	int size[2];
};

struct decon_debug {
	void __iomem *eint_pend;
	struct dentry *debug_root;
#ifdef CONFIG_DECON_EVENT_LOG
	struct dentry *debug_event;
	struct dentry *debug_dump;
	struct dentry *debug_bts;
	struct dentry *debug_win;
	struct dentry *debug_systrace;
#if defined(CONFIG_DSIM_CMD_TEST)
	struct dentry *debug_cmd;
#endif
	struct dentry *debug_recovery_cnt;
	struct dentry *debug_cmd_lp_ref;

	struct dpu_log *event_log;
	u32 event_log_cnt;
	atomic_t event_log_idx;
	dpu_log_level_t event_log_level;
#endif
	struct dpu_afbc_info prev_afbc_info;
	struct dpu_afbc_info cur_afbc_info;
	struct ion_handle *handle[MAX_DECON_WIN][MAX_PLANE_CNT];
	int prev_vgf_win_id[2];
};

struct decon_update_regs {
	struct mutex lock;
	struct list_head list;
	struct list_head saved_list;
	struct task_struct *thread;
	struct kthread_worker worker;
	struct kthread_work work;
	atomic_t remaining_frame;
};

struct decon_vsync {
	wait_queue_head_t wait;
	ktime_t timestamp;
	bool active;
	int irq_refcount;
	struct mutex lock;
	struct task_struct *thread;
};

#if defined(CONFIG_EXYNOS_COMMON_PANEL)
struct decon_fsync {
	wait_queue_head_t wait;
	ktime_t timestamp;
	bool active;
	int irq_refcount;
	struct mutex lock;
	struct task_struct *thread;
};
#endif

struct decon_hiber {
#if defined(CONFIG_EXYNOS_HIBERNATION_THREAD)
	wait_queue_head_t wait;
#endif
	ktime_t timestamp;
	struct mutex lock;
	struct task_struct *thread;
	struct kthread_worker worker;
	struct kthread_work work;
	atomic_t trig_cnt;
	atomic_t block_cnt;
	bool init_status;
	void __iomem *cam_status;
	u32 enter_cnt;
	u32 exit_cnt;
};

struct decon_win_update {
	bool enabled;
	u32 rect_w;
	u32 rect_h;
	u32 hori_cnt;
	u32 verti_cnt;
	/* previous update region */
	struct decon_rect prev_up_region;
};

struct decon_bts_ops {
	void (*bts_init)(struct decon_device *decon);
	void (*bts_calc_bw)(struct decon_device *decon,
			struct decon_reg_data *regs);
	void (*bts_update_bw)(struct decon_device *decon,
			struct decon_reg_data *regs, u32 is_after);
	void (*bts_acquire_bw)(struct decon_device *decon);
	void (*bts_release_bw)(struct decon_device *decon);
	void (*bts_deinit)(struct decon_device *decon);
};

struct decon_bts {
	bool enabled;
	u32 resol_clk;
	u32 bw[BTS_DPP_MAX];
	/* each decon must know other decon's BW to get overall BW */
	u32 ch_bw[3][BTS_DPU_MAX];
	u32 peak;
	u32 total_bw;
	u32 prev_total_bw;
	u32 max_disp_freq;
	u32 prev_max_disp_freq;
	enum bts_bw_type type;
	struct decon_bts_ops *ops;
	struct bts_decon_info bts_info;
	struct pm_qos_request mif_qos;
	struct pm_qos_request int_qos;
	struct pm_qos_request disp_qos;
	u32 scen_updated;
	u32 used_cnt;
};

/* cursor async */
struct decon_cursor {
	struct decon_reg_data regs;
	struct mutex lock;
	u32 xpos;
	u32 ypos;
	bool unmask;	/* if true, cursor unmask period */
};

/* systrace */
struct decon_systrace_data {
	pid_t pid;
};

struct decon_device {
	int id;
	enum decon_state state;

	unsigned long prev_used_dpp;
	unsigned long cur_using_dpp;
	unsigned long dpp_err_stat;

	struct mutex lock;
	struct mutex pm_lock;
	spinlock_t slock;

	struct ion_client *ion_client;

	struct sync_timeline *timeline;
	int timeline_max;

	struct v4l2_subdev *out_sd[MAX_DSIM_CNT];
	struct v4l2_subdev *dsim_sd[MAX_DSIM_CNT];
	struct v4l2_subdev *dpp_sd[MAX_DPP_SUBDEV];
	struct v4l2_subdev *displayport_sd;
#ifdef CONFIG_EXYNOS_COMMON_PANEL
	struct v4l2_subdev *panel_sd;
	struct panel_state *panel_state;
#endif
	struct v4l2_device v4l2_dev;
	struct v4l2_subdev sd;

	struct device *dev;
	struct decon_dt_info dt;
	struct decon_win *win[MAX_DECON_WIN];
	struct decon_resources res;
	struct decon_debug d;
	struct decon_update_regs up;
	struct decon_vsync vsync;
#if defined(CONFIG_EXYNOS_COMMON_PANEL)
	struct decon_fsync fsync;
#endif
	struct decon_lcd *lcd_info;
	struct decon_win_update win_up;
	struct decon_hiber hiber;
	struct decon_bts bts;
	struct decon_cursor cursor;

	int frame_cnt;
	int frame_cnt_target;
	wait_queue_head_t wait_vstatus;
	int eint_status;
#ifdef CONFIG_LOGGING_BIGDATA_BUG
	int eint_pend;
#endif
	int	update_regs_list_cnt;

	u32 prev_protection_bitmask;
	unsigned long prev_aclk_khz;

	atomic_t is_shutdown;
	bool up_list_saved;
#if defined(CONFIG_EXYNOS_ITMON)
	struct notifier_block itmon_nb;
	bool notified;
#endif
	unsigned long prev_hdr_bits;
	struct exynos_hdr_static_info prev_hdr_info;
	enum hwc_ver ver;
#if defined(CONFIG_EXYNOS_COMMON_PANEL)
	atomic_t bypass;
	struct decon_reg_data last_regs;
#endif
#ifdef CONFIG_DISPLAY_USE_INFO
	struct notifier_block dpui_notif;
#endif
#ifdef CONFIG_SUPPORT_DSU
	struct dsu_info dsu;
#endif
	/* systrace */
	struct decon_systrace_data systrace;
};

static inline struct decon_device *get_decon_drvdata(u32 id)
{
	return decon_drvdata[id];
}

/* register access subroutines */
static inline u32 decon_read(u32 id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(id);
	return readl(decon->res.regs + reg_id);
}

static inline u32 decon_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = decon_read(id, reg_id);
	val &= (mask);
	return val;
}

static inline void decon_write(u32 id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(id);
	writel(val, decon->res.regs + reg_id);
}

static inline void decon_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = decon_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	decon_write(id, reg_id, val);
}

static inline u32 dsc_read(u32 dsc_id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(0);
	u32 dsc_offset = dsc_id ? DSC1_OFFSET : DSC0_OFFSET;

	return readl(decon->res.regs + dsc_offset + reg_id);
}

static inline void dsc_write(u32 dsc_id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(0);
	u32 dsc_offset = dsc_id ? DSC1_OFFSET : DSC0_OFFSET;

	writel(val, decon->res.regs + dsc_offset + reg_id);
}

static inline void dsc_write_mask(u32 dsc_id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = dsc_read(dsc_id, reg_id);

	val = (val & mask) | (old & ~mask);
	dsc_write(dsc_id, reg_id, val);
}

static inline u32 sysreg_read(u32 id, u32 reg_id)
{
	struct decon_device *decon = get_decon_drvdata(id);
	return readl(decon->res.ss_regs + reg_id);
}

static inline u32 sysreg_read_mask(u32 id, u32 reg_id, u32 mask)
{
	u32 val = sysreg_read(id, reg_id);
	val &= (mask);
	return val;
}

static inline void sysreg_write(u32 id, u32 reg_id, u32 val)
{
	struct decon_device *decon = get_decon_drvdata(id);
	writel(val, decon->res.ss_regs + reg_id);
}

static inline void sysreg_write_mask(u32 id, u32 reg_id, u32 val, u32 mask)
{
	u32 old = sysreg_read(id, reg_id);

	val = (val & mask) | (old & ~mask);
	sysreg_write(id, reg_id, val);
}

/* common function API */
bool decon_validate_x_alignment(struct decon_device *decon, int x, u32 w,
		u32 bits_per_pixel);
int decon_wait_for_vsync(struct decon_device *decon, u32 timeout);
int decon_check_limitation(struct decon_device *decon, int idx,
		struct decon_win_config *config);

/* DECON to DSI interface functions */
int decon_register_irq(struct decon_device *decon);
int decon_get_clocks(struct decon_device *decon);
void decon_set_clocks(struct decon_device *decon);
int decon_get_out_sd(struct decon_device *decon);
int decon_get_pinctrl(struct decon_device *decon);
int decon_register_ext_irq(struct decon_device *decon);
int decon_create_vsync_thread(struct decon_device *decon);
void decon_destroy_vsync_thread(struct decon_device *decon);
#if defined(CONFIG_EXYNOS_COMMON_PANEL)
int decon_create_fsync_thread(struct decon_device *decon);
void decon_destroy_fsync_thread(struct decon_device *decon);
#endif
int decon_create_psr_info(struct decon_device *decon);
void decon_destroy_psr_info(struct decon_device *decon);

/* DECON to DISPLAYPORT interface functions */
int decon_displayport_register_irq(struct decon_device *decon);
void decon_displayport_free_irq(struct decon_device *decon);
int decon_displayport_create_vsync_thread(struct decon_device *decon);
int decon_displayport_get_clocks(struct decon_device *decon);
int decon_displayport_get_out_sd(struct decon_device *decon);
int decon_displayport_get_hdr_capa(struct decon_device *decon,
		struct decon_hdr_capabilities *hdr_capa);
int decon_displayport_get_hdr_capa_info(struct decon_device *decon,
		struct decon_hdr_capabilities_info *hdr_capa_info);
int decon_displayport_get_config(struct decon_device *dex,
		struct exynos_displayport_data *displayport_data);
int decon_displayport_set_config(struct decon_device *dex,
		struct exynos_displayport_data *displayport_data);

/* window update related function */
#define DPU_FULL_RECT(r, lcd)			\
	do {					\
		(r)->left = 0;			\
		(r)->top = 0;			\
		(r)->right = (lcd)->xres - 1;	\
		(r)->bottom = (lcd)->yres - 1;	\
	} while (0)
void dpu_init_win_update(struct decon_device *decon);
void dpu_prepare_win_update_config(struct decon_device *decon,
		struct decon_win_config_data *win_data,
		struct decon_reg_data *regs);
void dpu_set_win_update_config(struct decon_device *decon,
		struct decon_reg_data *regs);
void dpu_set_win_update_partial_size(struct decon_device *decon,
		struct decon_rect *up_region);
/* cursor async */
void dpu_cursor_win_update_config(struct decon_device *decon,
		struct decon_reg_data *regs);

/* internal only function API */
int decon_check_var(struct fb_var_screeninfo *var, struct fb_info *info);
int decon_set_par(struct fb_info *info);
int decon_pan_display(struct fb_var_screeninfo *var, struct fb_info *info);
int decon_setcolreg(unsigned regno,
			    unsigned red, unsigned green, unsigned blue,
			    unsigned transp, struct fb_info *info);
int decon_mmap(struct fb_info *info, struct vm_area_struct *vma);

u32 wincon(u32 transp_len, u32 a0, u32 a1, int plane_alpha,
		enum decon_blending blending, int idx);

static inline u32 win_start_pos(int x, int y)
{
	return (WIN_STRPTR_Y_F(y) | WIN_STRPTR_X_F(x));
}

static inline u32 win_end_pos(int x, int y,  u32 xres, u32 yres)
{
	return (WIN_ENDPTR_Y_F(y + yres - 1) | WIN_ENDPTR_X_F(x + xres - 1));
}


/* HIBER releated */
int decon_exit_hiber(struct decon_device *decon);
int decon_enter_hiber(struct decon_device *decon);
int decon_lcd_off(struct decon_device *decon);
int decon_register_hiber_work(struct decon_device *decon);
int decon_hiber_block_exit(struct decon_device *decon);
u32 decon_reg_get_cam_status(void __iomem *cam_status);
#ifdef CONFIG_SUPPORT_HMD
bool is_hmd_running(struct decon_device *decon);
#endif

static inline void decon_hiber_block(struct decon_device *decon)
{
	if (decon)
		atomic_inc(&decon->hiber.block_cnt);
}

static inline bool decon_is_hiber_blocked(struct decon_device *decon)
{
	return (atomic_read(&decon->hiber.block_cnt) > 0);
}

static inline int decon_get_hiber_block_cnt(struct decon_device *decon)
{
	return (atomic_read(&decon->hiber.block_cnt));
}

static inline void decon_hiber_unblock(struct decon_device *decon)
{
	if (decon) {
		if (decon_is_hiber_blocked(decon))
			atomic_dec(&decon->hiber.block_cnt);
	}
}

static inline void decon_hiber_block_reset(struct decon_device *decon)
{
	if (decon)
		atomic_set(&decon->hiber.block_cnt, 0);
}

static inline void decon_hiber_trig_reset(struct decon_device *decon)
{
	if (decon)
		atomic_set(&decon->hiber.trig_cnt, 0);
}

static inline bool decon_min_lock_cond(struct decon_device *decon)
{
	return (atomic_read(&decon->hiber.block_cnt) <= 0);
}

static inline bool is_cam_not_running(struct decon_device *decon)
{
	if (!decon->id)
		return (!(decon_reg_get_cam_status(decon->hiber.cam_status) & 0xF));
	else
		return true;
}

static inline bool decon_hiber_enter_cond(struct decon_device *decon)
{
	return ((atomic_read(&decon->hiber.block_cnt) <= 0)
		&& is_cam_not_running(decon)
		&& is_displayport_not_running()
#ifdef CONFIG_SUPPORT_HMD
		&& is_hmd_running(decon)
#endif
		&& (atomic_inc_return(&decon->hiber.trig_cnt) >
			DECON_ENTER_HIBER_CNT));
}

static inline void decon_enter_shutdown(struct decon_device *decon)
{
	atomic_inc(&decon->is_shutdown);
}

static inline bool decon_is_enter_shutdown(struct decon_device *decon)
{
	return atomic_read(&decon->is_shutdown);
}

static inline void decon_enter_shutdown_reset(struct decon_device *decon)
{
	atomic_set(&decon->is_shutdown, 0);
}

#if defined(CONFIG_EXYNOS_COMMON_PANEL)
static inline void decon_set_bypass(struct decon_device *decon, bool on)
{
	atomic_set(&decon->bypass, !!on);
}

static inline void decon_bypass_on(struct decon_device *decon)
{
	atomic_inc(&decon->bypass);
}

static inline void decon_bypass_off(struct decon_device *decon)
{
	atomic_dec(&decon->bypass);
}

/* TODO : remove decon global function */
static inline void decon_set_bypass_global(u32 id, bool on)
{
	struct decon_device *decon = get_decon_drvdata(id);

	decon_set_bypass(decon, on);
}

static inline void decon_bypass_on_global(u32 id)
{
	struct decon_device *decon = get_decon_drvdata(id);

	decon_bypass_on(decon);
}

static inline void decon_bypass_off_global(u32 id)
{
	struct decon_device *decon = get_decon_drvdata(id);

	decon_bypass_off(decon);
}

static inline bool decon_is_bypass(struct decon_device *decon)
{
	return atomic_read(&decon->bypass);
}

int decon_reset_panel(struct decon_device *decon);

static inline int decon_reset_panel_global(u32 id)
{
	struct decon_device *decon = get_decon_drvdata(id);

	return decon_reset_panel(decon);
}
#endif

#ifdef CONFIG_LOGGING_BIGDATA_BUG
void log_decon_bigdata(struct decon_device *decon);
#endif

enum disp_pwr_mode {
	DISP_PWR_OFF = 0,
#ifdef CONFIG_SUPPORT_DOZE
	DISP_PWR_DOZE,
#endif
	DISP_PWR_NORMAL,
#ifdef CONFIG_SUPPORT_DOZE
	DISP_PWR_DOZE_SUSPEND,
#endif
	DISP_PWR_MAX,
};

typedef int (*set_pwr_state_t)(void *);

struct disp_pwr_state {
	u32 state;
	set_pwr_state_t set_pwr_state;
};

static inline bool IS_DECON_ON_STATE(struct decon_device *decon)
{
	return decon->state == DECON_STATE_INIT ||
		decon->state == DECON_STATE_ON ||
#ifdef CONFIG_SUPPORT_DOZE
		decon->state == DECON_STATE_DOZE ||
#endif
		decon->state == DECON_STATE_TUI;
}

static inline bool IS_DECON_OFF_STATE(struct decon_device *decon)
{
	return decon->state == DECON_STATE_HIBER ||
#ifdef CONFIG_SUPPORT_DOZE
		decon->state == DECON_STATE_DOZE_SUSPEND ||
#endif
		decon->state == DECON_STATE_OFF;
}

#ifdef CONFIG_SUPPORT_DOZE
static inline bool IS_DECON_DOZE_STATE(struct decon_device *decon)
{
	return decon->state == DECON_STATE_DOZE ||
		decon->state == DECON_STATE_DOZE_SUSPEND;
}
#else
static inline bool IS_DECON_DOZE_STATE(struct decon_device *decon)
{
	return false;
}
#endif

static inline bool IS_DECON_HIBER_STATE(struct decon_device *decon)
{
	return decon->state == DECON_STATE_HIBER;
}

/* CAL APIs list */
void dpu_reg_set_qactive_pll(u32 id, u32 en);
int decon_reg_init(u32 id, u32 dsi_idx, struct decon_param *p);
//void decon_reg_init_probe(u32 id, u32 dsi_idx, struct decon_param *p);
int decon_reg_start(u32 id, struct decon_mode_info *psr);
int decon_reg_stop_perframe(u32 id, u32 dsi_idx, struct decon_mode_info *psr);
int decon_reg_stop_inst(u32 id, u32 dsi_idx, struct decon_mode_info *psr);
int decon_reg_reset(u32 id);
void decon_reg_direct_on_off(u32 id, u32 en);
int decon_reg_stop(u32 id, u32 dsi_idx, struct decon_mode_info *psr);
int decon_reg_stop_tui(u32 id, u32 dsi_idx, struct decon_mode_info *psr);
void decon_reg_release_resource(u32 id, struct decon_mode_info *psr);
void decon_reg_set_int(u32 id, struct decon_mode_info *psr, u32 en);
void decon_reg_set_window_control(u32 id, int win_idx,
		struct decon_window_regs *regs, u32 winmap_en);
void decon_reg_update_req_and_unmask(u32 id, struct decon_mode_info *psr);
int decon_reg_wait_update_done_and_mask(u32 id, struct decon_mode_info *psr,
		u32 timeout);
void decon_reg_set_trigger(u32 id, struct decon_mode_info *psr,
		enum decon_set_trig en);
int decon_reg_wait_for_update_timeout(u32 id, unsigned long timeout);
int decon_reg_get_interrupt_and_clear(u32 id, u32 *ext_irq);
#if defined(CONFIG_SOC_EYXNOS8895)
void decon_reg_config_data_path_size(u32 id,
	u32 width, u32 height, u32 overlap_w);
void decon_reg_set_blender_bg_image_size(u32 id, enum decon_dsi_mode dsi_mode,
		struct decon_lcd *lcd_info);
#elif defined(CONFIG_SOC_EXYNOS9810)
void decon_reg_config_data_path_size(u32 id, u32 width, u32 height,
		u32 overlap_w, struct decon_dsc *p, struct decon_param *param);
u32 dsc_get_dual_slice_mode(struct decon_lcd *lcd_info);
u32 dsc_get_slice_mode_change(struct decon_lcd *lcd_info);
#endif
void decon_reg_set_dispif_size(u32 id, u32 width, u32 height);
void decon_reg_get_clock_ratio(struct decon_clocks *clks,
		struct decon_lcd *lcd_info);
void decon_reg_clear_int_all(u32 id);
void decon_reg_all_win_shadow_update_req(u32 id);
void decon_reg_update_req_window(u32 id, u32 win_idx);
void decon_reg_update_req_window_mask(u32 id, u32 win_idx);
void decon_reg_set_partial_update(u32 id, enum decon_dsi_mode dsi_mode,
		struct decon_lcd *lcd_info, bool in_slice[],
		u32 partial_w, u32 partial_h);
int decon_reg_wait_idle_status_timeout(u32 id, unsigned long timeout);
void decon_reg_set_start_crc(u32 id, u32 en);
void decon_reg_set_select_crc_bits(u32 id, u32 bit_sel);
void decon_reg_get_crc_data(u32 id, u32 *w0_data, u32 *w1_data);
void dpu_sysreg_set_lpmux(void __iomem *sysreg);
void decon_reg_set_win_enable(u32 id, u32 win_idx, u32 en);

/* tui feature support external to security driver(gud) */
int decon_tui_protection(bool tui_en);
int decon_reg_stop_nreset(u32 id, struct decon_mode_info *psr);
void decon_reg_update_req_global(u32 id);

/* helper functions */
int dpu_get_sd_by_drvname(struct decon_device *decon, char *drvname);
u32 dpu_translate_fmt_to_dpp(u32 format);
u32 dpu_get_bpp(enum decon_pixel_format fmt);
int dpu_get_meta_plane_cnt(enum decon_pixel_format format);
int dpu_get_plane_cnt(enum decon_pixel_format format, bool is_hdr);
u32 dpu_get_alpha_len(int format);

void decon_dump(struct decon_device *decon, int dsi_dump);
void decon_to_psr_info(struct decon_device *decon, struct decon_mode_info *psr);
void decon_to_init_param(struct decon_device *decon, struct decon_param *p);
void decon_create_timeline(struct decon_device *decon, char *name);
#if defined(CONFIG_DPU_2_0_RELEASE_FENCES)
void decon_create_release_fences(struct decon_device *decon,
		struct decon_win_config_data *win_data,
		struct sync_file *sync_file);
#endif
int decon_create_fence(struct decon_device *decon, struct sync_file **sync_file);
void decon_wait_fence(struct sync_file *fence);
void decon_signal_fence(struct decon_device *decon);

bool decon_intersect(struct decon_rect *r1, struct decon_rect *r2);
int decon_intersection(struct decon_rect *r1,
		struct decon_rect *r2, struct decon_rect *r3);
void dpu_dump_data_to_console(void *v_addr, int buf_size, int id);

bool is_decon_rect_differ(struct decon_rect *r1, struct decon_rect *r2);
bool is_rgb32(int format);
bool is_scaling(struct decon_win_config *config);
bool is_full(struct decon_rect *r, struct decon_lcd *lcd);
bool is_decon_opaque_format(int format);
void __iomem *dpu_get_sysreg_addr(void);
u32 dpu_dma_type_to_channel(enum decon_idma_type type);
void dpu_dump_afbc_info(void);
#if defined(CONFIG_EXYNOS_CONTENT_PATH_PROTECTION)
void decon_set_protected_content(struct decon_device *decon,
		struct decon_reg_data *regs);
#endif

int decon_runtime_suspend(struct device *dev);
int decon_runtime_resume(struct device *dev);
void decon_dpp_stop(struct decon_device *decon, bool do_reset);

int decon_set_out_sd_state(struct decon_device *decon,
		enum decon_state state);
int decon_update_last_regs(struct decon_device *decon,
		struct decon_reg_data *regs);

/* cursor async mode functions */
void decon_set_cursor_reset(struct decon_device *decon,
		struct decon_reg_data *regs);
void decon_set_cursor_unmask(struct decon_device *decon, bool unmask);
void dpu_cursor_win_update_config(struct decon_device *decon,
		struct decon_reg_data *regs);
int decon_set_cursor_win_config(struct decon_device *decon, int x, int y);
void decon_reg_set_te_qactive_pll_mode(u32 id, u32 en);

int dpu_sysmmu_fault_handler(struct iommu_domain *domain,
	struct device *dev, unsigned long iova, int flags, void *token);

#ifdef CONFIG_SUPPORT_DSU
int set_dsu_win_config(struct decon_device *decon,
	struct decon_win_config *windata, struct decon_reg_data *regs);
void init_dsu_info(struct decon_device *decon);
int set_dsu_config(struct decon_device *decon, struct decon_reg_data *regs);
void decon_reg_set_dsu(u32 id, enum decon_dsi_mode dsi_mode, struct decon_param *p);
#endif
/* IOCTL commands */
#define S3CFB_SET_VSYNC_INT		_IOW('F', 206, __u32)
#define S3CFB_DECON_SELF_REFRESH	_IOW('F', 207, __u32)
#define S3CFB_WIN_CONFIG_OLD		_IOW('F', 209, \
						struct decon_win_config_data_old)
#define S3CFB_WIN_CONFIG		_IOW('F', 209, \
						struct decon_win_config_data)
#define EXYNOS_DISP_INFO		_IOW('F', 260, \
						struct decon_disp_info)

#define S3CFB_START_CRC			_IOW('F', 270, u32)
#define S3CFB_SEL_CRC_BITS		_IOW('F', 271, u32)
#define S3CFB_GET_CRC_DATA		_IOR('F', 272, u32)

#define EXYNOS_GET_DISPLAYPORT_CONFIG		_IOW('F', 300, \
						struct exynos_displayport_data)
#define EXYNOS_SET_DISPLAYPORT_CONFIG		_IOW('F', 301, \
						struct exynos_displayport_data)

#define EXYNOS_DPU_DUMP		_IOW('F', 302, \
						struct decon_win_config_data)

#define S3CFB_POWER_MODE		_IOW('F', 223, __u32)

/* HDR support */
#define S3CFB_GET_HDR_CAPABILITIES _IOW('F', 400, struct decon_hdr_capabilities)
#define S3CFB_GET_HDR_CAPABILITIES_NUM _IOW('F', 401, struct decon_hdr_capabilities_info)

/* cursor async */
#define DECON_WIN_CURSOR_POS		_IOW('F', 222, struct decon_user_window)

/* DPU aclk */
#define EXYNOS_DPU_GET_ACLK		_IOR('F', 500, u32)

#if defined(CONFIG_EXYNOS_COMMON_PANEL)
#define V4L2_EVENT_DECON                (V4L2_EVENT_PRIVATE_START + 1000)
#define V4L2_EVENT_DECON_FRAME_START    (V4L2_EVENT_DECON + 1)
#define V4L2_EVENT_DECON_FRAME_DONE     (V4L2_EVENT_DECON + 2)
#define V4L2_EVENT_DECON_VSYNC          (V4L2_EVENT_DECON + 3)
#endif

#endif /* ___SAMSUNG_DECON_H__ */
