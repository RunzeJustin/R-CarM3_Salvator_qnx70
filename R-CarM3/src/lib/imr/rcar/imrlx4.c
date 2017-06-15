/*
* $QNXLicenseC:
* Copyright 2014, QNX Software Systems.
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

#include "imrlx4.h"
#include <arm/r-car.h>

rcar_imr_t* imr;

paddr_t rcar_imrlx4_mphys(void *addr)
{
	off64_t offset;

	if(mem_offset64(addr, NOFD, 1, &offset, 0) == -1) {
		return -1;
	}
	return offset;
}

void rcar_imrlx4_setup()
{
	uint32_t reg;

	/* Enable clock */
	rcar_imrlx4_enable_clock();

	/* Basic configure */
	rcar_imrlx4_configure();

	/* Display list initial */
	rcar_imrlx4_DL_init();

	/* Wait for interrupt */
	if(rcar_imrlx4_create_thread()) {
		fprintf(stderr, "%s: create interrupt handler failed \r\n", __FUNCTION__);
	}

	//Enable interrupts
	reg = in32(imr->vbase + RCAR_IMRLX4_ICR);
	reg |= (3 << 0);
	out32(imr->vbase + RCAR_IMRLX4_ICR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_IMR);
	reg &= ~(3 << 0);
	out32(imr->vbase + RCAR_IMRLX4_IMR, reg);
}

void rcar_imrlx4_enable_clock()
{
	//Enable clock for IMRLX4 module
	uintptr_t SMSTPCR8_reg;
	uintptr_t MSTPSR8_reg;

	uint32_t tmp, mask;
	SMSTPCR8_reg   = mmap_device_io(4, 0xE6150990);
	MSTPSR8_reg	   = mmap_device_io(4, 0xE61509A0);

	mask = 1 << 23; //IMR0
	/* Enale supply clock to module */
	tmp = in32(MSTPSR8_reg);
	tmp &= ~mask;
	out32(SMSTPCR8_reg, tmp);

	/* Unmap register */
	munmap_device_io(SMSTPCR8_reg, 4);
	munmap_device_io(MSTPSR8_reg, 4);
}

