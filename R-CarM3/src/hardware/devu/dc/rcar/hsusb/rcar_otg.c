/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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


#include <fcntl.h>
#include <descriptors.h>

#include "rcar_otg.h"

extern char usbdc_config_descriptor[];
extern const struct rcar_pipe_config rcar_pipe[RCAR_MAX_NUM_PIPE];


static iousb_pipe_methods_t rcar_control_pipe_methods = {
    rcar_control_endpoint_enable,
    rcar_control_endpoint_disable,
    rcar_control_transfer,
    rcar_control_transfer_abort,
    NULL
};

static iousb_pipe_methods_t rcar_interrupt_pipe_methods = {
    rcar_endpoint_enable,
    rcar_endpoint_disable,
    rcar_transfer,
    rcar_transfer_abort,
    NULL
};

static iousb_pipe_methods_t rcar_bulk_pipe_methods = {
    rcar_endpoint_enable,
    rcar_endpoint_disable,
    rcar_transfer,
    rcar_transfer_abort,
    NULL
};

static iousb_pipe_methods_t rcar_isoch_pipe_methods = {
    rcar_endpoint_enable,
    rcar_endpoint_disable,
    rcar_transfer,
    rcar_transfer_abort,
    NULL
};


dc_methods_t rcar_controller_methods = {
    20,
    rcar_init,
    rcar_start,
    rcar_stop,
    rcar_shutdown,
    rcar_dma_malloc,
    rcar_dma_free,
    rcar_set_bus_state,
    rcar_set_device_feature,
    rcar_clear_device_feature,
    rcar_set_device_address,
    rcar_get_descriptor,
    rcar_select_configuration,
    rcar_interrupt,
    rcar_set_endpoint_state,
    rcar_clear_endpoint_state,
    NULL,
    &rcar_control_pipe_methods,
    &rcar_interrupt_pipe_methods,
    &rcar_bulk_pipe_methods,
    &rcar_isoch_pipe_methods,
};

usb_controller_methods rcar_usb_controller_methods = {
    NULL,
    &rcar_controller_methods,
    NULL,
    NULL
};

io_usb_dll_entry_t io_usb_dll_entry = {
    USBDC_DLL_NAME,
    0xffff,  // pci device id
    0xffff,
    0xffff,
    USB_CONTROLLER_DEVICE,
    NULL,
    NULL,
    &rcar_usb_controller_methods
};

void rcar_complete_urb( rcar_dctrl_t * dc, rcar_ep_t *ep, uint32_t urb_status ) {
    iousb_transfer_t *urb = ep->urb;
    rcar_dctrl_t *rcar = ep->dc;

    if ( urb ) {
        // we have sent/received everyting ... complete the urb
        urb->actual_len = ep->bytes_xfered;
        ep->urb = 0;
        /* if(ep->pipenum)
           RCAR_SLOGF_DBG(dc, _SLOG_DEBUG1,
           "URB COMPLETE epnum = 0x%x epdir = 0x%x urb->actual_len = %d ",
           ep->pipenum, ep->dir, urb->actual_len  );
        */
        rcar_pthread_mutex_lock( &rcar->usb_mutex );
        if(ep->req_xfer_len)
        {
            munmap_device_memory((void *)ep->xfer_buffer, ep->req_xfer_len);
            ep->xfer_buffer = 0;
        }

        urb->urb_complete_cbf( ep->iousb_ep, urb, urb_status, 0);
        rcar_pthread_mutex_unlock( &rcar->usb_mutex );
    }
}
void
rcar_slogf( rcar_dctrl_t * dc, int level, const char *fmt, ...)
{
    va_list arglist;

    if ( dc && ( level > dc->verbosity ) )
        return;

    va_start( arglist, fmt );
    vslogf( 12, level, fmt, arglist );
    va_end( arglist );
    return;
}

int rcar_xfer_abort( rcar_dctrl_t * dc, rcar_ep_t *ep )
{
    rcar_pthread_mutex_lock(&dc->usb_mutex);
    RCAR_SLOGF_DBG(dc, _SLOG_DEBUG2, "%s: aborting endpoint = %d", __func__, ep->pipenum );

    if(ep->req_xfer_len)
    {
        RCAR_SLOGF_DBG(dc, _SLOG_DEBUG2, "%s: free transfer buffer momory", __func__ );
        munmap_device_memory((void *)ep->xfer_buffer, ep->req_xfer_len);
        ep->xfer_buffer = 0;
    }

    rcar_pthread_mutex_unlock(&dc->usb_mutex);
    return EOK;
}


