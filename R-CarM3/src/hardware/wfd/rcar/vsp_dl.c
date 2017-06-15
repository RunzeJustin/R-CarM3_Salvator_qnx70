/*
 * $QNXLicenseC:
 * Copyright 2014-2016, QNX Software Systems.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "vsp.h"

enum {
	DL_MEM_NO_USE,
	DL_MEM_USE,
};

#define	DL_FLAG_NONE			0x00000000
#define	DL_FLAG_INTERRUPT		0x00000001
#define	DL_FLAG_AUTO_REPEAT		0x00000002
#define	DL_FLAG_MANUAL_REPEAT	0x00000004
#define	DL_FLAG_HEADER_LESS		0x00000008
#define	DL_FLAG_BODY_WRITEBLE	0x00000010

#define	DL_FLAG_REPEAT_MASK	(DL_FLAG_MANUAL_REPEAT | DL_FLAG_AUTO_REPEAT)

void vsp_dl_set_to_core(struct vsp_private_data *vdata, vsp_dev_t* dev, uintptr_t reg, uint32_t data, struct dl_body *body)
{
	uint32_t reg_off = get_reg_off(reg);
	body->dlist[body->reg_count].set_address = reg_off;
	body->dlist[body->reg_count].set_data = data;
	body->reg_count++;
}

void vsp_rpf_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, rpf_par_t *in, int rpf_id, struct dl_body *body)
{
	/* input image size (width/height) */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].src_bsize, in->src_bsize, body);
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].src_esize, in->src_esize, body);
	/* input image format */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].infmt, in->infmt, body);
	/* input data swap */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].dswap, in->dswap, body);
	/* input image position (master layer = 0.0) */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].loc, in->loc, body);
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].alph_sel, in->alph_sel, body);
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].vrtcol_set, in->vrtcol_set, body);
	/* input image stride */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].srcm_pstride, in->srcm_pstride, body);
	/* input image address */
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].srcm_addr_y, in->srcm_addr_y, body);
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].srcm_addr_c0, in->srcm_addr_c0, body);
    vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.rpf[rpf_id].srcm_addr_c1, in->srcm_addr_c1, body);
	return;
}

void vsp_uds_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, uds_par_t *in,	struct dl_body *body)
{
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].ctrl, in->ctrl, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].scale, in->scale, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].alpth, in->alpth, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].alpval, in->alpval, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].pass_bwidt, in->pass_bwidt, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].hphase, in->hphase, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].hszclip, in->hszclip, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].clip_size, in->clip_size, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.uds[0].fill_color, in->fill_color, body);
	return;
}

void vsp_wpf_to_dl_core(struct vsp_private_data *vdata, vsp_dev_t* dev, wpf_par_t *out, struct dl_body *body)
{
	int rpf_id = 0;
	rpf_par_t rpf_par;

	for (rpf_id = 0; rpf_id < VSPD_INPUT_IMAGE_NUM; rpf_id++)
	{
		rpf_par = dev->param.rpf_par[rpf_id];

		if (!rpf_par.active) /* Check if any pipeline is deactive */
		{
			/* Deactivate RPF source */
			out->src_rpf &= ~(SRC_RPF_MASK << rpf_id*2);
		}
	}

	/* select input rpf */
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].src_rpf, out->src_rpf, body);
	/* output image format is RGBA8888 */

	/* crop horizontal input image */
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].hszclip, out->hszclip, body);

	/* crop vertical input image */
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].vszclip, out->vszclip, body);
    
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].outfmt, 0x00800000|out->outfmt, body);

	/* output data swap */
  	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dswap, out->dswap, body);
	/* output image stride */
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_stride_y, out->dstm_stride_y, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_stride_c, out->dstm_stride_c, body);

	/* output image address */
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_y, out->dstm_addr_y, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_c0, out->dstm_addr_c0, body);
	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].dstm_addr_c1, out->dstm_addr_c1, body);


	vsp_dl_set_to_core(vdata, dev, (uintptr_t)&dev->reg.wpf[0].rndctrl, 0, body);

	return;
}


static void vsp_dl_free_multi_body(struct dl_body *body)
{
	struct dl_body *free_body = body;
	struct dl_body *tmp_body;

	if (body == NULL)
		return;

	free_body = body;

	do {
		free_body->use = DL_MEM_NO_USE;
		tmp_body = free_body->next;
		free_body->next = NULL;
		free_body = tmp_body;
	} while ((free_body != NULL)  && (free_body != body));
}


