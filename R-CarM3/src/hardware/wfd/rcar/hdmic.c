/*
 * $QNXLicenseC:
 * Copyright 2014-2015, QNX Software Systems. 
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */
#include <stddef.h>
#include <unistd.h>
#include "hdmic.h"

enum hdmi_datamap {
	RGB444_8B = 0x01,
	RGB444_10B = 0x03,
	RGB444_12B = 0x05,
	RGB444_16B = 0x07,
	YCbCr444_8B = 0x09,
	YCbCr444_10B = 0x0B,
	YCbCr444_12B = 0x0D,
	YCbCr444_16B = 0x0F,
	YCbCr422_8B = 0x16,
	YCbCr422_10B = 0x14,
	YCbCr422_12B = 0x12,
};

static const unsigned short csc_coeff_default[3][4] = {
	{ 0x2000, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x2000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x2000, 0x0000 }
};

static const unsigned short csc_coeff_rgb_out_eitu601[3][4] = {
	{ 0x2000, 0x6926, 0x74fd, 0x010e },
	{ 0x2000, 0x2cdd, 0x0000, 0x7e9a },
	{ 0x2000, 0x0000, 0x38b4, 0x7e3b }
};

static const unsigned short csc_coeff_rgb_out_eitu709[3][4] = {
	{ 0x2000, 0x7106, 0x7a02, 0x00a7 },
	{ 0x2000, 0x3264, 0x0000, 0x7e6d },
	{ 0x2000, 0x0000, 0x3b61, 0x7e25 }
};

static const unsigned short csc_coeff_rgb_in_eitu601[3][4] = {
	{ 0x2591, 0x1322, 0x074b, 0x0000 },
	{ 0x6535, 0x2000, 0x7acc, 0x0200 },
	{ 0x6acd, 0x7534, 0x2000, 0x0200 }
};

static const unsigned short csc_coeff_rgb_in_eitu709[3][4] = {
	{ 0x2dc5, 0x0d9b, 0x049e, 0x0000 },
	{ 0x62f0, 0x2000, 0x7d11, 0x0200 },
	{ 0x6756, 0x78ab, 0x2000, 0x0200 }
};

static const struct dw_hdmi_mpll_config rcar_du_hdmienc_mpll_cfg[] = {
	{
		44900000, {
			{ 0x0003, 0x0000},
			{ 0x0003, 0x0000},
			{ 0x0003, 0x0000}
		},
	}, {
		90000000, {
			{ 0x0002, 0x0000},
			{ 0x0002, 0x0000},
			{ 0x0002, 0x0000}
		},
	}, {
		182750000, {
			{ 0x0001, 0x0000},
			{ 0x0001, 0x0000},
			{ 0x0001, 0x0000}
		},
	}, {
		297000000, {
			{ 0x0000, 0x0000},
			{ 0x0000, 0x0000},
			{ 0x0000, 0x0000}
		},
	}, {
		~0U, {
			{ 0xFFFF, 0xFFFF },
			{ 0xFFFF, 0xFFFF },
			{ 0xFFFF, 0xFFFF },
		},
	}
};
static const struct dw_hdmi_curr_ctrl rcar_du_hdmienc_cur_ctr[] = {
	/*      pixelclk    bpp8    bpp10   bpp12 */
	{
		35500000,  { 0x0344, 0x0000, 0x0000 },
	}, {
		44900000,  { 0x0285, 0x0000, 0x0000 },
	}, {
		71000000,  { 0x1184, 0x0000, 0x0000 },
	}, {
		90000000,  { 0x1144, 0x0000, 0x0000 },
	}, {
		140250000, { 0x20c4, 0x0000, 0x0000 },
	}, {
		182750000, { 0x2084, 0x0000, 0x0000 },
	}, {
		297000000, { 0x0084, 0x0000, 0x0000 },
	}, {
		~0U,      { 0x0000, 0x0000, 0x0000 },
	}
};

static const struct dw_hdmi_multi_div rcar_du_hdmienc_multi_div[] = {
	/*      pixelclk    bpp8    bpp10   bpp12 */
	{
		35500000,  { 0x0328, 0x0000, 0x0000 },
	}, {
		44900000,  { 0x0128, 0x0000, 0x0000 },
	}, {
		71000000,  { 0x0314, 0x0000, 0x0000 },
	}, {
		90000000,  { 0x0114, 0x0000, 0x0000 },
	}, {
		140250000, { 0x030a, 0x0000, 0x0000 },
	}, {
		182750000, { 0x010a, 0x0000, 0x0000 },
	}, {
		281250000, { 0x0305, 0x0000, 0x0000 },
	}, {
		297000000, { 0x0105, 0x0000, 0x0000 },
	}, {
		~0U,      { 0x0000, 0x0000, 0x0000 },
	}
};

static const struct dw_hdmi_phy_config rcar_du_hdmienc_phy_config[] = {
	/*pixelclk   symbol   term   vlev*/
	{ 74250000,  0x8009, 0x0004, 0x0272},
	{ 148500000, 0x802b, 0x0004, 0x028d},
	{ 297000000, 0x8039, 0x0005, 0x028d},
	{ ~0U,	     0x0000, 0x0000, 0x0000}
};

static inline void hdmi_writeb(struct dw_hdmi *hdmi, unsigned char val, int offset)
{
	out8(hdmi->regs + offset, val);
}