uint32_t rcar_interrupt( usbdc_device_t *udc )
{
    rcar_dctrl_t            *dc = udc->dc_data;

    rcar_irq_process(dc);
    return EOK;
}



static void * delayed_execution_handler( rcar_dctrl_t * dc ) {
    struct _pulse       pulse;

    while( 1 ) {
        if( ( MsgReceivePulse( dc->chid, &pulse, sizeof( pulse ), NULL ) ) == -1 ) {
            continue;
        }

        switch( pulse.code ) {
            case PULSE_DELAY_SETUP_PKT:
                RCAR_SLOGF_DBG( dc, _SLOG_INFO, "%s() : PULSE_DELAY_SETUP_PKT received", __func__ );
                dc->udc->usbdc_self->usbdc_setup_packet_process( dc->udc, (uint8_t *)dc->setup_packet );

                break;

            case RCAR_CODE_TIMER:
                if(dc->vbus_active == 0)
                {
                    if(dc->softconnect)
                    {
                        rcar_disconnect( dc );
                        rcar_vbus_session(dc, 1);
                    }
                }

                break;

            case RCAR_DMAC_IRQ_EVENT:
                rcar_dma_irq(dc);
                InterruptUnmask(dc->irq_dma, dc->iid_dma);

                break;

            default:
                RCAR_SLOGF_DBG( dc, _SLOG_ERROR, "%s() : Unknown pulse code %d", __func__ , pulse.code );

                break;

        }
    }
    return NULL;
}


uint32_t
rcar_set_device_address(  usbdc_device_t *udc, uint32_t address )
{
    rcar_dctrl_t    *dc = udc->dc_data;

    RCAR_SLOGF_DBG(dc, _SLOG_INFO, "%s: address = 0x%x", __func__, address);

    control_end(dc, 1);
    return EOK;
}


uint32_t
rcar_select_configuration( usbdc_device_t *udc, uint8_t config )
{
    rcar_dctrl_t        *dc = udc->dc_data;

    control_end(dc, 1);
    return EOK;
}


uint32_t
rcar_set_endpoint_state( usbdc_device_t *udc, iousb_endpoint_t *iousb_ep, uint32_t ep_state )
{
    rcar_dctrl_t    *dc         = udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;

    switch ( ep_state ) {
        case IOUSB_ENDPOINT_STATE_READY :
            break;

        case IOUSB_ENDPOINT_STATE_STALLED :
            ep->flags |= EPFLAG_STALLED;
            pipe_stall(dc, iousb_ep->edesc.bEndpointAddress & 0x7f);
            break;

        case IOUSB_ENDPOINT_STATE_RESET :
            break;
        case IOUSB_ENDPOINT_STATE_ENABLE :
            break;

        case IOUSB_ENDPOINT_STATE_DISABLED :
            break;

        case IOUSB_ENDPOINT_STATE_NAK :
            break;

        default :
            break;
    }


    return EOK;
}

uint32_t
rcar_clear_endpoint_state( usbdc_device_t *udc, iousb_endpoint_t *iousb_ep, uint32_t ep_state )
{
    rcar_dctrl_t        *dc = udc->dc_data;
    rcar_ep_t           *ep = iousb_ep->user;
    uint8_t         pipenum = iousb_ep->edesc.bEndpointAddress & 0x7f;

    switch ( ep_state ) {
        case IOUSB_ENDPOINT_STATE_READY :
            break;

        case IOUSB_ENDPOINT_STATE_STALLED :
            pipe_stop(dc, pipenum );
            control_reg_sqclr(dc, pipenum);
            control_end(dc, 1);
            ep->flags &= ~EPFLAG_STALLED;

            break;

        case IOUSB_ENDPOINT_STATE_RESET :
            break;

        case IOUSB_ENDPOINT_STATE_ENABLE :
            break;

        case IOUSB_ENDPOINT_STATE_DISABLED :
            break;

        case IOUSB_ENDPOINT_STATE_NAK :
            break;

        default :
            break;
    }

    return EOK;
}