void rcar_imrlx4_configure()
{
	uint32_t reg;

	//Source and Destination coordinate Decimal Point
	reg = in32(imr->vbase + RCAR_IMRLX4_UVDPOR);
	reg &= ~(0x07);
	reg &= ~(1 << 8);
	out32(imr->vbase + RCAR_IMRLX4_UVDPOR, reg);

	//Source Width and Source Height
	reg = in32(imr->vbase + RCAR_IMRLX4_SUSR);
	reg &= ~(0x7FF | (0x7FF << 16));
	reg |= ((imr->img.dw - 2) << 16) | ((imr->img.dw - 1) << 0);
	out32(imr->vbase + RCAR_IMRLX4_SUSR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_SVSR);
	reg &= ~(0x7FF);
	reg |= ((imr->img.dh - 1) << 0);
	out32(imr->vbase + RCAR_IMRLX4_SVSR, reg);

	//Source stride and destination stride
	reg = in32(imr->vbase + RCAR_IMRLX4_SSTR);
	reg &= ~(0x3FFF);
	reg |= ((imr->img.dw * imr->img.bpp) << 0);
	out32(imr->vbase + RCAR_IMRLX4_SSTR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_DSTR);
	reg &= ~(0x3FFF);
	reg |= ((imr->img.dw * imr->img.bpp) << 0);
	out32(imr->vbase + RCAR_IMRLX4_DSTR, reg);

	//Triangle mode: Texture mapping enable, clockwise drawing
	reg = in32(imr->vbase + RCAR_IMRLX4_TRIMR);
	reg &= ~(0x47 << 0);
	reg |= (1 << 0) | (1 << 6);
	out32(imr->vbase + RCAR_IMRLX4_TRIMSR, reg);

	//X Clip MIN, X Clip MAX
	reg = in32(imr->vbase + RCAR_IMRLX4_XMINR);
	reg &= ~(0x1FFF);
	reg |= (imr->img.cx << 0);
	out32(imr->vbase + RCAR_IMRLX4_XMINR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_XMAXR);
	reg &= ~(0x1FFF);
	reg |= (imr->img.dw << 0);
	out32(imr->vbase + RCAR_IMRLX4_XMAXR, reg);

	//Y Clip MIN, Y Clip MAX
	reg = in32(imr->vbase + RCAR_IMRLX4_YMINR);
	reg &= ~(0x1FFF);
	reg |= (imr->img.cy << 0);
	out32(imr->vbase + RCAR_IMRLX4_YMINR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_YMAXR);
	reg &= ~(0x1FFF);
	reg |= (imr->img.dh << 0);
	out32(imr->vbase + RCAR_IMRLX4_YMAXR, reg);

	//Render mode 2: YUV422 enable, YCFORM(UYVY)
	reg = in32(imr->vbase + RCAR_IMRLX4_CMRCR2);
	reg &= ~(0x64 << 0);
	reg |= (1 << 2) | (1 << 5) | (1 << 12) | (1 << 15);
	out32(imr->vbase + RCAR_IMRLX4_CMRCSR2, reg);

	//Triangle color: YCFORM(UYVY)
	reg = in32(imr->vbase + RCAR_IMRLX4_TRICR);
	reg |= (1 << 31);
	out32(imr->vbase + RCAR_IMRLX4_TRICR, reg);

	//Render mode 1: Luminance/Hue correction enable
	//Use correction offset parameter specified by register
	reg = in32(imr->vbase + RCAR_IMRLX4_CMRCR);
	reg &= ~((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) |
			 (1 << 5) | (1 << 6) | (1 << 9) | (1 << 12) |
			 (1 << 16) | (1 << 17) | (1 << 18) | (1 << 19));
	reg |= (1 << 1) | (1 << 2) | (1 << 16) |
		   (1 << 17) | (1 << 18) | (1 << 19);
	out32(imr->vbase + RCAR_IMRLX4_CMRCSR, reg);

	//Minimum/Maximum Luminance
	reg = in32(imr->vbase + RCAR_IMRLX4_YLMINR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.Y.min;
	out32(imr->vbase + RCAR_IMRLX4_YLMINR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_YLMAXR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.Y.max;
	out32(imr->vbase + RCAR_IMRLX4_YLMAXR, reg);

	//Minimun/Maximum Hue
	reg = in32(imr->vbase + RCAR_IMRLX4_UBMINR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.U.min;
	out32(imr->vbase + RCAR_IMRLX4_UBMINR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_UBMAXR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.U.max;
	out32(imr->vbase + RCAR_IMRLX4_UBMAXR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_VRMINR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.V.min;
	out32(imr->vbase + RCAR_IMRLX4_VRMINR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_VRMAXR);
	reg &= ~(0xFFF);
	reg |= imr->img_conf.V.max;
	out32(imr->vbase + RCAR_IMRLX4_VRMAXR, reg);

	//Correction decimal point
	reg = in32(imr->vbase + RCAR_IMRLX4_CPDPOR);
	reg &= ~((7 << 0) | (7 << 4) | (7 << 8));
	reg |= (0 << 0) | (0 << 4) | (0 << 8);
	out32(imr->vbase + RCAR_IMRLX4_CPDPOR, reg);

	//Luminance correction parameter
	reg = in32(imr->vbase + RCAR_IMRLX4_YLCPR);
	reg &= ~(0xFFFF);
	reg |= ((imr->img_conf.Y.scal) << 8) | ((imr->img_conf.Y.offs) << 0);
	out32(imr->vbase + RCAR_IMRLX4_YLCPR, reg);

	//Hue correction parameter
	reg = in32(imr->vbase + RCAR_IMRLX4_UBCPR);
	reg &= ~(0xFFFF);
	reg |= ((imr->img_conf.U.scal) << 8) | ((imr->img_conf.U.offs) << 0);
	out32(imr->vbase + RCAR_IMRLX4_UBCPR, reg);
	reg = in32(imr->vbase + RCAR_IMRLX4_VRCPR);
	reg &= ~(0xFFFF);
	reg |= ((imr->img_conf.V.scal) << 8) | ((imr->img_conf.V.offs) << 0);
	out32(imr->vbase + RCAR_IMRLX4_VRCPR, reg);
}

void rcar_imrlx4_DL_init()
{
	uint32_t memSize, squareW, squareH,
			 numSquare, numTri, numVertex;
	uint32_t *display_list;
	int offs1 = 4, offs2 = 0;

	squareW = imr->img_conf.squareW;
	squareH = imr->img_conf.squareH;
	numSquare = (imr->img.dw / squareW) * (imr->img.dh / squareH);
	numTri = numSquare * 2; //1 square is divided into 2 triangles
	numVertex = numTri * 3;

	//4 byte coordinate for every vertex
	//4 byte dummy luminance for every vertex
	//4 byte dummy hue for every vertex
	//12 byte for TRI, SYNCM, TRAP instruction
	memSize = (numVertex * 4 + 3) * 4;
	display_list = mmap(0, memSize, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_ANON | MAP_PHYS | MAP_PRIVATE, NOFD, 0);
	memset(display_list, 0, memSize);

	//TRI
	memset((uint8_t*)display_list, numVertex, 1);
	memset((uint8_t*)display_list + 1, numVertex >> 8, 1);
	memset((uint8_t*)display_list + 3, RCAR_IMRLX4_INST_TRI, 1);

	for(int k = 1; k <= numTri; k++){
		int C = ((k - 1) / 2) % (imr->img.dw / squareW);   //integer modulo
		int R = (k - 1) / (2 * (imr->img.dw / squareW));  //integer division
		if (k % 2 != 0){
		    //coords = (C, R + 1); (C, R); (C + 1, R);
			uint16_t coordsX;
			uint16_t coordsY;
			coordsX = C * squareW;
			coordsY = (R + 1) * squareH;
			//u, v coordinates
			memset((uint8_t*)display_list + 0 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 1 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 2 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 3 + offs1, coordsX >> 8, 1);
			//X, Y coordinates
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 4 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 5 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 6 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 7 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 1 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );

			coordsX = C * squareW;
			coordsY = R * squareH;
			memset((uint8_t*)display_list + 16 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 17 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 18 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 19 + offs1, coordsX >> 8, 1);
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 20 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 21 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 22 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 23 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 5 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );

			coordsX = (C + 1) * squareW;
			coordsY = R * squareH;
			memset((uint8_t*)display_list + 32 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 33 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 34 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 35 + offs1, coordsX >> 8, 1);
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 36 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 37 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 38 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 39 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 9 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );
		}
		else{
		    //coords = (C + 1, R); (C, R + 1); (C + 1, R + 1);
			uint16_t coordsX;
			uint16_t coordsY;
			coordsX = (C + 1) * squareW;
			coordsY = R * squareH;
			memset((uint8_t*)display_list + 0 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 1 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 2 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 3 + offs1, coordsX >> 8, 1);
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 4 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 5 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 6 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 7 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 1 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );

			coordsX = C * squareW;
			coordsY = (R + 1) * squareH;
			memset((uint8_t*)display_list + 16 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 17 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 18 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 19 + offs1, coordsX >> 8, 1);
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 20 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 21 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 22 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 23 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 5 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );

			coordsX = (C + 1) * squareW;
			coordsY = (R + 1) * squareH;
			memset((uint8_t*)display_list + 32 + offs1, coordsY, 1);
			memset((uint8_t*)display_list + 33 + offs1, coordsY >> 8, 1);
			memset((uint8_t*)display_list + 34 + offs1, coordsX, 1);
			memset((uint8_t*)display_list + 35 + offs1, coordsX >> 8, 1);
			if(imr->img_conf.def_set == 1)
			{
				memset((uint8_t*)display_list + 36 + offs1, coordsY, 1);
				memset((uint8_t*)display_list + 37 + offs1, coordsY >> 8, 1);
				memset((uint8_t*)display_list + 38 + offs1, coordsX, 1);
				memset((uint8_t*)display_list + 39 + offs1, coordsX >> 8, 1);
			}
			else memcpy(display_list + 9 + (offs1 / 4), imr->img_conf.coords + (offs2++), 4 );
		}
		offs1 += 48;
	}
	//SYNCM
	memset((uint8_t*)display_list + (memSize - 5), RCAR_IMRLX4_INST_SYNCM, 1);
	//TRAP
	memset((uint8_t*)display_list + (memSize - 1), RCAR_IMRLX4_INST_TRAP, 1);

	//Display List Start Address Register
	out32(imr->vbase + RCAR_IMRLX4_DLSAR, rcar_imrlx4_mphys(display_list));
}