/* This function is called when the frame end interrupt occurs */
void vsp_dl_irq_frame_end(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	/* no operation */
}

/* This function is called when the display start interrupt occurs. */
int vsp_dl_irq_display_start(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	struct dl_memory *dlmemory = vdata->dlmemory;
	struct dl_body *next_body = NULL;
	struct dl_body *free_body = NULL;

	if (!(dlmemory->flag & DL_FLAG_HEADER_LESS))
		return 1;

	InterruptLock( &dlmemory->lock );

	/* DU progressive mode */
	dlmemory->flag |= DL_FLAG_BODY_WRITEBLE;
	if (dlmemory->pending_body) {
		/* update next frame for pending. */
		free_body = dlmemory->next_body;
		dlmemory->next_body = dlmemory->pending_body;
		next_body = dlmemory->next_body;
		dlmemory->pending_body = NULL;
		dlmemory->active_body_next_set = next_body;
	}

	InterruptUnlock( &dlmemory->lock );

	if (next_body) {
		dev->reg.dl->hdr_addr0 = next_body->paddr;
		dev->reg.dl->body_size0 = (next_body->reg_count * 8) | VI6_DL_BODY_SIZE_UPD0;
	}

	vsp_dl_free_multi_body(free_body);

	return 0;
}

static void vsp_dl_irq_dl_frame_end_header_mode(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	/* no operation */
}

static void vsp_dl_irq_dl_frame_end_header_less(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	struct dl_memory *dlmemory = vdata->dlmemory;
	struct dl_body *free_body = NULL;

	InterruptLock( &dlmemory->lock );

	dlmemory->flag &= ~DL_FLAG_BODY_WRITEBLE;
	if (dlmemory->flag & DL_FLAG_REPEAT_MASK) {

		if ((dlmemory->next_body != NULL)) {
			/* free old Display List */
			free_body = dlmemory->active_body;
			dlmemory->active_body = dlmemory->next_body;
			dlmemory->next_body = NULL;
		}

		if (dlmemory->active_body_next_set) {
			dlmemory->active_body_now = dlmemory->active_body_next_set;
			dlmemory->active_body_next_set = NULL;
		}
	} else {
		free_body = dlmemory->active_body;
		dlmemory->active_body = NULL;
	}

	InterruptUnlock( &dlmemory->lock );

	vsp_dl_free_multi_body(free_body);

}

/* This function is called when the Display List frame end interrupt occurs */
void vsp_dl_irq_dl_frame_end(struct vsp_private_data *vdata, vsp_dev_t *dev)
{
	/* update next Display List & free old Display List & */
	/* VSP Start Command                                  */

	if (vdata->dlmemory->flag & DL_FLAG_HEADER_LESS)
		/* header less auto repeat mode */
		vsp_dl_irq_dl_frame_end_header_less(vdata, dev);
	else
		/* normal auto repeat mode */
		vsp_dl_irq_dl_frame_end_header_mode(vdata, dev);
}


static int vsp_dl_config(struct dl_memory *dlmemory)
{
	int i, k;
	int ret = EXIT_SUCCESS;

	if(dlmemory->dl_mode == DL_MODE_AUTO_REPEAT)
    {
		uint32_t offset = 0;
		/* specify a multiple of 16 bytes for Display List header address*/
		offset = (((dlmemory->paddr + 15) / 16)) * 16 - dlmemory->paddr;
		/* header config */
		struct dl_head *head[DISPLAY_LIST_NUM];
        for (i = 0; i < DISPLAY_LIST_NUM; i++) 
        {
            head[i] = &dlmemory->head[i];
            head[i]->size = DL_HEADER_SIZE;
            head[i]->paddr = dlmemory->paddr + offset;
            head[i]->dheader = dlmemory->vaddr + offset;
            head[i]->dheader_offset = offset;

            if (i < DISPLAY_LIST_NUM-1 )
            {
                head[i]->next = (struct dl_head *)head[i]->dheader + DL_BODY_SIZE*DL_BODY_NUM_FOR_WORK + DL_HEADER_SIZE;
            }
            else
            {
                head[i]->next = NULL;
            }

            offset += DL_HEADER_SIZE;

            memset(head[i]->dheader, 0, DL_HEADER_SIZE);
             

            /* body config */
            for (k = 0; k < DL_BODY_NUM_FOR_WORK; k++) 
            {
                struct dl_body *body = &(dlmemory->body[i + k]);

                body->reg_count = 0;
                body->paddr = dlmemory->paddr + offset;
                body->dlist = dlmemory->vaddr + offset;
                body->dlist_offset = offset;
                
                head[i]->dl_body_list[k] = body;
                /* specify a multiple of 8 bytes for Display List body address*/
                offset += DL_BODY_SIZE;
            }
            offset += DL_BODY_SIZE*DL_BODY_NUM_FOR_WORK;
        }
	} else if(dlmemory->dl_mode == DL_MODE_HEADER_LESS_AUTO_REPEAT)
    {
		/* header less body config */
		for (i = 0; i < DISPLAY_LIST_NUM; i++) 
        {
			struct dl_body *single_body = &dlmemory->single_body[i];

			single_body->reg_count = 0;
			single_body->use = DL_MEM_NO_USE;
			single_body->paddr = dlmemory->paddr + DL_BODY_SIZE * i;
			single_body->dlist = dlmemory->vaddr + DL_BODY_SIZE * i;
			single_body->dlist_offset = DL_BODY_SIZE * i;
			single_body->next = NULL;
			single_body->flag = 0;
		}
	} else {
		ret = EXIT_FAILURE;
	}
	
	return ret;
}