int rcar_disconnect( rcar_dctrl_t  * dc ) {
    int         rc = EOK;
    int         i;
    rcar_ep_t    *ep;
    //unidrectional non-control endpoints assumed
    for( i=1; i < dc->n_ep; i++ ) {
        ep = &dc->ep_arr[i];
        if ( ep->flags & EPFLAG_XFER_ACTIVER ) {
            ep->flags &= ~EPFLAG_XFER_ACTIVER;
            rcar_xfer_abort(dc, ep);
        }
    }

    delay( 1 );
    return rc;
}

/* call to enable connection to host or signal resume to host */
uint32_t
rcar_set_bus_state( usbdc_device_t *udc, uint32_t device_state )
{
    rcar_dctrl_t  *rcar = udc->dc_data;
    switch ( device_state ) {
        case IOUSB_BUS_STATE_DISCONNECTED :
            printf("vbus disconnect\n");
            rcar_disconnect( rcar );
            rcar_vbus_session(rcar, 0);
            rcar->softconnect = 0;
            rcar->udc->usbdc_self->usbdc_device_state_change( rcar->udc, IOUSB_DEVICE_STATE_REMOVED);
            break;

        case IOUSB_BUS_STATE_CONNECTED :
            // do soft connect
            rcar->softconnect = 1;
            break;

        case IOUSB_BUS_STATE_RESUME :
            break;
    }


    return EOK;
}

/* enable a feature of a device */
uint32_t
rcar_set_device_feature( usbdc_device_t *udc, uint32_t feature, uint16_t wIndex )
{
    rcar_dctrl_t  *rcar = udc->dc_data;
    rcar_ep_t *ep;
    switch ( feature ) {
        case USB_FEATURE_EPT_HALT :

            ep = rcar->epaddr2ep[wIndex & 0x0f];
            pipe_stall(rcar, ep->pipenum);

            control_end(rcar, 1);

            return( ENOTSUP );
            break;

        case USB_FEATURE_DEV_WAKEUP :
            control_end(rcar, 1);
            return( ENOTSUP );
            break;

        case USB_FEATURE_TEST_MODE :
            control_end(rcar, 1);
            /* Wait for the completion of status stage */
            rcar_feature_test_mode_wait_complete(rcar, wIndex);

            return( ENOTSUP );
            break;
        default :
            pipe_stall(rcar, 0);

            return( ENOTSUP );
            break;
    }

    return( ENOTSUP );
}

/* clear a feature of a device */
uint32_t
rcar_clear_device_feature( usbdc_device_t *udc, uint32_t feature )
{
    rcar_dctrl_t  *rcar = udc->dc_data;
    switch ( feature ) {
        case USB_FEATURE_DEV_WAKEUP :
            control_end(rcar, 1);
            return( ENOTSUP );
            break;

        case USB_FEATURE_TEST_MODE :
            control_end(rcar, 1);
            return( ENOTSUP );
            break;
        default :
            pipe_stall(rcar, 0);
            return( ENOTSUP );
    }

    return( ENOTSUP );
}