static inline unsigned char hdmi_readb(struct dw_hdmi *hdmi, int offset)
{
	return in8(hdmi->regs + offset);
}

static void hdmi_modb(struct dw_hdmi *hdmi, unsigned char data, unsigned char mask, unsigned reg)
{
	unsigned char val = hdmi_readb(hdmi, reg) & ~mask;

	val |= data & mask;
	hdmi_writeb(hdmi, val, reg);
}

static void hdmi_mask_writeb(struct dw_hdmi *hdmi, unsigned char data, unsigned int reg,
			     unsigned char shift, unsigned char mask)
{
	hdmi_modb(hdmi, data << shift, mask, reg);
}

static void hdmi_set_cts_n(struct dw_hdmi *hdmi, unsigned int cts,
			   unsigned int n)
{
	/* Must be set/cleared first */
	//hdmi_modb(hdmi, 0, HDMI_AUD_CTS3_CTS_MANUAL, HDMI_AUD_CTS3);	

	/* nshift factor = 0 */
	//hdmi_modb(hdmi, 0, HDMI_AUD_CTS3_N_SHIFT_MASK, HDMI_AUD_CTS3);

	hdmi_writeb(hdmi, 0x00, HDMI_AUD_N3);
	
	/* Use auto CTS */
	hdmi_writeb(hdmi, (cts >> 16) & 0x0f, HDMI_AUD_CTS3);
	hdmi_writeb(hdmi, (cts >> 8) & 0xff, HDMI_AUD_CTS2);
	hdmi_writeb(hdmi, cts & 0xff, HDMI_AUD_CTS1);

	hdmi_writeb(hdmi, (n >> 16) & 0x0f, HDMI_AUD_N3);
	hdmi_writeb(hdmi, (n >> 8) & 0xff, HDMI_AUD_N2);
	hdmi_writeb(hdmi, n & 0xff, HDMI_AUD_N1);
}

static unsigned int hdmi_compute_n(unsigned int freq, uint32_t pixel_clk,
				   unsigned int ratio)
{
	unsigned int n = (128 * freq) / 1000;

	switch (freq) {
	case 32000:
		if (pixel_clk == 25170000)
			n = (ratio == 150) ? 9152 : 4576;
		else if (pixel_clk == 27020000)
			n = (ratio == 150) ? 8192 : 4096;
		else if (pixel_clk == 74170000 || pixel_clk == 148350000)
			n = 11648;
		else
			n = 4096;
		break;

	case 44100:
		if (pixel_clk == 25170000)
			n = 7007;
		else if (pixel_clk == 74170000)
			n = 17836;
		else if (pixel_clk == 148350000)
			n = (ratio == 150) ? 17836 : 8918;
		else
			n = 6272;
		break;

	case 48000:
		if (pixel_clk == 25170000)
			n = (ratio == 150) ? 9152 : 6864;
		else if (pixel_clk == 27020000)
			n = (ratio == 150) ? 8192 : 6144;
		else if (pixel_clk == 74170000)
			n = 11648;
		else if (pixel_clk == 148350000)
			n = (ratio == 150) ? 11648 : 5824;
		else
			n = 6144;
		break;

	case 88200:
		n = hdmi_compute_n(44100, pixel_clk, ratio) * 2;
		break;

	case 96000:
		n = hdmi_compute_n(48000, pixel_clk, ratio) * 2;
		break;

	case 176400:
		n = hdmi_compute_n(44100, pixel_clk, ratio) * 4;
		break;

	case 192000:
		n = hdmi_compute_n(48000, pixel_clk, ratio) * 4;
		break;

	default:
		break;
	}

	return n;
}