int vsp_dl_create(struct vsp_private_data *vdata, port_t *port, int dl_mode)
{
	struct dl_memory *_dlmemory;
	int ret;
	unsigned int dl_mem_size = 0;
	int pt_fd = -1;

	_dlmemory = malloc(sizeof(*_dlmemory));
	if (!_dlmemory) {
		ret = EXIT_FAILURE;
		goto error1;
	}

	if(dl_mode == DL_MODE_AUTO_REPEAT){
		/* normal auto mode */
		dl_mem_size = (DL_HEADER_SIZE + (DL_BODY_SIZE * DL_BODY_NUM_FOR_WORK))*DISPLAY_LIST_NUM;
	}else if(dl_mode == DL_MODE_HEADER_LESS_AUTO_REPEAT){
		/* header less auto mode */
		dl_mem_size = DL_BODY_SIZE * DISPLAY_LIST_NUM;
	}else{
		ret = EXIT_FAILURE;
		goto error1;
	}

	pt_fd = posix_typed_mem_open(RCARDU_POSIX_TYPED_MEM_PATH, O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG);
    if (pt_fd == -1) {
    	SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't open posix typed memory)");
        ret = EXIT_FAILURE; 
        goto error1;
    }

	/* allocate virtual memory for display list */ 
    _dlmemory->vaddr = mmap64(0, dl_mem_size, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, pt_fd, 0);
    if (_dlmemory->vaddr == MAP_FAILED) {
    	SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't mmap posix typed memory)");
        ret = EXIT_FAILURE; 
        close(pt_fd);
        goto error1;
    }
    close(pt_fd);

    if (mem_offset64(_dlmemory->vaddr, NOFD, dl_mem_size, (off64_t *)&_dlmemory->paddr, 0) == -1){
    	SLOG_ERROR("return: WFD_ERROR_OUT_OF_MEMORY (Can't get physical offset of posix typed memory)");
        ret = EXIT_FAILURE; 
        goto error2;
    }

	_dlmemory->size = dl_mem_size;
	_dlmemory->flag = 0;
	_dlmemory->active_body = NULL;
	_dlmemory->next_body = NULL;
	_dlmemory->pending_body = NULL;
	_dlmemory->active_body_index = 0;
	_dlmemory->start = 0;
	_dlmemory->dl_mode = dl_mode;
	memset( &_dlmemory->lock, 0, sizeof( intrspin_t ) );
	vsp_dl_config(_dlmemory);
	vdata->dlmemory = _dlmemory;
	return EXIT_SUCCESS;

error2:
	munmap(_dlmemory->vaddr, dl_mem_size);

	free(_dlmemory);
error1:
	return ret;
}

void vsp_dl_destroy(struct vsp_private_data *vdata)
{
	munmap(vdata->dlmemory->vaddr, vdata->dlmemory->size);
	free(vdata->dlmemory);
}