uint32_t
rcar_get_descriptor( usbdc_device_t *udc, uint8_t type, uint8_t index, uint16_t lang_id,
                     uint8_t **desc, uint32_t speed )
{
    switch ( type ) {
        case USB_DESC_DEVICE :
            switch( speed ) {
                case IOUSB_DEVICE_HIGH_SPEED:
                    *desc = (uint8_t *) USBDC_HS_DEVICE_DESCRIPTOR;
                    break;
                case IOUSB_DEVICE_SUPER_SPEED:
                    *desc = (uint8_t *) USBDC_SS_DEVICE_DESCRIPTOR;
                    break;
                case IOUSB_DEVICE_FULL_SPEED:
                default:
                    *desc = (uint8_t *) USBDC_FS_DEVICE_DESCRIPTOR;
                    break;
            }
            break;

        case USB_DESC_CONFIGURATION :
            if ( index < USBDC_NUM_CONFIGURATIONS ) {
                switch( speed ) {
                    case IOUSB_DEVICE_HIGH_SPEED:
                        *desc = (uint8_t *) USBDC_HS_CONFIG_DESCRIPTOR[index];
                        break;
                    case IOUSB_DEVICE_SUPER_SPEED:
                        *desc = (uint8_t *) USBDC_SS_CONFIG_DESCRIPTOR[index];
                        break;
                    case IOUSB_DEVICE_FULL_SPEED:
                    default:
                        *desc = (uint8_t *) USBDC_FS_CONFIG_DESCRIPTOR[index];
                        break;
                }
            } else {
                return( ENOTSUP );
            }
            break;

        case USB_DESC_STRING :
            if ( index <= USBDC_MAX_STRING_DESCRIPTOR ) {
                switch( speed ) {
                    case IOUSB_DEVICE_HIGH_SPEED:
                        *desc = (uint8_t *) USBDC_HS_STRING_DESCRIPTOR[index];
                        break;
                    case IOUSB_DEVICE_SUPER_SPEED:
                        *desc = (uint8_t *) USBDC_SS_STRING_DESCRIPTOR[index];
                        break;
                    case IOUSB_DEVICE_FULL_SPEED:
                    default:
                        *desc = (uint8_t *) USBDC_FS_STRING_DESCRIPTOR[index];
                        break;
                }

            } else {
                return ENOTSUP;
            }
            break;

        case USB_DESC_DEVICE_QUALIFIER :
            switch( speed ) {
                case IOUSB_DEVICE_HIGH_SPEED:
                    *desc = (uint8_t *) USBDC_HS_DEVICE_QUALIFIER_DESCRIPTOR;
                    break;
                case IOUSB_DEVICE_SUPER_SPEED:
                    *desc = (uint8_t *) USBDC_SS_DEVICE_QUALIFIER_DESCRIPTOR;
                    break;
                case IOUSB_DEVICE_FULL_SPEED:
                default:
                    *desc = (uint8_t *) USBDC_FS_DEVICE_QUALIFIER_DESCRIPTOR;
                    break;
            }
            break;

        case USB_DESC_OTHER_SPEED_CONF :
            // What about SuperSpeed
            RCAR_SLOGF_DBG(dc, _SLOG_INFO, "%s : get USB_DESC_OTHER_SPEED_CONF speed = %d index = %d",
                       __func__, speed, index);
            *desc  = (speed == IOUSB_DEVICE_HIGH_SPEED) ? (uint8_t *) USBDC_HS_CONFIG_DESCRIPTOR[0] :
                  (uint8_t *) USBDC_FS_CONFIG_DESCRIPTOR[0];

            break;


        case USB_DESC_INTERFACE_POWER :
        case USB_DESC_INTERFACE :
        case USB_DESC_ENDPOINT :
        default :
            return ENOTSUP;
            break;
    }
    return EOK;
}

/* This function is meant to configure ep0 and should only be called at
 * driver init time
 */
int ep0_cfg( rcar_dctrl_t * dc ) {
    rcar_ep_t               *ep = &dc->ep_arr[0];
    ep->mps = rcar_pipe[0].maxpacket;
    ep->pipenum = 0;
    ep->dir = 0;
    ep->type = USB_ATTRIB_CONTROL;
    ep->urb = 0;
    ep->xfer_flags = 0;
    ep->req_xfer_len = 0;

    ep->flags = EPFLAG_ENABLED;
    ep->use_dma = 0;
    return EOK;
}

void ep0_decfg( rcar_dctrl_t * dc ) {
    rcar_ep_t               *ep = &dc->ep_arr[0];

    ep->flags &= ~EPFLAG_ENABLED;
}


void *rcar_dma_malloc(usbdc_device_t *udc, size_t len)
{
    rcar_dctrl_t    *dc = ( rcar_dctrl_t * ) udc->dc_data;
    void            *rc;

    if (dc->tpmem_fd == -1) {
        if ((dc->tpmem_fd = posix_typed_mem_open("below4G", O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG)) < 0) {
            RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: - Cannot open \"below4G\" typed-memory", __FUNCTION__);
            return NULL;
        }
    }

    rc = mmap64(0, len, PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_SHARED, dc->tpmem_fd, 0);

    if (rc == MAP_FAILED) {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: - malloc dma failed", __FUNCTION__);
        return NULL;
    }

    return rc;
}
uint32_t rcar_dma_free(usbdc_device_t *udc, uint32_t *addr, size_t len)
{
    munmap((void *)addr, len);
    addr = 0;
    return EOK;
}
_uint32
rcar_control_endpoint_enable( void *chdl, iousb_device_t *device, iousb_endpoint_t *iousb_ep )
{
    usbdc_device_t  *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t        *dc = ( rcar_dctrl_t * ) udc->dc_data;

    /* ep0 has already been configured in ep0_cfg()... all we have to do is link it to the
     * generic parent struct. This code will have to change to support non ep0 control enpdpoints
     */
    iousb_ep->user = &dc->ep_arr[0];
    return EOK;
}