static unsigned int hdmi_compute_cts(unsigned int freq, uint32_t pixel_clk,
				     unsigned int ratio)
{
	unsigned int cts = 0;

	switch (freq) {
	case 32000:
		if (pixel_clk == 297000000) {
			cts = 222750;
			break;
		}
	case 48000:
	case 96000:
	case 192000:
		switch (pixel_clk) {
		case 25200000:
		case 27000000:
		case 54000000:
		case 74250000:
		case 148500000:
			cts = pixel_clk / 1000;
			break;
		case 297000000:
			cts = 247500;
			break;
		/*
		 * All other TMDS clocks are not supported by
		 * DWC_hdmi_tx. The TMDS clocks divided or
		 * multiplied by 1,001 coefficients are not
		 * supported.
		 */
		default:
			break;
		}
		break;
	case 44100:
	case 88200:
	case 176400:
		switch (pixel_clk) {
		case 25200000:
			cts = 28000;
			break;
		case 27000000:
			cts = 30000;
			break;
		case 54000000:
			cts = 60000;
			break;
		case 74250000:
			cts = 82500;
			break;
		case 148500000:
			cts = 165000;
			break;
		case 297000000:
			cts = 247500;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if (ratio == 100)
		return cts;
	return (cts * ratio) / 100;
}

static void hdmi_set_clk_regenerator(struct dw_hdmi *hdmi,
	uint32_t pixel_clk, unsigned int sample_rate, unsigned int ratio)
{
	unsigned int n, cts;

	n = hdmi_compute_n(sample_rate, pixel_clk, ratio);
	cts = hdmi_compute_cts(sample_rate, pixel_clk, ratio);
	if (!cts) {
		SLOG_WARNING("%s: pixel clock/sample rate not supported: %u / %d",
			__func__, pixel_clk, sample_rate);
	}

	SLOG_DEBUG("%s: samplerate=%d ratio=%d pixelclk=%u N=%d cts=%d",
		__func__, sample_rate, ratio, pixel_clk, n, cts);

	hdmi->audio_n = n;
	hdmi->audio_cts = cts;
	hdmi_set_cts_n(hdmi, cts, hdmi->audio_enable ? n : 0);
}

static void hdmi_clk_regenerator_update_pixel_clock(struct dw_hdmi *hdmi)
{
	hdmi_set_clk_regenerator(hdmi, hdmi->hdmi_data.video_mode.mpixelclock,
				 hdmi->sample_rate, hdmi->ratio);
}

void dw_hdmi_set_sample_rate(struct dw_hdmi *hdmi, unsigned int rate)
{
	hdmi->sample_rate = rate;
	hdmi_set_clk_regenerator(hdmi, hdmi->hdmi_data.video_mode.mpixelclock,
				 hdmi->sample_rate, hdmi->ratio);
}

void dw_hdmi_audio_enable(struct dw_hdmi *hdmi)
{
	hdmi->audio_enable = 1;
	hdmi_set_cts_n(hdmi, hdmi->audio_cts, hdmi->audio_n);
}
void dw_hdmi_audio_disable(struct dw_hdmi *hdmi)
{
	hdmi->audio_enable = 0;
	hdmi_set_cts_n(hdmi, hdmi->audio_cts, 0);
}

/*
 * this submodule is responsible for the video data synchronization.
 * for example, for RGB 4:4:4 input, the data map is defined as
 *			pin{47~40} <==> R[7:0]
 *			pin{31~24} <==> G[7:0]
 *			pin{15~8}  <==> B[7:0]
 */
static void hdmi_video_sample(struct dw_hdmi *hdmi)
{
	int color_format = 0;
	unsigned char val;

	if (hdmi->hdmi_data.enc_in_format == RGB) {
		if (hdmi->hdmi_data.enc_color_depth == 8)
			color_format = 0x01;
		else if (hdmi->hdmi_data.enc_color_depth == 10)
			color_format = 0x03;
		else if (hdmi->hdmi_data.enc_color_depth == 12)
			color_format = 0x05;
		else if (hdmi->hdmi_data.enc_color_depth == 16)
			color_format = 0x07;
		else
			return;
	} else if (hdmi->hdmi_data.enc_in_format == YCBCR444) {
		if (hdmi->hdmi_data.enc_color_depth == 8)
			color_format = 0x09;
		else if (hdmi->hdmi_data.enc_color_depth == 10)
			color_format = 0x0B;
		else if (hdmi->hdmi_data.enc_color_depth == 12)
			color_format = 0x0D;
		else if (hdmi->hdmi_data.enc_color_depth == 16)
			color_format = 0x0F;
		else
			return;
	} else if (hdmi->hdmi_data.enc_in_format == YCBCR422_8BITS) {
		if (hdmi->hdmi_data.enc_color_depth == 8)
			color_format = 0x16;
		else if (hdmi->hdmi_data.enc_color_depth == 10)
			color_format = 0x14;
		else if (hdmi->hdmi_data.enc_color_depth == 12)
			color_format = 0x12;
		else
			return;
	}

	val = HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE |
		((color_format << HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET) &
		HDMI_TX_INVID0_VIDEO_MAPPING_MASK);
	hdmi_writeb(hdmi, val, HDMI_TX_INVID0);

	/* Enable TX stuffing: When DE is inactive, fix the output data to 0 */
	val = HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE |
		HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE |
		HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE;
	hdmi_writeb(hdmi, val, HDMI_TX_INSTUFFING);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_GYDATA0);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_GYDATA1);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_RCRDATA0);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_RCRDATA1);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_BCBDATA0);
	delay(10);
	hdmi_writeb(hdmi, 0x0, HDMI_TX_BCBDATA1);
	delay(10);
}

static int is_color_space_conversion(struct dw_hdmi *hdmi)
{
	return hdmi->hdmi_data.enc_in_format != hdmi->hdmi_data.enc_out_format;
}

static int is_color_space_decimation(struct dw_hdmi *hdmi)
{
	if (hdmi->hdmi_data.enc_out_format != YCBCR422_8BITS)
		return 0;
	if (hdmi->hdmi_data.enc_in_format == RGB ||
	    hdmi->hdmi_data.enc_in_format == YCBCR444)
		return 1;
	return 0;
}

static int is_color_space_interpolation(struct dw_hdmi *hdmi)
{
	if (hdmi->hdmi_data.enc_in_format != YCBCR422_8BITS)
		return 0;
	if (hdmi->hdmi_data.enc_out_format == RGB ||
	    hdmi->hdmi_data.enc_out_format == YCBCR444)
		return 1;
	return 0;
}