struct dl_body *vsp_dl_get_body(struct vsp_private_data *vdata)
{
	struct dl_body *body = NULL;
	struct dl_memory *dlmemory = vdata->dlmemory;
 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return NULL;
	}

	InterruptLock( &dlmemory->lock );

	if(vdata->active == 0 || dlmemory->active_body_index == DL_BODY_NUM_FOR_WORK - 1){
		dlmemory->active_body_index = 0;
	}else{
		dlmemory->active_body_index++;
	}

	body = &dlmemory->body[dlmemory->active_body_index];
	InterruptUnlock(&dlmemory->lock);

	body->reg_count = 0;
	body->next = NULL;

	return body;
}

struct dl_body *vsp_dl_get_single_body(struct dl_memory *dlmemory)
{
	struct dl_body *body = NULL;
	int i;

 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return NULL;
	}

	InterruptLock( &dlmemory->lock );
	for (i = 0; i < DISPLAY_LIST_NUM; i++) {
		if (dlmemory->single_body[i].use == DL_MEM_NO_USE) {
			body = &dlmemory->single_body[i];
			body->use = DL_MEM_USE;
			break;
		}
	}
	InterruptUnlock(&dlmemory->lock);

	if (body == NULL)
		return NULL;

	body->reg_count = 0;
	body->next = NULL;

	return body;
}

static int vsp_dl_header_setup(struct dl_head *head)
{
	struct display_header *dheader = head->dheader;
	memset(head->dheader, 0, sizeof(*head->dheader));
	struct dl_body *dl_body_list = head->dl_body_list[0];

	if (dl_body_list != NULL)
	{
		dheader->display_list[0].num_bytes = (dl_body_list->reg_count * 8) << 0;
		dheader->display_list[0].plist = dl_body_list->paddr;
	}

	/* display list body num */
	dheader->num_list_minus1 = DL_LINKED_BODY_NUM - 1;
	dheader->pnext_header = head->paddr;

	dheader->int_auto = 1 << 0;

	return EXIT_SUCCESS;
}

static int vsp_dl_start_header_mode(struct vsp_private_data *vdata, vsp_dev_t *dev, struct dl_head *head)
{
 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return EXIT_FAILURE;
	}

	InterruptLock( &vdata->dlmemory->lock );
	vsp_dl_header_setup(&vdata->dlmemory->head[0]);
	vdata->dlmemory->start = 1;
	InterruptUnlock(&vdata->dlmemory->lock);

	dev->reg.dl->ctrl = VI6_DL_CTRL_AR_WAIT | VI6_DL_CTRL_DL_ENABLE;
	/* DL LWORD swap */
	dev->reg.dl->swap = VI6_DL_SWAP_LWS;
	dev->reg.dl->hdr_addr0 = head->paddr;

	dev->reg.vi6_ctrl->wpf_irq[0].irq_enb = 0;
	dev->reg.vi6_ctrl->dsp_irq.dsp_irq_enb = 0;

	dsb();

	start_wpf(dev, 0);

	return EXIT_SUCCESS;
}

static int vsp_dl_start_header_less(struct vsp_private_data *vdata, vsp_dev_t *dev, struct dl_body *body)
{
	struct dl_memory *dlmemory = vdata->dlmemory;

	body->next = body;

 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return EXIT_FAILURE;
	}

	InterruptLock( &dlmemory->lock );

	dlmemory->active_body = body;
	dlmemory->active_body_now = body;

	InterruptUnlock( &dlmemory->lock );

	/* DL_CTRL */
	dev->reg.dl->ctrl = VI6_DL_CTRL_AR_WAIT
			| VI6_DL_CTRL_DL_ENABLE
			| VI6_DL_CTRL_CFM0
			| VI6_DL_CTRL_NH0;
	/* DL_SWAP LWORD */
	dev->reg.dl->swap = VI6_DL_SWAP_LWS;
	/* DL_HDR_ADDR0 */
	dev->reg.dl->hdr_addr0 = body->paddr;
	/* DL_BODY_SIZE */
	dev->reg.dl->body_size0 = (body->reg_count * 8) | VI6_DL_BODY_SIZE_UPD0;

	/* Enable WPF0 interrupt */
	dev->reg.vi6_ctrl->wpf_irq[0].irq_enb = VI6_WPFn_IRQ_DFE;
	dev->reg.vi6_ctrl->dsp_irq.dsp_irq_enb = VI6_DISP_IRQ_STA_DST;

	dsb();

	start_wpf(dev, 0);

	return EXIT_SUCCESS;
}