// used by both bulk and interrupt endpoints
_uint32
rcar_endpoint_enable( void *chdl, iousb_device_t *device, iousb_endpoint_t *iousb_ep )
{
    usbdc_device_t  *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t    *rcar = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;
    uint32_t        epnum = iousb_ep->edesc.bEndpointAddress & 0x7f;
    uint32_t        epdir = iousb_ep->edesc.bEndpointAddress & USB_ENDPOINT_IN;

    RCAR_SLOGF_DBG(rcar, _SLOG_INFO, "%s: ep = 0x%x mps = %d", __func__,
               iousb_ep->edesc.bEndpointAddress, iousb_ep->edesc.wMaxPacketSize);

    if ( !iousb_ep->user ) {
        ep = iousb_ep->user = &rcar->ep_arr[epnum];
        ep->iousb_ep = iousb_ep;
        ep->dc = rcar;
        ep->mps = iousb_ep->edesc.wMaxPacketSize;
        ep->pipenum = epnum;
        ep->dir = epdir;
        ep->type = iousb_ep->edesc.bmAttributes & USB_ENDPOINT_ATTRIB_ETYPE_MASK;
        ep->flags = EPFLAG_ENABLED;
        ep->intval = iousb_ep->edesc.bInterval;
        rcar_ep_enable(ep);
    }
    return EOK;
}

_uint32
rcar_control_endpoint_disable( void *chdl, iousb_endpoint_t *iousb_ep )
{
    return ( EOK );
}

_uint32
rcar_endpoint_disable( void *chdl, iousb_endpoint_t *iousb_ep )
{
    usbdc_device_t  *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t    *rcar = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;

    rcar_pthread_mutex_lock(&rcar->usb_mutex);
    RCAR_SLOGF_DBG(rcar, _SLOG_INFO, "%s: epnum = %d", __func__,ep->pipenum );

    rcar_ep_disable(ep);

    ep->flags &= ~EPFLAG_ENABLED;
    iousb_ep->user = 0;
    rcar_pthread_mutex_unlock(&rcar->usb_mutex);
    return ( EOK );
}

// assumes control endpoint is the default control pipe only, and uses epidx0 and epidx1
_uint32
rcar_control_transfer_abort( void *chdl, iousb_transfer_t *urb, iousb_endpoint_t *iousb_ep )
{
    usbdc_device_t      *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t        * dc = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;

    if ( ep == NULL ) {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: ep==NULL", __func__);
        return EOK;
    }


    RCAR_SLOGF_DBG(dc, _SLOG_DEBUG1, "%s: ep = 0x%x  ep->pipenum = %d ", __func__,
                   iousb_ep->edesc.bEndpointAddress, ep->pipenum);

    // abort any active transfers
    if ( ( ep->flags & EPFLAG_XFER_ACTIVER ) ) {
        rcar_xfer_abort( dc, ep);
    }
    ep->flags &= ~EPFLAG_XFER_ACTIVER;
    ep->urb = 0;

    return EOK;
}