void rcar_imrlx4_update_frame(paddr_t source, paddr_t dest)
{
	//Stop render
	out32(imr->vbase + RCAR_IMRLX4_CR, (0 << 0));

	//Start source address
	out32(imr->vbase + RCAR_IMRLX4_SSAR, rcar_imrlx4_mphys((void*)source));

	//Start destination address
	out32(imr->vbase + RCAR_IMRLX4_DSAR, rcar_imrlx4_mphys((void*)dest));

	//Start render
	out32(imr->vbase + RCAR_IMRLX4_CR, (1 << 0));
}

void *rcar_imrlx4_event_handler(void *data)
{
	struct _pulse pulse;
	iov_t iov;
	int	rcvid;
	uint32_t stat;
	int retry = 0;

	rcar_imr_t *imr = (rcar_imr_t*)data;

	SETIOV(&iov, &pulse, sizeof(pulse));

	for (;;) {
		if ((rcvid = MsgReceivev(imr->chid, &iov, 1, NULL)) == -1)
			continue;

		switch (pulse.code){
			case RCAR_IMRLX4_PULSE:
				//Status
				stat = in32(imr->vbase + RCAR_IMRLX4_SR);
				//Clear status
				out32(imr->vbase + RCAR_IMRLX4_SRCR, stat);
				if(stat & 0x01){
					MsgSendPulse(imr->img.hcoid, 21, imr->img.pulse, 0);
				}
				else{
					retry++;
					if(retry == 30){
						if(stat & 0x02)
							fprintf(stderr, "An illegal instruction has been decoded in the DL \r\n");
						return NULL;
					}
				}
				InterruptUnmask(imr->irq, imr->iid);
				break;
			case RCAR_IMRLX4_END:
				return NULL;
			default:
				if (rcvid)
					MsgReplyv(rcvid, ENOTSUP, &iov, 1);
				break;
		}
	}
	return 0;
}