int vsp_dl_start(struct vsp_private_data *vdata, vsp_dev_t *dev, void *dl, int dl_mode)
{
	struct dl_memory *dlmemory = vdata->dlmemory;
	int result = EXIT_SUCCESS;

	if ((dlmemory->start != 0) ||
	    (dlmemory->active_body != NULL)) {
		return EXIT_FAILURE;
	}

	switch (dl_mode) {
	case DL_MODE_HEADER_LESS_AUTO_REPEAT:
		dlmemory->flag = DL_FLAG_HEADER_LESS | DL_FLAG_AUTO_REPEAT;
		result = vsp_dl_start_header_less(vdata, dev, dl);
		break;
	case DL_MODE_AUTO_REPEAT:
		dlmemory->flag = DL_FLAG_AUTO_REPEAT;
		result = vsp_dl_start_header_mode(vdata, dev, dl);
		break;
	default:
		return EXIT_FAILURE;
	}

	return result;
}

static int vsp_dl_swap_header_mode(struct vsp_private_data *vdata, struct dl_head *head)
{
	struct display_header *dheader = head->dheader;
	struct dl_body *dl_body_list = head->dl_body_list[0];

	if (dl_body_list != NULL)
	{
	 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
		    return EXIT_FAILURE;
		}
		InterruptLock( &vdata->dlmemory->lock );
		dheader->display_list[0].plist = dl_body_list->paddr;
		InterruptUnlock( &vdata->dlmemory->lock );
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int vsp_dl_swap_header_less(struct vsp_private_data *vdata, vsp_dev_t *dev, struct dl_body *body)
{
	struct dl_memory *dlmemory = vdata->dlmemory;
	uint32_t stat;
	int write_enable = 1;

	body->next = body;

 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return EXIT_FAILURE;
	}

	InterruptLock( &dlmemory->lock );

	if (dlmemory->flag & DL_FLAG_BODY_WRITEBLE) {
		stat = dev->reg.dl->body_size0;
		if (stat & VI6_DL_BODY_SIZE_UPD0) {
			write_enable = 0;
		}
		if (!(stat & VI6_DL_BODY_SIZE_UPD0) &&
		     (dlmemory->next_body != NULL)) {
			write_enable = 0;
		}
	} else {
		write_enable = 0;
	}
	if (!write_enable) {
		vsp_dl_free_multi_body(dlmemory->pending_body);
		dlmemory->pending_body = body;
		InterruptUnlock( &dlmemory->lock );
		return EXIT_SUCCESS;
	}

	dev->reg.dl->hdr_addr0 = body->paddr;
	dev->reg.dl->body_size0 = (body->reg_count * 8) | VI6_DL_BODY_SIZE_UPD0;

	vsp_dl_free_multi_body(dlmemory->next_body);
	dlmemory->next_body = body;
	dlmemory->active_body_next_set = body;

	InterruptUnlock( &dlmemory->lock );

	return EXIT_SUCCESS;
}

int vsp_dl_swap(struct vsp_private_data *vdata, vsp_dev_t *dev, void *dl)
{
	dsb();

	struct dl_memory *dlmemory = vdata->dlmemory;

	if (dlmemory->flag & DL_FLAG_HEADER_LESS) {
		if (dlmemory->active_body == NULL) {
			return EXIT_FAILURE;
		}
		return vsp_dl_swap_header_less(vdata, dev, dl);
	} else {
		if (dlmemory->start == 0) {
			return EXIT_FAILURE;
		}
		return vsp_dl_swap_header_mode(vdata, dl);
	}
}

void vsp_dl_reset(struct vsp_private_data *vdata)
{
	struct dl_memory *dlmemory = vdata->dlmemory;
	int i;

 	if (-1 == ThreadCtl(_NTO_TCTL_IO, 0)) {
	    return;
	}
 
	InterruptLock( &dlmemory->lock );

	dlmemory->flag = 0;
	dlmemory->active_body = NULL;
	dlmemory->next_body = NULL;
	dlmemory->pending_body = NULL;
	dlmemory->active_body_now = NULL;
	dlmemory->active_body_next_set = NULL;
	dlmemory->active_body_index = 0;
	dlmemory->start = 0;

	if(dlmemory->dl_mode == DL_MODE_AUTO_REPEAT){
		for (i = 0; i < DL_BODY_NUM_FOR_WORK; i++) {
			dlmemory->body[i].reg_count = 0;
		}
	} else {
		for (i = 0; i < DISPLAY_LIST_NUM; i++) {
			vsp_dl_free_multi_body(&dlmemory->single_body[i]);
		}
	}
 
	InterruptUnlock( &dlmemory->lock );

}