_uint32
rcar_control_transfer( void *chdl, iousb_transfer_t *urb, iousb_endpoint_t *iousb_ep,
                       uint8_t *buffer, _uint32 length, _uint32 flags )
{
    usbdc_device_t  *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t        *dc = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;
    uint32_t            idx = ( flags & PIPE_FLAGS_TOKEN_IN ) ? 1 : 0;

    rcar_pthread_mutex_lock(&dc->usb_mutex);
    ep->urb = urb;  //backref
    ep->dc = dc;    //backref
    ep->xfer_flags = flags;
    ep->flags |= EPFLAG_XFER_ACTIVER;

    if ( ( flags & PIPE_FLAGS_TOKEN_OUT ) && !( flags & PIPE_FLAGS_TOKEN_STATUS ) &&
         ( length % ep->mps ) ) {
        // massage the length to be module mps
        length = ep->mps * ( ( length / ep->mps ) + 1 );
        RCAR_SLOGF_DBG(dc, _SLOG_DEBUG1, "%s: changed length from to %d which is module mps  ",
                       __func__, length );
    }
    ep->req_xfer_len = length;
    ep->bytes_xfered = 0;

    if(ep->req_xfer_len)
    {
        ep->xfer_buffer = mmap_device_memory( NULL, ep->req_xfer_len,
                                              PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, (uint64_t)buffer );
        if(ep->xfer_buffer == MAP_FAILED)
        {
            printf("map xfer buffer failed\n");
            rcar_pthread_mutex_unlock(&dc->usb_mutex);
            return -1;
        }
    }
    else
    {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: xfer lentgh = 0", __func__ );
        ep->xfer_buffer = 0;
    }
    start_ep0(ep, idx);

    rcar_pthread_mutex_unlock(&dc->usb_mutex);
    return EOK;
}


_uint32
rcar_transfer_abort( void *chdl, iousb_transfer_t *urb, iousb_endpoint_t *iousb_ep )
{
    usbdc_device_t  *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t        *dc = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;

    if ( ep ) {
        RCAR_SLOGF_DBG(dc, _SLOG_DEBUG1, "%s:  ep->pipenum = %d", __func__, ep->pipenum );

        if ( ep->flags & EPFLAG_XFER_ACTIVER ) {
            ep->flags &= ~EPFLAG_XFER_ACTIVER;
            rcar_xfer_abort( dc, ep );
        }
        ep->urb = 0;

    } else {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: ep == NULL... cannot abort", __func__ );
    }

    return EOK;
}

_uint32
rcar_transfer( void *chdl, iousb_transfer_t *urb, iousb_endpoint_t *iousb_ep, uint8_t *buffer,
               _uint32 length, _uint32 flags )
{
    usbdc_device_t      *udc = ( usbdc_device_t * ) chdl;
    rcar_dctrl_t        *dc = ( rcar_dctrl_t * ) udc->dc_data;
    rcar_ep_t       *ep = iousb_ep->user;

    rcar_pthread_mutex_lock(&dc->usb_mutex);
    ep->urb = urb;
    ep->xfer_flags = flags;

    if ( ( flags & PIPE_FLAGS_TOKEN_OUT ) && ( length % ep->mps ) ) {
        // massage the length to be module mps
        length = ep->mps * ( ( length / ep->mps ) + 1 );
    }
    ep->req_xfer_len = length;
    ep->bytes_xfered = 0;
    ep->dc= dc;         //backref
    ep->flags |= EPFLAG_XFER_ACTIVER;
    ep->xfer_padd_buffer = buffer;

    if(ep->req_xfer_len)
    {
        ep->xfer_buffer = mmap_device_memory( NULL, length, PROT_READ|PROT_WRITE|PROT_NOCACHE,
                                              0, (uint64_t)buffer );
        if(ep->xfer_buffer == MAP_FAILED)
        {
            rcar_pthread_mutex_unlock(&dc->usb_mutex);
            printf("map xfer buffer failed\n");
            return -1;
        }
    }
    else
    {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: xfer lentgh = 0", __func__ );
        ep->xfer_buffer = 0;
    }

    start_packet(ep);

    rcar_pthread_mutex_unlock(&dc->usb_mutex);

    return EOK;
}