static void dw_hdmi_update_csc_coeffs(struct dw_hdmi *hdmi)
{
	const unsigned short (*csc_coeff)[3][4] = &csc_coeff_default;
	unsigned i;
	uint32_t csc_scale = 1;

	if (is_color_space_conversion(hdmi)) {
		if (hdmi->hdmi_data.enc_out_format == RGB) {
			if (hdmi->hdmi_data.colorimetry ==
					HDMI_COLORIMETRY_ITU_601)
				csc_coeff = &csc_coeff_rgb_out_eitu601;
			else
				csc_coeff = &csc_coeff_rgb_out_eitu709;
		} else if (hdmi->hdmi_data.enc_in_format == RGB) {
			if (hdmi->hdmi_data.colorimetry ==
					HDMI_COLORIMETRY_ITU_601)
				csc_coeff = &csc_coeff_rgb_in_eitu601;
			else
				csc_coeff = &csc_coeff_rgb_in_eitu709;
			csc_scale = 0;
		}
	}

	/* The CSC registers are sequential, alternating MSB then LSB */
	for (i = 0; i < sizeof(csc_coeff_default[0])/sizeof(csc_coeff_default[0][0]); i++) {
		unsigned short coeff_a = (*csc_coeff)[0][i];
		unsigned short coeff_b = (*csc_coeff)[1][i];
		unsigned short coeff_c = (*csc_coeff)[2][i];

		hdmi_writeb(hdmi, coeff_a & 0xff, HDMI_CSC_COEF_A1_LSB + i * 2);
		delay(10);
		hdmi_writeb(hdmi, coeff_a >> 8, HDMI_CSC_COEF_A1_MSB + i * 2);
		delay(10);
		hdmi_writeb(hdmi, coeff_b & 0xff, HDMI_CSC_COEF_B1_LSB + i * 2);
		delay(10);
		hdmi_writeb(hdmi, coeff_b >> 8, HDMI_CSC_COEF_B1_MSB + i * 2);
		delay(10);
		hdmi_writeb(hdmi, coeff_c & 0xff, HDMI_CSC_COEF_C1_LSB + i * 2);
		delay(10);
		hdmi_writeb(hdmi, coeff_c >> 8, HDMI_CSC_COEF_C1_MSB + i * 2);
		delay(10);
	}

	hdmi_modb(hdmi, csc_scale, HDMI_CSC_SCALE_CSCSCALE_MASK,
		  HDMI_CSC_SCALE);
}

static void hdmi_video_csc(struct dw_hdmi *hdmi)
{
	int color_depth = 0;
	int interpolation = HDMI_CSC_CFG_INTMODE_DISABLE;
	int decimation = 0;

	/* YCC422 interpolation to 444 mode */
	if (is_color_space_interpolation(hdmi))
		interpolation = HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1;
	else if (is_color_space_decimation(hdmi))
		decimation = HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3;

	if (hdmi->hdmi_data.enc_color_depth == 8)
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP;
	else if (hdmi->hdmi_data.enc_color_depth == 10)
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP;
	else if (hdmi->hdmi_data.enc_color_depth == 12)
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP;
	else if (hdmi->hdmi_data.enc_color_depth == 16)
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP;
	else
		return;

	/* Configure the CSC registers */
	hdmi_writeb(hdmi, interpolation | decimation, HDMI_CSC_CFG);
	delay(10);
	hdmi_modb(hdmi, color_depth, HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK,
		  HDMI_CSC_SCALE);

	dw_hdmi_update_csc_coeffs(hdmi);
}

/*
 * HDMI video packetizer is used to packetize the data.
 * for example, if input is YCC422 mode or repeater is used,
 * data should be repacked this module can be bypassed.
 */