int rcar_imrlx4_create_thread()
{
	ThreadCtl(_NTO_TCTL_IO, 0);

	if ((imr->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return -1;

	if ((imr->coid = ConnectAttach(0, 0, imr->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		goto fail;

	pthread_attr_init(&imr->attr);
	pthread_attr_setschedpolicy(&imr->attr, SCHED_RR);
	imr->param.sched_priority = 21;
	pthread_attr_setschedparam(&imr->attr, &imr->param);
	pthread_attr_setinheritsched(&imr->attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setdetachstate(&imr->attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&imr->attr, 8192);

	imr->event.sigev_notify   = SIGEV_PULSE;
	imr->event.sigev_coid     = imr->coid;
	imr->event.sigev_code     = RCAR_IMRLX4_PULSE;
	imr->event.sigev_priority = 21;

	// Create imrlx4 event handler
	if (pthread_create(&imr->tid, &imr->attr, (void *)rcar_imrlx4_event_handler, imr)) {
		fprintf(stderr, "%s:  Unable to create event handler\n", __FUNCTION__);
		goto fail;
	}
	if ((imr->iid = InterruptAttachEvent(imr->irq, &imr->event, _NTO_INTR_FLAGS_TRK_MSK|_NTO_INTR_FLAGS_END)) == -1){
		fprintf(stderr,"%s: Interrupt attach failed.\n", __FUNCTION__);
		goto fail;
	}

	return 0;

fail:
	ConnectDetach(imr->coid);
	ChannelDestroy(imr->chid);
	return -1;
}

int rcar_imrlx4_init(img_info_t img)
{
	if((imr = calloc(1, sizeof(rcar_imr_t))) == NULL) {
		fprintf(stderr, "%s: calloc failed \r\n", __FUNCTION__);
		return 0;
	}

	imr->img.hcoid = img.hcoid;
	imr->img.pulse = img.pulse;
	imr->img.channel = img.channel;
	imr->img.cx = img.cx;
	imr->img.cy = img.cy;
	imr->img.dw = img.dw;
	imr->img.dh = img.dh;
	imr->img.bpp = img.bpp;

	/* Physical base address and interrupt number */
	switch(imr->img.channel) {
		case 0:
			imr->pbase = RCAR_IMRLX40_BASE;
			imr->irq = RCAR_INTCSYS_IMRLX40;
			break;
		case 1:
			imr->pbase = RCAR_IMRLX41_BASE;
			imr->irq = RCAR_INTCSYS_IMRLX41;
			break;
		case 2:
			imr->pbase = RCAR_IMRLX42_BASE;
			imr->irq = RCAR_INTCSYS_IMRLX42;
			break;
		case 3:
			imr->pbase = RCAR_IMRLX43_BASE;
			imr->irq = RCAR_INTCSYS_IMRLX43;
			break;
		default:
			fprintf(stderr, "%s: Not supported IMR channel \r\n", __FUNCTION__);
			return -1;
	}

	if ((imr->vbase = (uintptr_t)mmap_device_io(RCAR_IMRLX4_SIZE, imr->pbase)) == (uintptr_t)MAP_FAILED) {
        fprintf(stderr, "%s: IMRLX4 base mmap_device_io (0x%x) failed \r\n", __FUNCTION__, (uint32_t)imr->pbase);
        rcar_imrlx4_fini();
        return (errno);
    }

	if(parse_device_config(imr) == -1)
	{
		imr->img_conf.def_set = 1;
		//square width, square height(unit use to divide image)
		if(imr->img.dw == 1920 && imr->img.dh == 1080)
		{
			imr->img_conf.squareW = 24;
			imr->img_conf.squareH = 24;
		}
		else if(imr->img.dw == 800 && imr->img.dh == 600)
		{
			imr->img_conf.squareW = 8;
			imr->img_conf.squareH = 8;
		}
		else 
        {
			imr->img_conf.squareW = 16;
			imr->img_conf.squareH = 16;
        }
		//Y 8-bpp precision
		imr->img_conf.Y.min = 0;
		imr->img_conf.Y.max = 255;
		imr->img_conf.Y.scal = 1;
		imr->img_conf.Y.offs = 0;
		//U 8-bpp precision
		imr->img_conf.U.min = 0;
		imr->img_conf.U.max = 255;
		imr->img_conf.U.scal = 1;
		imr->img_conf.U.offs = 0;
		//V 8-bpp precision
		imr->img_conf.V.min = 0;
		imr->img_conf.V.max = 255;
		imr->img_conf.V.scal = 1;
		imr->img_conf.V.offs = 0;
		fprintf(stderr, "%s: Use default setting \r\n", __FUNCTION__);
	}

	/* Enable IMRLX4 */
	rcar_imrlx4_setup();

	return 0;
}

int rcar_imrlx4_fini()
{
	/* Stop IMRLX4 */
	out32(imr->vbase + RCAR_IMRLX4_CR, (0 << 0));

	MsgSendPulse(imr->coid, 21, RCAR_IMRLX4_END, 0);
	usleep(10);

	pthread_cancel(imr->tid);
	pthread_join(imr->tid, NULL);
	InterruptDetach(imr->iid);
	ConnectDetach(imr->coid);
	ChannelDestroy(imr->chid);

	munmap_device_io(imr->vbase, RCAR_IMRLX4_SIZE);

	free(imr);

	return 0;
}
