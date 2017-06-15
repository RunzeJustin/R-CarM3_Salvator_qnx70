/*
 * $QNXLicenseC:
 * Copyright 2014, 2016 QNX Software Systems.
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

#include "rcar_support.h"

static rcar_version_t rcar_version = RCAR_VERSION_H2;

inline int rcar_version_set( rcar_version_t version ) {
    switch( version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_E2:
    case RCAR_VERSION_V2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
        rcar_version = version;
        return EOK;
    default:
        return EINVAL;
    }
}

inline rcar_version_t rcar_version_get( void ) {
    return rcar_version;
}

bool rcar_ssi_supported( uint32_t ssi_channel ) {
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_E2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        if( ssi_channel >= 0 && ssi_channel <= 9 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_V2:
        if( ssi_channel >= 3 && ssi_channel <= 4 ) {
            return true;
        }
        return false;
    }
}

bool rcar_ssi_tdmext_supported( uint32_t ssi_channel ) {
    if( !rcar_ssi_supported( ssi_channel ) ) {
        return false;
    }
    switch( ssi_channel ) {
    case 0: case 1: case 2:
    case 3: case 4: case 9:
        return true;
    case 5: case 6: case 7:
    case 8: default:
        return false;
    }
}

bool rcar_ssi_tdmsplit_supported( uint32_t ssi_channel ) {
#ifdef RCAR_TDMSPLIT_SUPPORT
    if( !rcar_ssi_supported( ssi_channel ) ) {
        return false;
    }
    switch( ssi_channel ) {
    case 0: case 1: case 2: case 9:
        return true;
    case 3: case 4: case 5: case 6:
    case 7: case 8: default:
        return false;
    }
#else
    return false;
#endif
}

int rcar_ssi_get_supported_range( uint32_t* min_ssi_channel, uint32_t* max_ssi_channel) {
    if( !min_ssi_channel || !max_ssi_channel ) {
        return EINVAL;
    }
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_E2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        *min_ssi_channel = 0;
        *max_ssi_channel = 9;
        break;
    case RCAR_VERSION_V2:
        *min_ssi_channel = 3;
        *max_ssi_channel = 4;
        break;
    }
    return EOK;
}

bool rcar_ssi_subchan_supported( uint32_t ssi_channel, uint32_t subchannel ) {
    if( !rcar_ssi_supported( ssi_channel ) ) {
        return false;
    }
    switch( ssi_channel ) {
        case 0: case 1: case 2: case 9:
            if( subchannel < 4 ) {
                return true;
            }
            return false;
        default:
            if( subchannel == 0 ) {
                return true;
            }
            return false;
    }
}

int rcar_ssi_subchan_get_supported_range( uint32_t ssi_channel, uint32_t* min_subchannel, uint32_t* max_subchannel ) {
    if( !rcar_ssi_supported( ssi_channel ) ) {
        return EINVAL;
    }
    switch( ssi_channel ) {
        case 0: case 1: case 2: case 9:
            *min_subchannel = 0;
            *max_subchannel = 3;
            break;
        default:
            *min_subchannel = 0;
            *max_subchannel = 0;
            break;
    }
    return EOK;
}

bool rcar_src_supported( uint32_t src_channel ) {
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        if( src_channel >= 0 && src_channel <= 9 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_E2:
        if( src_channel >= 1 && src_channel <= 6 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_V2:
        return false; /* no SRC support */
    }
}

bool rcar_src_multichan_supported( uint32_t src_channel ) {
    if( !rcar_src_supported( src_channel ) ) {
        return false;
    }
    switch( src_channel ) {
    case 0: case 1: case 3: case 4:
        return true;
    case 2: case 5: case 6: case 7:
    case 8: case 9: default:
        return false;
    }
}

bool rcar_src_highsound_supported( uint32_t src_channel ) {
    if( !rcar_src_supported( src_channel ) ) {
        return false;
    }
    switch( src_channel) {
    case 0: case 1: case 2:
    case 3: case 4: case 9:
        return true;
    case 5: case 6: case 7:
    case 8: default:
        return false;
    }
}

bool rcar_src_inline_supported( uint32_t src_channel ) {
    if( !rcar_src_supported( src_channel ) ) {
        return false;
    }
    switch( src_channel ) {
    case 0: case 1: case 2:
    case 3: case 4: case 5:
        return true;
    case 6: case 7: case 8:
    case 9: default:
        return false;
    }
}

int rcar_src_get_supported_range( uint32_t* min_src_channel, uint32_t* max_src_channel) {
    if( !min_src_channel || !max_src_channel ) {
        return EINVAL;
    }
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        *min_src_channel = 0;
        *max_src_channel = 9;
        break;
    case RCAR_VERSION_E2:
        *min_src_channel = 1;
        *max_src_channel = 6;
        break;
    case RCAR_VERSION_V2:
        return ENOTSUP;
    }
    return EOK;
}