int
rcar_process_args( rcar_dctrl_t *dc, char *args )
{

    int opt;
    char *value;
    char *c;
    int len;

    static char *driver_opts[] = {
        "verbose",
        "ser",
        "dmac",
        "bwait",
        NULL
    };

    dc->serial_string = NULL;
    if( !args )
        return EOK;

    // convert args
    len = strlen( args );
    while ( ( c = strchr( args, ':' ) ) ) {
        if ( c - args > len )
            break;
        *c = ',';
    }
    while( *args != '\0' ) {
        if( ( opt = getsubopt( &args, driver_opts, &value ) ) != -1 ) {
            switch ( opt ) {
                case 0 :
                    if ( value )
                        dc->verbosity = strtoul( value, 0, 0 );
                    else
                        dc->verbosity = 5;

                    continue;
                case 1 :     // this arg should move up for know we build a proper string desc
                    if ( value ) {
                        uint8_t  slen;
                        uint32_t x;
                        // max 8bit length each char requires 2 bytes for encoding plus 2 byte header.
                        slen = min(strlen( value ), 127 );
                        dc->udc->serial_string = calloc( 1, 3 + 2 * slen );
                        dc->udc->serial_string[0] = 2 + slen *2; // string header is 2 bytes
                        dc->udc->serial_string[1] = USB_DESC_STRING;
                        for ( x = 1 ; x < slen + 1 ; x ++ ) {
                            dc->udc->serial_string[x * 2] = value[x - 1];
                        }
                    }
                    continue;
                case 2:
                    // "dmac"
                    dc->dmac = 1;
                    continue;
                case 3:
                    // "buswait"
                    if ( value )
                        dc->bwait = strtoul( value, 0, 0 );
                    else
                        dc->bwait = 9;
                    continue;
                default :
                    break;
            }
        }
    }
    return EOK;
}

static void
default_config_set( usbdc_device_t *udc )
{
    rcar_dctrl_t * dc = ( rcar_dctrl_t * ) udc->dc_data;

    dc->verbosity = _SLOG_DEBUG2;
    dc->flags     = DC_FLAG_UNKNOW_SPEED;

    udc->hw_ctrl.cname = "rcar_otg";
    udc->hw_ctrl.max_transfer_size = 0x10000;
    udc->hw_ctrl.max_unaligned_xfer = 0x10000;
    udc->hw_ctrl.buff_alignment_mask = 0x1;
    udc->hw_ctrl.capabilities   = DC_CAP_FULL_SPEED | DC_CAP_HIGH_SPEED | DC_CAP_TEST_MODES_SUPPORTED;

}

static int create_delayed_execution_handler( rcar_dctrl_t * dc ) {
    pthread_attr_t          attr;
    struct sched_param      param;
    int                     error;

    dc->chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK );
    if ( dc->chid < 0 ) {
        error = errno;
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: failed to create channel (error=%d)",__func__,error);
        goto fail;
    }

    dc->coid = ConnectAttach( 0, 0, dc->chid, _NTO_SIDE_CHANNEL, 0 );
    if ( dc->coid < 0 ) {
        error = errno;
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: failed to attach to channel (error=%d)",__func__);
        goto fail2;
    }

    pthread_attr_init( &attr );
    pthread_attr_setschedpolicy( &attr, SCHED_RR );
    param.sched_priority = 21;
    pthread_attr_setschedparam( &attr, &param );
    pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );

    error = pthread_create( &dc->tid, &attr, (void*)delayed_execution_handler, dc );
    if( error != EOK ) {
        RCAR_SLOGF_DBG(dc, _SLOG_ERROR, "%s: failed to create interrupt handler thread (error = %d) ",
                   __func__,error);
        goto fail3;
    }

    return EOK;

  fail3:
    ConnectDetach( dc->coid );
  fail2:
    ChannelDestroy( dc->chid );
  fail:
    return error;
}

void destroy_delayed_execution_handler( rcar_dctrl_t * dc ) {
    pthread_cancel( dc->tid );
    ConnectDetach( dc->coid );
    ChannelDestroy( dc->chid );
}