static void hdmi_video_packetize(struct dw_hdmi *hdmi)
{
	unsigned int color_depth = 0;
	unsigned int remap_size = HDMI_VP_REMAP_YCC422_16bit;
	unsigned int output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_PP;
	struct hdmi_data_info *hdmi_data = &hdmi->hdmi_data;
	unsigned char val, vp_conf;

	if (hdmi_data->enc_out_format == RGB ||
	    hdmi_data->enc_out_format == YCBCR444) {
		if (!hdmi_data->enc_color_depth) {
			output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
		} else if (hdmi_data->enc_color_depth == 8) {
			color_depth = 4;
			output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
		} else if (hdmi_data->enc_color_depth == 10) {
			color_depth = 5;
		} else if (hdmi_data->enc_color_depth == 12) {
			color_depth = 6;
		} else if (hdmi_data->enc_color_depth == 16) {
			color_depth = 7;
		} else {
			return;
		}
	} else if (hdmi_data->enc_out_format == YCBCR422_8BITS) {
		if (!hdmi_data->enc_color_depth ||
		    hdmi_data->enc_color_depth == 8)
			remap_size = HDMI_VP_REMAP_YCC422_16bit;
		else if (hdmi_data->enc_color_depth == 10)
			remap_size = HDMI_VP_REMAP_YCC422_20bit;
		else if (hdmi_data->enc_color_depth == 12)
			remap_size = HDMI_VP_REMAP_YCC422_24bit;
		else
			return;
		output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_YCC422;
	} else {
		return;
	}

	/* set the packetizer registers */
	val = ((color_depth << HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET) &
		HDMI_VP_PR_CD_COLOR_DEPTH_MASK) |
		((hdmi_data->pix_repet_factor <<
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET) &
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK);
	hdmi_writeb(hdmi, val, HDMI_VP_PR_CD);

	hdmi_modb(hdmi, HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE,
		  HDMI_VP_STUFF_PR_STUFFING_MASK, HDMI_VP_STUFF);

	/* Data from pixel repeater block */
	if (hdmi_data->pix_repet_factor > 1) {
		vp_conf = HDMI_VP_CONF_PR_EN_ENABLE |
			  HDMI_VP_CONF_BYPASS_SELECT_PIX_REPEATER;
	} else { /* data from packetizer block */
		vp_conf = HDMI_VP_CONF_PR_EN_DISABLE |
			  HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER;
	}

	hdmi_modb(hdmi, vp_conf,
		  HDMI_VP_CONF_PR_EN_MASK |
		  HDMI_VP_CONF_BYPASS_SELECT_MASK, HDMI_VP_CONF);
	delay(10);

	hdmi_modb(hdmi, 1 << HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET,
		  HDMI_VP_STUFF_IDEFAULT_PHASE_MASK, HDMI_VP_STUFF);
	delay(10);

	hdmi_writeb(hdmi, remap_size, HDMI_VP_REMAP);

	if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_PP) {
		vp_conf = HDMI_VP_CONF_BYPASS_EN_DISABLE |
			  HDMI_VP_CONF_PP_EN_ENABLE |
			  HDMI_VP_CONF_YCC422_EN_DISABLE;
	} else if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_YCC422) {
		vp_conf = HDMI_VP_CONF_BYPASS_EN_DISABLE |
			  HDMI_VP_CONF_PP_EN_DISABLE |
			  HDMI_VP_CONF_YCC422_EN_ENABLE;
	} else if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS) {
		vp_conf = HDMI_VP_CONF_BYPASS_EN_ENABLE |
			  HDMI_VP_CONF_PP_EN_DISABLE |
			  HDMI_VP_CONF_YCC422_EN_DISABLE;
	} else {
		return;
	}

	hdmi_modb(hdmi, vp_conf,
		  HDMI_VP_CONF_BYPASS_EN_MASK | HDMI_VP_CONF_PP_EN_ENMASK |
		  HDMI_VP_CONF_YCC422_EN_MASK, HDMI_VP_CONF);
	delay(10);

	hdmi_modb(hdmi, HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE |
			HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE,
		  HDMI_VP_STUFF_PP_STUFFING_MASK |
		  HDMI_VP_STUFF_YCC422_STUFFING_MASK, HDMI_VP_STUFF);
	delay(10);

	hdmi_modb(hdmi, output_select, HDMI_VP_CONF_OUTPUT_SELECTOR_MASK,
		  HDMI_VP_CONF);
}

static inline void hdmi_phy_test_clear(struct dw_hdmi *hdmi,
				       unsigned char bit)
{
	hdmi_modb(hdmi, bit << HDMI_PHY_TST0_TSTCLR_OFFSET,
		  HDMI_PHY_TST0_TSTCLR_MASK, HDMI_PHY_TST0);
}

static int hdmi_phy_wait_i2c_done(struct dw_hdmi *hdmi, int msec)
{
	unsigned int val;

	while ((val = hdmi_readb(hdmi, HDMI_IH_I2CMPHY_STAT0) & 0x3) == 0) {
		if (msec-- == 0)
		{
			SLOG_ERROR("hdmi_phy_wait_i2c_done FAIL");
			return 0;
		}
		delay(1);
	}
	hdmi_writeb(hdmi, val, HDMI_IH_I2CMPHY_STAT0);

	return 1;
}

static void __hdmi_phy_i2c_write(struct dw_hdmi *hdmi, unsigned short data,
				 unsigned char addr)
{
	hdmi_writeb(hdmi, 0xFF, HDMI_IH_I2CMPHY_STAT0);
	hdmi_writeb(hdmi, addr, HDMI_PHY_I2CM_ADDRESS_ADDR);
	hdmi_writeb(hdmi, (unsigned char)(data >> 8),
		    HDMI_PHY_I2CM_DATAO_1_ADDR);
	hdmi_writeb(hdmi, (unsigned char)(data >> 0),
		    HDMI_PHY_I2CM_DATAO_0_ADDR);
	hdmi_writeb(hdmi, HDMI_PHY_I2CM_OPERATION_ADDR_WRITE,
		    HDMI_PHY_I2CM_OPERATION_ADDR);
	hdmi_phy_wait_i2c_done(hdmi, 1000);
}

static int hdmi_phy_i2c_write(struct dw_hdmi *hdmi, unsigned short data,
			      unsigned char addr)
{
	__hdmi_phy_i2c_write(hdmi, data, addr);
	return 0;
}

static void dw_hdmi_phy_enable_powerdown(struct dw_hdmi *hdmi, int enable)
{
	hdmi_mask_writeb(hdmi, !enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_PDZ_OFFSET,
			 HDMI_PHY_CONF0_PDZ_MASK);
}

static void dw_hdmi_phy_enable_tmds(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_ENTMDS_OFFSET,
			 HDMI_PHY_CONF0_ENTMDS_MASK);
}

static void dw_hdmi_phy_enable_spare(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_SPARECTRL_OFFSET,
			 HDMI_PHY_CONF0_SPARECTRL_MASK);
}

static void dw_hdmi_phy_gen2_pddq(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET,
			 HDMI_PHY_CONF0_GEN2_PDDQ_MASK);
}

static void dw_hdmi_phy_gen2_txpwron(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET,
			 HDMI_PHY_CONF0_GEN2_TXPWRON_MASK);
}

static void dw_hdmi_phy_sel_data_en_pol(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_SELDATAENPOL_OFFSET,
			 HDMI_PHY_CONF0_SELDATAENPOL_MASK);
}