bool rcar_cmd_supported( uint32_t cmd_channel ) {
    if( cmd_channel >= 0 && cmd_channel <= 1 && rcar_version != RCAR_VERSION_V2 ) {
        return true;
    }
    return false;
}

int rcar_cmd_get_supported_range( uint32_t* min_cmd_channel, uint32_t* max_cmd_channel ) {
    if( !min_cmd_channel || !max_cmd_channel ) {
        return EINVAL;
    }
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_E2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        *min_cmd_channel = 0;
        *max_cmd_channel = 1;
        break;
    case RCAR_VERSION_V2:
        return ENOTSUP;
    }
    return EOK;
}

bool rcar_mlm_supported( uint32_t mlm_channel ) {
#ifdef RCAR_MLP_SUPPORT
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        if( mlm_channel >= 0 && mlm_channel <= 7 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_E2:
        if( mlm_channel >= 0 && mlm_channel <= 3 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_V2:
        return false; /* no MLM support */
    }
#endif
    return false;
}

int rcar_mlm_get_supported_range( uint32_t* min_mlm_channel, uint32_t* max_mlm_channel) {
#ifdef RCAR_MLP_SUPPORT
    if( !min_mlm_channel || !max_mlm_channel ) {
        return EINVAL;
    }
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        *min_mlm_channel = 0;
        *max_mlm_channel = 7;
        break;
    case RCAR_VERSION_E2:
        *min_mlm_channel = 0;
        *max_mlm_channel = 3;
        break;
    case RCAR_VERSION_V2:
        return ENOTSUP;
    }
    return EOK;
#else
    return ENOTSUP;
#endif
}


bool rcar_dtcpp_supported( uint32_t dtcpp ) {
#if defined(RCAR_MLP_SUPPORT) && defined(RCAR_DTCP_SUPPORT)
    if( dtcpp >= 0 && dtcpp <= 1 && rcar_version != RCAR_VERSION_V2 ) {
        return true;
    }
#endif
    return false;
}

bool rcar_dtcpc_supported( uint32_t dtcpc ) {
#if defined(RCAR_MLP_SUPPORT) && defined(RCAR_DTCP_SUPPORT)
    if( dtcpc >= 0 && dtcpc <= 1 && rcar_version != RCAR_VERSION_V2 ) {
        return true;
    }
#endif
    return false;
}

int rcar_dtcp_get_supported_range( uint32_t* min_dtcp, uint32_t* max_dtcp) {
#if defined(RCAR_MLP_SUPPORT) && defined(RCAR_DTCP_SUPPORT)
    if( !min_dtcp || !max_dtcp ) {
        return EINVAL;
    }
    switch( rcar_version ) {
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    case RCAR_VERSION_E2:
    case RCAR_VERSION_H3:
	case RCAR_VERSION_M3:
    default:
        *min_dtcp = 0;
        *max_dtcp = 1;
        break;
    case RCAR_VERSION_V2:
        return ENOTSUP;
    }
    return EOK;
#else
    return ENOTSUP;
#endif
}

bool rcar_dmac_channel_supported( uint32_t dma_channel ) {
    switch( rcar_version ) {
	case RCAR_VERSION_M3:
        if( dma_channel >= 0 && dma_channel <= 31 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_H3:
        if( dma_channel >= 0 && dma_channel <= 31 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    default:
        if( dma_channel >= 0 && dma_channel <= 25 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_E2:
        if( dma_channel >= 0 && dma_channel <= 12 ) {
            return true;
        }
        return false;
    case RCAR_VERSION_V2:
        if( dma_channel >= 0 && dma_channel <= 1 ) {
            return true;
        }
        return false;
    }
}

int rcar_dmac_get_supported_range( uint32_t* min_dma_channel, uint32_t* max_dma_channel) {
    if( !min_dma_channel || !max_dma_channel ) {
        return EINVAL;
    }
    switch( rcar_version ) {
	case RCAR_VERSION_M3:
        *min_dma_channel = 0;
        *max_dma_channel = 31;
        break;
    case RCAR_VERSION_H3:
        *min_dma_channel = 0;
        *max_dma_channel = 31;
        break;
    case RCAR_VERSION_H2:
    case RCAR_VERSION_M2:
    default:
        *min_dma_channel = 0;
        *max_dma_channel = 25;
        break;
    case RCAR_VERSION_E2:
        *min_dma_channel = 0;
        *max_dma_channel = 12;
        break;
    case RCAR_VERSION_V2:
        *min_dma_channel = 0;
        *max_dma_channel = 1;
        break;
    }
    return EOK;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/deva/ctrl/rcar/rcar_support.c $ $Rev: 812827 $")
#endif