uint32_t
rcar_init( usbdc_device_t *udc, io_usbdc_self_t *udc_self, char *args )
{
    rcar_dctrl_t * dc;
    pthread_mutexattr_t mattr;
    int rc;

    /* allocate device ctx */
    dc = udc->dc_data = calloc( 1, sizeof( rcar_dctrl_t ) );
    dc->setup_packet = calloc( 1, sizeof( usb100_setup_packet_t ) );
    memset (&dc->lock, 0, sizeof( intrspin_t ));
    if ( !dc ) {
        RCAR_SLOGF_DBG( NULL, _SLOG_ERROR, "%s calloc failed",__FUNCTION__);
        rc = ENOMEM;
        goto error;
    }
    dc->udc = udc;
    /* set default driver configurations */
    default_config_set( udc );
    /* parse command line arguments to override default configs */
    //set default config
	dc->tpmem_fd = -1;
    dc->dmac=1;
    dc->irq_dma = RCAR_USBDMAC1_IRQ;
    dc->bwait = 9;
    dc->vbus_active = 0;
    dc->softconnect = 0;
    rc = rcar_process_args(dc, args);
    if ( rc ) {
        RCAR_SLOGF_DBG( dc, _SLOG_ERROR, "%s couldn't parse command line args",__FUNCTION__);
        goto error2;
    }

    // map io
    rc = rcar_map_regs(dc);
    if ( rc != EOK ) {
        RCAR_SLOGF_DBG( dc, _SLOG_ERROR, "%s mmap failed",__FUNCTION__);
        rc = ENOMEM;
        goto error2;
    }

    // cache how many endpoints are supported by the controller
    dc->n_ep = RCAR_MAX_NUM_PIPE;
    //enable endpoint 0
    dc->ep_arr[0].pipenum = 0;
    dc->ep_arr[0].fifoaddr = CFIFO;
    dc->ep_arr[0].fifosel = CFIFOSEL;
    dc->ep_arr[0].fifoctr = CFIFOCTR;
    dc->ep_arr[0].pipectr = get_pipectr_addr(0);
    dc->ep_arr[0].mps = rcar_pipe[0].maxpacket;
    dc->ep_arr[0].dir = 0;
    dc->ep_arr[0].type = USB_ATTRIB_CONTROL;
    dc->ep_arr[0].urb = 0;
    dc->ep_arr[0].xfer_flags = 0;
    dc->ep_arr[0].req_xfer_len = 0;
    dc->ep_arr[0].flags = EPFLAG_ENABLED;
    dc->ep_arr[0].use_dma = 0;
    dc->ep_arr[0].control_phase = CONTROL_PHASE_SETUP;

    dc->pipenum2ep[0] = &dc->ep_arr[0];
    dc->epaddr2ep[0] = &dc->ep_arr[0];

    // create the driver mutex
    pthread_mutexattr_init( &mattr );
    pthread_mutexattr_setrecursive( &mattr, PTHREAD_RECURSIVE_ENABLE );
    if( pthread_mutex_init( &dc->usb_mutex, &mattr ) == -1 ) {
        RCAR_SLOGF_DBG( dc, _SLOG_ERROR, "%s could not create mutex",__FUNCTION__);
        rc = ENOMEM;
        goto error3;
    }

    // setup usb controller
    dc->chid = -1;
    rc = create_delayed_execution_handler( dc );
    if ( rc != EOK ) {
        RCAR_SLOGF_DBG( dc, _SLOG_ERROR,
                    "%s failed to created delayed execution handler err = %d",__FUNCTION__, rc);
        goto error4;
    }

    //attche rcar_timer
    if(dc->chid < 0)
    {
        dc->chid = ChannelCreate( _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK );
    }
    //attach timer
    dc->scount = 0;
    rcar_setupPulseAndTimer(dc->chid);
    //attach interrupt
    if(dc->dmac)
    {
        rcar_dmac_attach_intr(dc);
    }
    udc->usbdc_self->usbdc_set_device_speed( udc, IOUSB_DEVICE_FULL_SPEED );

    return EOK;

  error4:
    pthread_mutex_destroy( &dc->usb_mutex );
  error3:
    rcar_unmap_reg(dc);
  error2:
    free(dc->setup_packet);
    free(dc);
  error:
    return rc;
}

uint32_t
rcar_start( usbdc_device_t *udc )
{
    return EOK;
}

uint32_t
rcar_stop( usbdc_device_t *udc )
{
    rcar_dctrl_t * dc = ( rcar_dctrl_t * ) udc->dc_data;

    rcar_udc_stop(dc);
    return EOK;
}

uint32_t
rcar_shutdown( usbdc_device_t *udc )
{
    rcar_dctrl_t * dc = ( rcar_dctrl_t * ) udc->dc_data;
    ep0_decfg( dc );
    rcar_vbus_session(dc, 0);
    destroy_delayed_execution_handler(dc);
    pthread_mutex_destroy( &dc->usb_mutex );
    rcar_unmap_reg(dc);
    if (dc->tpmem_fd != -1)
        close(dc->tpmem_fd);
    free(dc->setup_packet);
    free(dc);

    return EOK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devu/dc/rcar/hsusb/rcar_otg.c $ $Rev: 810496 $")
#endif