static void dw_hdmi_phy_sel_interface_control(struct dw_hdmi *hdmi, unsigned char enable)
{
	hdmi_mask_writeb(hdmi, enable, HDMI_PHY_CONF0,
			 HDMI_PHY_CONF0_SELDIPIF_OFFSET,
			 HDMI_PHY_CONF0_SELDIPIF_MASK);
}

static int hdmi_phy_configure(struct dw_hdmi *hdmi, unsigned char res, int cscon)
{
	unsigned res_idx;
	unsigned char val, msec;
	const struct dw_hdmi_mpll_config *mpll_config = rcar_du_hdmienc_mpll_cfg;
	const struct dw_hdmi_curr_ctrl *curr_ctrl = rcar_du_hdmienc_cur_ctr;
	const struct dw_hdmi_phy_config *phy_config = rcar_du_hdmienc_phy_config;
	const struct dw_hdmi_multi_div *multi_div = rcar_du_hdmienc_multi_div;

	switch (res) {
	case 0:	/* color resolution 0 is 8 bit colour depth */
	case 8:
		res_idx = DW_HDMI_RES_8;
		break;
	case 10:
		res_idx = DW_HDMI_RES_10;
		break;
	case 12:
		res_idx = DW_HDMI_RES_12;
		break;
	default:
		return -1;
	}

	/* PLL/MPLL Cfg - always match on final entry */
	for (; mpll_config->mpixelclock != ~0UL; mpll_config++)
		if (hdmi->hdmi_data.video_mode.mpixelclock <=
		    mpll_config->mpixelclock)
			break;

	for (; curr_ctrl->mpixelclock != ~0UL; curr_ctrl++)
		if (hdmi->hdmi_data.video_mode.mpixelclock <=
		    curr_ctrl->mpixelclock)
			break;

	for (; phy_config->mpixelclock != ~0UL; phy_config++)
		if (hdmi->hdmi_data.video_mode.mpixelclock <=
		    phy_config->mpixelclock)
			break;

	for (; multi_div->mpixelclock != (~0UL); multi_div++)
		if (hdmi->hdmi_data.video_mode.mpixelclock <=
			multi_div->mpixelclock)
			break;

	if (mpll_config->mpixelclock == ~0UL ||
	    curr_ctrl->mpixelclock == ~0UL ||
	    phy_config->mpixelclock == ~0UL) {
		SLOG_ERROR("Pixel clock %d - unsupported by HDMI",
			hdmi->hdmi_data.video_mode.mpixelclock);
		return -1;
	}

	if (multi_div->mpixelclock == ~0UL) {
		SLOG_ERROR("Pixel clock %d - unsupported by HDMI",
			hdmi->hdmi_data.video_mode.mpixelclock);
		return -1;
	}

	/* Enable csc path */
	if (cscon)
		val = HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH;
	else
		val = HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS;
	hdmi_writeb(hdmi, val, HDMI_MC_FLOWCTRL);
	
	/* gen2 tx power off */
	dw_hdmi_phy_gen2_txpwron(hdmi, 0);

	/* gen2 pddq */
	dw_hdmi_phy_gen2_pddq(hdmi, 1);

	/* PHY reset */
	hdmi_writeb(hdmi, HDMI_MC_PHYRSTZ_DEASSERT, HDMI_MC_PHYRSTZ);
	delay(10);
	hdmi_writeb(hdmi, HDMI_MC_PHYRSTZ_ASSERT, HDMI_MC_PHYRSTZ);
	delay(10);

	hdmi_writeb(hdmi, HDMI_MC_HEACPHY_RST_ASSERT, HDMI_MC_HEACPHY_RST);

	hdmi_phy_test_clear(hdmi, 1);
	hdmi_writeb(hdmi, HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2,
		    HDMI_PHY_I2CM_SLAVE_ADDR);
	hdmi_phy_test_clear(hdmi, 0);

	hdmi_phy_i2c_write(hdmi, mpll_config->res[res_idx].cpce, 0x06);
	hdmi_phy_i2c_write(hdmi, curr_ctrl->curr[res_idx], 0x10);
	hdmi_phy_i2c_write(hdmi, multi_div->multi[res_idx], 0x11);

	dw_hdmi_phy_enable_powerdown(hdmi, 0);

	/* toggle TMDS enable */
	dw_hdmi_phy_enable_tmds(hdmi, 0);
	dw_hdmi_phy_enable_tmds(hdmi, 1);

	/* gen2 tx power on */
	dw_hdmi_phy_gen2_txpwron(hdmi, 1);
	dw_hdmi_phy_gen2_pddq(hdmi, 0);

	dw_hdmi_phy_enable_spare(hdmi, 1);

	/*Wait for PHY PLL lock */
	msec = 5;
	do {
		val = hdmi_readb(hdmi, HDMI_PHY_STAT0) & HDMI_PHY_TX_PHY_LOCK;
		if (!val)
			break;

		if (msec == 0) {
			SLOG_ERROR( "PHY PLL not locked");
			return -1;
		}

		delay(1);
		msec--;
	} while (1);

	return 0;
}

static int dw_hdmi_phy_init(struct dw_hdmi *hdmi)
{
	int i, ret;

	for (i = 0; i < 2; i++) {
		dw_hdmi_phy_sel_data_en_pol(hdmi, 1);
		dw_hdmi_phy_sel_interface_control(hdmi, 0);
		dw_hdmi_phy_enable_tmds(hdmi, 0);
		dw_hdmi_phy_enable_powerdown(hdmi, 1);

		/* Configure phy without color space conversion */
		ret = hdmi_phy_configure(hdmi, 8, 0);
		if (ret)
			return ret;
	}

	hdmi->phy_enabled = 1;
	return 0;
}

static void hdmi_tx_hdcp_config(struct dw_hdmi *hdmi)
{
	unsigned char de;

	if (hdmi->hdmi_data.video_mode.mdataenablepolarity)
		de = HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_HIGH;
	else
		de = HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_LOW;

	/* disable rx detect */
	hdmi_modb(hdmi, HDMI_A_HDCPCFG0_RXDETECT_DISABLE,
		  HDMI_A_HDCPCFG0_RXDETECT_MASK, HDMI_A_HDCPCFG0);

	hdmi_modb(hdmi, de, HDMI_A_VIDPOLCFG_DATAENPOL_MASK, HDMI_A_VIDPOLCFG);

	hdmi_modb(hdmi, HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_DISABLE,
		  HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_MASK, HDMI_A_HDCPCFG1);
}

static void hdmi_av_composer(struct dw_hdmi *hdmi)
{
	port_t *port = (port_t *)hdmi->port;	
    const struct wfdcfg_timing * timing = port->active_mode->timings;
	
	unsigned char inv_val;
	struct hdmi_vmode *vmode = &hdmi->hdmi_data.video_mode;
	int hblank, vblank, h_de_hs, v_de_vs, hsync_len, vsync_len;

	vmode->mpixelclock = port->pixel_clk;

	/* Set up HDMI_FC_INVIDCONF */
	inv_val = HDMI_FC_INVIDCONF_HDCP_KEEPOUT_ACTIVE |
				HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH |
				HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH |
				HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH |
				HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW |
				HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE |
				HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE;
	hdmi_writeb(hdmi, inv_val, HDMI_FC_INVIDCONF);
	delay(10);
	
	/* Set up horizontal active pixel width */
	hdmi_writeb(hdmi, timing->hpixels >> 8, HDMI_FC_INHACTV1);
	delay(10);
	hdmi_writeb(hdmi, timing->hpixels, HDMI_FC_INHACTV0);
	delay(10);
	/* Set up vertical active lines */
	hdmi_writeb(hdmi, timing->vlines >> 8, HDMI_FC_INVACTV1);
	delay(10);
	hdmi_writeb(hdmi, timing->vlines, HDMI_FC_INVACTV0);
	delay(10);
	/* Set up horizontal blanking pixel region width */
	hblank = timing->hsw + timing->hfp + timing->hbp;
	hdmi_writeb(hdmi, hblank >> 8, HDMI_FC_INHBLANK1);
	delay(10);
	hdmi_writeb(hdmi, hblank, HDMI_FC_INHBLANK0);
	delay(10);
	
	/* Set up vertical blanking pixel region width */
	vblank = timing->vsw + timing->vfp + timing->vbp;
	hdmi_writeb(hdmi, vblank, HDMI_FC_INVBLANK);
	delay(10);
	/* Set up HSYNC active edge delay width (in pixel clks) */
	h_de_hs = timing->hfp; /*hsync_start - hdisplay */
	hdmi_writeb(hdmi, h_de_hs >> 8, HDMI_FC_HSYNCINDELAY1);
	delay(10);
	hdmi_writeb(hdmi, h_de_hs, HDMI_FC_HSYNCINDELAY0);
	delay(10);
	
	/* Set up VSYNC active edge delay (in lines) */
	v_de_vs = timing->vfp; /* vsync_start - vdisplay */
	hdmi_writeb(hdmi, v_de_vs, HDMI_FC_VSYNCINDELAY);
	delay(10);
	
	/* Set up HSYNC active pulse width (in pixel clks) */
	hsync_len = timing->hsw; /* hsync_end - hsync_start */
	hdmi_writeb(hdmi, hsync_len >> 8, HDMI_FC_HSYNCINWIDTH1);
	delay(10);
	hdmi_writeb(hdmi, hsync_len, HDMI_FC_HSYNCINWIDTH0);
	delay(10);
	
	/* Set up VSYNC active edge delay (in lines) */
	vsync_len = timing->vsw; /* vsync_end - vsync_start */
	hdmi_writeb(hdmi, vsync_len, HDMI_FC_VSYNCINWIDTH);
}

static void dw_hdmi_phy_disable(struct dw_hdmi *hdmi)
{
	if (!hdmi->phy_enabled)
		return;

	dw_hdmi_phy_enable_tmds(hdmi, 0);
	dw_hdmi_phy_enable_powerdown(hdmi, 1);

	hdmi->phy_enabled = 0;
}

/* HDMI Initialization Step B.4 */
static void dw_hdmi_enable_video_path(struct dw_hdmi *hdmi)
{
	unsigned char clkdis;

	/* control period minimum duration */
	hdmi_writeb(hdmi, 12, HDMI_FC_CTRLDUR);
	delay(10);
	hdmi_writeb(hdmi, 32, HDMI_FC_EXCTRLDUR);
	delay(10);
	hdmi_writeb(hdmi, 1, HDMI_FC_EXCTRLSPAC);
	delay(10);

	/* Set to fill TMDS data channels */
	hdmi_writeb(hdmi, 0x0B, HDMI_FC_CH0PREAM);
	delay(10);
	hdmi_writeb(hdmi, 0x16, HDMI_FC_CH1PREAM);
	delay(10);
	hdmi_writeb(hdmi, 0x21, HDMI_FC_CH2PREAM);
	delay(10);

	/* Enable pixel clock and tmds data path */
	clkdis = 0x7F;
	clkdis &= ~HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
	hdmi_writeb(hdmi, clkdis, HDMI_MC_CLKDIS);

	clkdis &= ~HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
	hdmi_writeb(hdmi, clkdis, HDMI_MC_CLKDIS);

	/* Enable csc path */
	if (is_color_space_conversion(hdmi)) {
		clkdis &= ~HDMI_MC_CLKDIS_CSCCLK_DISABLE;
		hdmi_writeb(hdmi, clkdis, HDMI_MC_CLKDIS);
	}
}

static void hdmi_enable_audio_clk(struct dw_hdmi *hdmi)
{
	hdmi_modb(hdmi, 0, HDMI_MC_CLKDIS_AUDCLK_DISABLE, HDMI_MC_CLKDIS);
}

/* Workaround to clear the overflow condition */
static void dw_hdmi_clear_overflow(struct dw_hdmi *hdmi)
{
	int count;
	unsigned char val;
	
	/* TMDS software reset */
	hdmi_writeb(hdmi, (unsigned char)~HDMI_MC_SWRSTZ_TMDSSWRST_REQ, HDMI_MC_SWRSTZ);

	val = hdmi_readb(hdmi, HDMI_FC_INVIDCONF);

	for (count = 0; count < 4; count++)
		hdmi_writeb(hdmi, val, HDMI_FC_INVIDCONF);
}

static int dw_hdmi_setup(struct dw_hdmi *hdmi)
{
	int ret;

	hdmi->audio_enable = 1;
	hdmi->sample_rate = 48000;
	hdmi->ratio = 100;
	
	hdmi->hdmi_data.enc_in_format = RGB;
	hdmi->hdmi_data.enc_out_format = RGB;

	hdmi->hdmi_data.enc_color_depth = 8;
	hdmi->hdmi_data.pix_repet_factor = 0;
	hdmi->hdmi_data.hdcp_enable = 0;
	hdmi->hdmi_data.video_mode.mdataenablepolarity = 1;

	/* HDMI Initialization Step B.1 */
	hdmi_av_composer(hdmi);

	/* HDMI Initialization Step B.2 */
	ret = dw_hdmi_phy_init(hdmi);
	if (ret)
		return ret;

	/* HDMI Initialization Step B.3 */
	dw_hdmi_enable_video_path(hdmi);

	if (hdmi->audio_enable)
	{
		hdmi_writeb(hdmi, 0x2F, HDMI_AUD_CONF0);			// Enable I2S0,1,2,3
		delay(10);
		hdmi_writeb(hdmi, 0x30, HDMI_AUD_CONF1);			// Right-justified, 16 bit data
		delay(10);
		hdmi_writeb(hdmi, 0x00, HDMI_AUD_CONF2);			// L-PCM audio data
		delay(10);
		hdmi_writeb(hdmi, 0x04, HDMI_AUD_INPUTCLKFS);		// 64Fs
	
		/* HDMI Initialization Step E - Configure audio */
		hdmi_clk_regenerator_update_pixel_clock(hdmi);
		hdmi_enable_audio_clk(hdmi);
	}
	
	hdmi_video_packetize(hdmi);
	hdmi_video_csc(hdmi);
	hdmi_video_sample(hdmi);
	hdmi_tx_hdcp_config(hdmi);
	dw_hdmi_clear_overflow(hdmi);
	hdmi->phy_enabled = 1;

	return 0;
}

int hdmi_init(void *arg)
{
    struct dw_hdmi *hdmi;
    port_t *port = (port_t *)arg;

    port->enc = malloc (sizeof(struct dw_hdmi));
    if (!port->enc)
        return -1;
    
    hdmi = (struct dw_hdmi*)port->enc;
    hdmi->port = (void *)port;
	
    switch (port->du_cfg->du_index)
    {
        case DU_CH_1:
            hdmi->hdmi_phy_base = HDMI0_REG_BASE;
            break;
        case DU_CH_2:
            hdmi->hdmi_phy_base = HDMI1_REG_BASE;
            break;
        default:
            SLOG_ERROR("Can't init HDMI output for DU%d",port->du_cfg->du_index);
            return -1;
            break;
    }
    hdmi->regs = mmap_device_io (HDMI_REG_SIZE, hdmi->hdmi_phy_base);
    if (hdmi->regs==-1)
    {
        SLOG_ERROR("HDMI: Unable to map device io at hdmi_phy_base: %x",hdmi->hdmi_phy_base);
        return -1;
    }
    
	dw_hdmi_setup (hdmi);
    return 0;
}

int hdmi_fini(void *arg) {
	struct dw_hdmi *hdmi;
	port_t *port = (port_t *)arg;
	hdmi = (struct dw_hdmi*)port->enc;
	
	dw_hdmi_phy_disable (hdmi);
	munmap_device_io(hdmi->regs, HDMI_REG_SIZE);
	free (port->enc);
	
	return 0;
}
