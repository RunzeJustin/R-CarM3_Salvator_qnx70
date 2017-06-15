/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems.
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

// Module Description:

#include <stdio.h>
#include <errno.h>
#include <atomic.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/trace.h>
#include <sys/slogcodes.h>
#include <sys/mman.h>

#include <internal.h>

#include <xpt.h>

extern sdio_vendor_t	sdio_vendors[];
sdio_ctrl_t				sdio_ctrl;

// tuning block pattern for 4 bit mode
const uint8_t	sdio_tbp_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

// tuning block pattern for 8 bit mode
const uint8_t	sdio_tbp_8bit[] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

void sdio_trace_event( int event, const char *fmt, ... )
{
	char		buf[255];
	va_list		arglist;

	va_start( arglist, fmt );
	vsnprintf( buf, 254, fmt, arglist );
	va_end( arglist );
	TraceEvent( _NTO_TRACE_INSERTUSRSTREVENT, event, buf );
}

ssize_t sdio_slogf( int opcode, int severity, int verbosity, int vlevel, const char *fmt, ... )
{
	ssize_t		ret;
	va_list		arglist;

	ret = 0;

	if( verbosity > 5 ) {
		va_start( arglist, fmt );
		vfprintf( stderr, fmt, arglist );
		va_end( arglist );
		fprintf( stderr, "\n" );
	}

	if( verbosity >= vlevel ) {
		va_start( arglist, fmt );
		ret = vslogf( opcode, severity, fmt, arglist );
		va_end( arglist );
	}

	return( ret );
}


//	An invalid sub option is a fatal error, so might be a good idea to tell
//	the user what the problem was.
static void report_invalid_suboption( const char* function, const char* suboption )
{
	char text[40];

	int i;

	for( i = 0; i < (sizeof text) - 1; i++ ) {
		char ch = suboption[i];
		if( !ch ||ch == ',' )
			break;

		text[i] = ch;
	}
	text[i] = '\0';

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 6, 1, "%s: Invalid sdio option: %s", function, text );
}


void sdio_dump( char *buffer, int cnt )
{
	char	*ptr, output[80];
	int		res, i, j, k, n;
	long	l;
	char	ref[80];

	l = 0L;
	res = cnt;
	ptr = buffer;
	while( res ) {
#define BL 16
#define AL BL/2*5+4
		snprintf( output, 80, "%08lx:  %-*.*s\"%-*.*s\"", l, AL, AL, "  ", BL, BL, "  " );
		k = strlen( output ) - (BL + 1);
		j = l % BL;
		i = 12 + (j << 1) + (j >> 1) + (j >= (BL / 2) ? 2 : 0);
		for( ; j < BL && res; j++, res--, ptr++, l++ ) {
			n = ((int) *ptr >> 4) & 0xf;
			output [i++] = (n < 10 ? (char) n + '0': (char) n + ('a' - 10));
			n = (int) *ptr & 0xf;
			output [i++] = (n < 10 ? (char) n + '0': (char) n + ('a' - 10));
			if (j & 1)
				i++;
			if (j == BL / 2 - 1)
				i += 2;
#define UC unsigned char
			output [j + k] = ((UC) *ptr < ' ' || (UC) *ptr > '~') ? '.' : *ptr;
#undef UC
		}
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 0, 0, "%s", output );
		memcpy( ref, output, 80 );
#undef BL
#undef AL
	}
}

// return position of MSB
int fls( int val )
{
	int		idx;

	idx = 32;
	while( val ) {
		if( val & 0x80000000u ) return( idx );
		val <<= 1;
		idx--;
	}
	return( 0 );
}

void *sdio_alloc( size_t size )
{
	void *addr = xpt_alloc( XPT_ALLOC_CONTIG | XPT_ALLOC_NOCACHE, size, NULL );

	if( addr == MAP_FAILED )
		addr = NULL;

	return addr;
}

int sdio_free( void *vaddr, size_t size )
{
	return( xpt_free( vaddr, size ) );
}

sdio_hc_t *sdio_hc_alloc( )
{
	sdio_hc_t		*hc;
	sdio_hc_cfg_t	*cfg;

	if( ( hc = calloc( 1, sizeof( sdio_hc_t ) ) ) ) {
		hc->path		= sdio_ctrl.nhc++;
		hc->priority	= sdio_ctrl.priority;
		cfg				= &hc->cfg;
		cfg->idx		= -1;
		cfg->caps		= ~0;
		cfg->verbosity	= sdio_ctrl.verbosity;
		cfg->idle_time	= SDIO_PM_IDLE_TIME;
		cfg->sleep_time	= SDIO_PM_SLEEP_TIME;
		hc->hc_coid		= hc->hc_chid = hc->hc_tid = hc->tuning_timerid = -1;
		TAILQ_INSERT_TAIL( &sdio_ctrl.hlist, hc, hlink );
	}

	return( hc );
}

int sdio_hc_free( sdio_hc_t *hc )
{
//	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: hc %p", __FUNCTION__, hc );

	TAILQ_REMOVE( &sdio_ctrl.hlist, hc, hlink );
	sdio_ctrl.nhc--;
	free( hc );

	return( EOK );
}

int sdio_vtop_sg( sdio_sge_t *vsg, sdio_sge_t *psg, int sgc, void *mhdl )
{
	return( xpt_vtop_sg( (SG_ELEM *)vsg, (SG_ELEM *)psg, sgc, mhdl ) );
}

paddr64_t sdio_vtop( void *vaddr )
{
	return( xpt_vtop( vaddr, NULL ) );
}

char *sdio_module_args( const char *module, int argc, char *argv[], int occurrence )
{
	char	*cp;
	int		n;

	if( argc < 2 ) {		// need at least 2 args
		return( NULL );
	}

	for( n = 0; n < argc - 1; ++n ) {
		if( !strcmp( argv[n], module ) ) {
			if( !--occurrence ) {
				return( ( cp = strdup( argv[n + 1] ) ) ? cp : NULL );
			}
		}
	}
	return( NULL );
}

int sdio_parse_number( const char *str )
{
	char	*cp;
	int		number;

	if( str == NULL || *str == '\0' ) {
		return( SDIO_INVALID_NUM );
	}

	number = strtol( str, &cp, 0 );
	if( ( !number && cp == str ) || number < 0 || *cp != '\0' ) {
		return( SDIO_INVALID_NUM );
	}
	return( number );
}

char *sdio_strchr( char *string, char token )
{
	char	*cp;
	int		escaped;

	escaped = 0;
	for( cp = string; *cp != '\0'; ++cp ) {
		if( escaped ) {
			escaped = 0, --cp;
			memmove( cp, cp + 1, strlen( cp ) );
		}
		else if( *cp == '\\' ) {
			escaped = !0;
		}
		else if( *cp == token ) {
			return( cp );
		}
	}
	return( NULL );
}

int sdio_parse_tuple( char *string, char separator, char **entry, ... )
{
	va_list	args;
	char	*cp;
	int		nargs;

	nargs = 0;
	va_start( args, entry );
	while( string != NULL || entry != NULL ) {
		cp = (string != NULL) ? sdio_strchr( string, separator ) : NULL;
		if( entry != NULL ) {
			*entry = string, ++nargs, entry = va_arg( args, char ** );
		}
		if( cp != NULL ) {
			*cp++ = '\0';
		}
		string = cp;
	}
	va_end( args );
	return( nargs );
}

int sdio_set_thread_state( uint32_t *tstate, int state )
{
	pthread_sleepon_lock( );
	*tstate = state;
	pthread_sleepon_signal( tstate );
	pthread_sleepon_unlock( );
	return( EOK );
}

int sdio_create_thread( pthread_t *tid, pthread_attr_t *aattr, void *(*func)(void *), void *arg, int priority, uint32_t *tstate, char *name )
{
	pthread_t			_tid;
	pthread_attr_t		*pattr;
	pthread_attr_t		attr;
	struct sched_param	param;
	int					status;

	if( tid == NULL ) {
		tid = &_tid;
	}

	if( tstate ) {
		*tstate = SDIO_TSTATE_CREATING;
	}

	pattr = aattr;
	if( aattr == NULL ) {
		pattr = &attr;
		pthread_attr_init( &attr );
		pthread_attr_setschedpolicy( &attr, SCHED_RR );
		param.sched_priority = priority;
		pthread_attr_setschedparam( &attr, &param );
		pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
		pthread_attr_setstacksize( &attr, SDIO_STACK_SIZE );
	}

	status = pthread_create( tid, pattr, func, arg );
	if( aattr == NULL ) {
		pthread_attr_destroy( &attr );
	}
	if( status ) {
		return( status );
	}

	if( name ) {
#if ( _NTO_VERSION >= 632 )
		if( pthread_setname_np( *tid, name ) ) {
//			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 0, 0, "%s:  pthread_setname_np %s", __FUNCTION__, strerror( status ) );
		}
#endif
	}

	if( tstate ) {
		pthread_sleepon_lock( );
		while( !( *tstate ) ) {
			pthread_sleepon_wait( tstate );
		}
		pthread_sleepon_unlock( );
		status = *tstate == SDIO_TSTATE_INITIALIZED ? EOK : EIO;
	}

	return( status );
}

uint32_t sdio_extract_bits( uint32_t *data, int bits, int start, int size )
{
	int			idx;
	int			shift;
	uint32_t	value;

	idx		= ( bits / 32 ) - ( start / 32 ) - 1;

	shift	= start & 31;
	value	= data[idx] >> shift;
	if( size + shift > 32 ) {
		value |= data[idx - 1] << ( 32 - shift );
	}

	return( value & ( ( 1llu << size ) - 1 ) );
}

int sdio_sg_start( sdio_hc_t *hc, sdio_sge_t *sge, int nsg )
{
	sdio_wspc_t	*wspc;

	wspc		= &hc->wspc;

	if( !nsg && sge == NULL ) {
		return( EINVAL );
	}

	wspc->nsg	= nsg;
	wspc->sge	= sge;
	wspc->sgc	= sge->sg_count;
	wspc->sga	= SDIO_DATA_PTR_V( sge->sg_address );

	return( EOK );
}

// return:  SDIO_TRUE - complete / SDIO_FALSE - more to do
int sdio_sg_nxt( sdio_hc_t *hc, uint8_t **addr, int *len, int blksz )
{
	sdio_wspc_t	*wspc;

	wspc = &hc->wspc;

	if( !wspc->nsg && !wspc->sgc ) {		// sge's complete
		return( SDIO_TRUE );
	}

	if( !wspc->sgc ) {						// advance to next sg entry
		wspc->sge++;
		wspc->nsg--;
		wspc->sgc	= wspc->sge->sg_count;
		wspc->sga	= SDIO_DATA_PTR_V( wspc->sge->sg_address );
	}

	*addr		= wspc->sga;
	*len		= min( wspc->sgc, blksz );
	wspc->sga	+= *len;
	wspc->sgc	-= *len;

	return( SDIO_FALSE );
}

sdio_device_errata_t *sdio_device_errata( sdio_dev_t *dev, sdio_device_errata_t *erratas )
{
	sdio_cid_t				*cid;
	sdio_ecsd_t		*ecsd;
	sdio_device_errata_t	*errata;

	cid	= &dev->cid;
	ecsd	= &dev->ecsd;
	for( errata = erratas; errata->pnm; errata++ ) {
		if( !strncmp( cid->pnm, errata->pnm, strlen( errata->pnm ) ) ) {
			if( ( cid->mid == errata->mid || errata->mid == DEV_ERRATA_WILDCARD ) &&
					( cid->oid == errata->oid || errata->oid == DEV_ERRATA_WILDCARD ) &&
					( ecsd->ext_csd_rev == errata->erev || errata->erev == DEV_ERRATA_WILDCARD ) &&
					cid->prv >= errata->prv_s && cid->prv <= errata->prv_e ) {
				sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s:  pnm %s, mid 0x%x, oid 0x%x, rev 0x%x:0x%x, errata 0x%x, rsettle %dms", __FUNCTION__, errata->pnm, errata->mid, errata->oid, errata->prv_s, errata->prv_e, errata->errata, errata->rsettle );
				return( errata );
			}
		}
	}

	return( NULL );
}

int sdio_reconcile_errata( sdio_dev_t *dev, sdio_device_errata_t *errata )
{
	sdio_hc_t	*hc;

	hc = dev->hc;

	if( ( errata->errata & DEV_ERRATA_ACMD12 ) ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s:  disabling ACMD12", __FUNCTION__ );
		hc->caps &= ~HC_CAP_ACMD12;
	}

	return( EOK );
}

int sdio_pwr( sdio_hc_t *hc, int vdd )
{
	int					bpos;
	static const char	*name[] = { "0.0", "", "", "", "", "", "", "1.7-1.9",
		"2.0", "2.1", "2.2", "2.3", "2.4", "2.5", "2.6",
		"2.7", "2.8", "2.9", "3.0", "3.1", "3.2", "3.3",
		"3.4", "3.5", "3.6", "", "", "", "", "", "" };

	if( ( bpos = ffs( vdd ) ) ) {
		bpos--;
	}

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  vdd %s (0x%x)", __FUNCTION__, name[bpos], vdd );
	return( hc->entry.pwr( hc, vdd ) );
}

int sdio_drv_type( sdio_hc_t *hc, int drv_type )
{
	static const char	*name[5] = { "B", "A", "C", "", "D" };

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  type %s", __FUNCTION__, name[drv_type >> 1] );

	return( hc->entry.drv_type( hc, drv_type ) );
}

int sdio_bus_mode( sdio_hc_t *hc, int bus_mode )
{
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  mode %s", __FUNCTION__, bus_mode ? "push pull" : "open drain" );

	return( hc->entry.bus_mode( hc, bus_mode ) );
}

int sdio_bus_width( sdio_hc_t *hc, int bus_width )
{
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  width %d", __FUNCTION__, bus_width );

	return( hc->entry.bus_width( hc, bus_width ) );
}

int sdio_clock( sdio_hc_t *hc, int clk )
{
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  clk %d", __FUNCTION__, clk );

	if( clk < hc->clk_min ) {
		return( EINVAL );
	}

	if( clk > hc->clk_max ) {
		clk = hc->clk_max;
	}

	return( hc->entry.clk( hc, clk ) );
}

int sdio_timing( sdio_hc_t *hc, int timing )
{
	static const char	*name[10] = { "INVALID", "LS", "HS", "HS DDR50", "SDR12", "SDR25", "SDR50", "SDR104", "HS200", "HS400" };

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  timing %s", __FUNCTION__, name[timing] );

	return( hc->entry.timing( hc, timing ) );
}

int sdio_signal_voltage( sdio_hc_t *hc, int voltage )
{
	static const char	*name[5] = { "0.0", "3.3", "3.0", "1.8", "1.2" };

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  %sV", __FUNCTION__, name[voltage] );

	return( hc->entry.signal_voltage( hc, voltage ) );
}

int _sdio_pwrmgnt( sdio_dev_t *dev, int pm )
{
	sdio_hc_t	*hc;
	int			status;

	hc		= dev->hc;
	status	= EOK;

	if( hc->entry.pm == NULL || pm == hc->pm_state || hc->wspc.cmd ) {
		return( EOK );
	}

	switch( pm ) {
		case PM_IDLE:
//			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s: idle", __FUNCTION__ );

			if( hc->pm_state == PM_ACTIVE ) {
				hc->entry.pm( hc, pm );
			}
			else {
				return( EOK );
			}
			break;

		case PM_ACTIVE:
//			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s (path %d): active", __FUNCTION__, hc->path );

			hc->entry.pm( hc, pm );

			if( hc->pm_state == PM_SLEEP && ( hc->caps & HC_CAP_SLEEP ) ) {
				sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s (path %d): sleep to active", __FUNCTION__, hc->path );
				hc->pm_state = pm;
				if( ( dev->caps & DEV_CAP_SLEEP ) ) {
					if( ( status = mmc_sleep_awake( dev, 0 ) ) == EOK ) {
						status = sdio_select_card( dev, dev->rca );
					}
					if( status ) {
						sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: awake fail", __FUNCTION__ );
						status = _sdio_reset( dev );
					}
				}
				else {
					if( ( dev->flags & DEV_FLAG_PRESENT ) ) {
						status = _sdio_reset( dev );
					}
				}
			}
			break;

		case PM_SLEEP:
// 			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s: sleep", __FUNCTION__ );

			if( hc->pm_state == PM_IDLE ) {		// wakeup when idle to deselect
				hc->entry.pm( hc, PM_ACTIVE );
			}

			if( ( hc->caps & HC_CAP_SLEEP ) ) {
				if( ( dev->caps & DEV_CAP_SLEEP ) ) {
					if( ( dev->flags & DEV_FLAG_WCE ) ) {
						if( ( status = mmc_cache( dev, SDIO_CACHE_FLUSH, SDIO_TIME_DEFAULT * 5 ) ) ) {
							sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: cache flush fail %s", __FUNCTION__, strerror( status ) );
						}
					}

					if( status == EOK ) {
						if( ( status = sdio_select_card( dev, 0 ) ) == EOK ) {
// Should we wait for STANDBY state before putting the device to sleep?
//							if( ( status = _sdio_wait_card_status( dev, NULL, CDS_CUR_STATE_MSK, CDS_CUR_STATE_STANDBY, SDIO_TIME_DEFAULT ) ) == EOK ) {
								status = mmc_sleep_awake( dev, 1 );
//							}
						}

						if( status ) {
							_sdio_reset( dev );
						}
					}
				}
				if( status ) {
					pm = PM_IDLE;							// set state back to idle
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: sleep fail", __FUNCTION__ );
				}
				else {
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s (path %d): sleep hc cnt %" PRId64 "", __FUNCTION__, hc->path, ++hc->pm_sleep_cnt );
				}
				hc->entry.pm( hc, pm );
			}
			break;

		default:
			break;
	}

	hc->pm_state = pm;

	return( status );
}

int sdio_select_voltage( sdio_hc_t *hc, int ocr )
{
	sdio_dev_t *dev;
	int			bpos;
	int			status;

	ocr	&= hc->ocr;
	dev = &hc->device;

	if( ( bpos = ffs( ocr ) ) ) {
		ocr = 1 << ( bpos - 1 );
		status = sdio_pwr( hc, ocr );
	}
	else {
		ocr		= 0;
		status	= EINVAL;
	}

	dev->ocr = ocr;

	return( status );
}

int sdio_tune( sdio_hc_t *hc, int cmd )
{
	int	status;

	status = EOK;

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:", __FUNCTION__ );

	if( hc->entry.tune ) {
		status = hc->entry.tune( hc, cmd );
	}

	return( status );
}

int sdio_preset( sdio_hc_t *hc, int state )
{
	int	status;

	status = EOK;

	if( hc->entry.preset ) {
		status = hc->entry.preset( hc, state );
	}

	return( status );
}

int sdio_power( sdio_hc_t *hc, int pwr )
{
	switch( pwr ) {
		case SDIO_PWR_ON:
			sdio_pwr( hc, 1 << ( fls( hc->ocr ) - 1 ) );
			sdio_clock( hc, hc->clk_init );
			sdio_bus_width( hc, BUS_WIDTH_1 );
			sdio_preset( hc, SDIO_FALSE );
			sdio_timing( hc, TIMING_LS );
			sdio_bus_mode( hc, BUS_MODE_OPEN_DRAIN );
			sdio_signal_voltage( hc, SIGNAL_VOLTAGE_3_3 );
			delay( 10 );		// pwr up delay
			break;

		case SDIO_PWR_OFF:
		default:
			sdio_pwr( hc, 0 );
			break;
	}

	return( EOK );
}

int _sdio_send_status( sdio_dev_t *dev, uint32_t *rsp, int hpi )
{
	struct sdio_cmd		*cmd;
	uint32_t			arg;
	int					status;

	memset( rsp, 0, SDIO_RSP_SIZE );

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	arg = ( dev->rca << 16 ) | ( hpi ? MMC_SEND_STATUS_HPI: 0 );

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_SEND_STATUS, arg );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
		memcpy( rsp, cmd->rsp, sizeof( cmd->rsp ) );
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sdio_send_csd( sdio_dev_t *dev, uint32_t *csd )
{
	struct sdio_cmd		*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R2, MMC_SEND_CSD, dev->rca << 16 );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
		csd[0] = cmd->rsp[0]; csd[1] = cmd->rsp[1];
		csd[2] = cmd->rsp[2]; csd[3] = cmd->rsp[3];
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sdio_select_card( sdio_dev_t *dev, int rca )
{
	struct sdio_cmd		*cmd;
	int					status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | ( rca ? SCF_RSP_R1: SCF_RSP_NONE ),
							MMC_SEL_DES_CARD, rca << 16 );

	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {

	}

	if( ( cmd->rsp[0] & CDS_CARD_IS_LOCKED ) ) {
		dev->flags |= DEV_FLAG_LOCKED;
	}

	sdio_free_cmd( cmd );

	return( status );
}

int _sdio_stop_transmission( sdio_dev_t *dev, int hpi )
{
	struct sdio_cmd		*cmd;
	uint32_t			arg;
	int					status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	arg = hpi ? ( ( dev->rca << 16 ) | MMC_STOP_TRANSMISSION_HPI ): 0;

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1B, MMC_STOP_TRANSMISSION, arg );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 /* SDIO_CMD_RETRIES */ ) ) == EOK ) {

	}

	sdio_free_cmd( cmd );

	return( status );
}

int _sdio_set_block_length( sdio_dev_t *dev, int blklen )
{
	struct sdio_cmd		*cmd;
	int					status;

		// cmd isn't supported in DDR/HS400/HC
	if( ( dev->flags & ( DEV_FLAG_DDR | DEV_FLAG_HS400 ) ) && ( dev->caps & DEV_CAP_HC ) ) {
		return( EOK );
	}

	if( dev->block_length == blklen ) {
		return( EOK );
	}

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_SET_BLOCKLEN, blklen );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
		dev->block_length = blklen;
	}

	sdio_free_cmd( cmd );

	return( status );
}

int _sdio_set_block_count( sdio_dev_t *dev, int blkcnt )
{
	struct sdio_cmd		*cmd;
	int					status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	sdio_setup_cmd( cmd, SCF_CTYPE_AC | SCF_RSP_R1, MMC_SET_BLOCK_COUNT, blkcnt );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) == EOK ) {

	}

	sdio_free_cmd( cmd );

	return( status );
}

void sdio_rsp( sdio_dev_t *dev, uint32_t *rsp )
{
	sdio_hc_t			*hc;
	static const char	*state[16] = {	"IDLE", "READY", "IDENT", "STANDBY",
										"TRAN", "DATA", "RCV", "PRG", "DIS", "BTST", "SLP", "RSVD", "RSVD", "RSVD", "RSVD", "RSVD" };

	hc		= dev->hc;

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "rsp 0x%x state %s %s", rsp[0], state[ CDS_CUR_STATE( rsp[0] ) ], ( rsp[0] & CDS_READY_FOR_DATA ) ? "Ready for DATA" : "" );

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		(rsp[0] & CDS_OUT_OF_RANGE			) ? "OOR " : "",
		(rsp[0] & CDS_ADDRESS_ERROR			) ? "ADDR" : "",
		(rsp[0] & CDS_BLOCK_LEN_ERROR		) ? "BLEN " : "",
		(rsp[0] & CDS_ERASE_SEQ_ERROR		) ? "ESEQ " : "",
		(rsp[0] & CDS_ERASE_PARAM			) ? "EPARM " : "",
		(rsp[0] & CDS_WP_VIOLATION			) ? "WPV " : "",
		(rsp[0] & CDS_CARD_IS_LOCKED		) ? "CIL " : "",
		(rsp[0] & CDS_LOCK_UNLOCK_FAILED	) ? "LUF " : "",
		(rsp[0] & CDS_COM_CRC_ERROR			) ? "CRC " : "",
		(rsp[0] & CDS_ILLEGAL_COMMAND		) ? "ILC " : "",
		(rsp[0] & CDS_CARD_ECC_FAILED		) ? "ECC " : "",
		(rsp[0] & CDS_CC_ERROR				) ? "CC " : "",
		(rsp[0] & CDS_ERROR					) ? "ERR " : "",
		(rsp[0] & CDS_UNDERRUN				) ? "UR " : "",
		(rsp[0] & CDS_OVERRUN				) ? "OR " : "",
		(rsp[0] & CDS_CID_CSD_OVERWRITE		) ? "CID " : "",
		(rsp[0] & CDS_WP_ERASE_SKIP			) ? "WPES " : "",
		(rsp[0] & CDS_CARD_ECC_DISABLED		) ? "CECCD " : "",
		(rsp[0] & CDS_ERASE_RESET			) ? "ER " : "",
		(rsp[0] & CDS_SWITCH_ERROR			) ? "SE " : "",
		(rsp[0] & CDS_URGENT_BKOPS			) ? "BKOPS " : "",
		(rsp[0] & CDS_APP_CMD_S				) ? "APP " : "" );
}

int _sdio_wait_card_status( sdio_dev_t *dev, uint32_t *rsp, uint32_t mask, uint32_t val, uint32_t msec )
{
	sdio_hc_t	*hc;
	int			status;
	uint32_t	resp[4];

	hc		= dev->hc;
	status	= EOK;
	rsp		= rsp ? rsp : resp;

	for( msec = max( msec, 1 ); msec; msec-- ) {
		if( ( status = _sdio_send_status( dev, rsp, SDIO_FALSE ) ) != EOK ) {
			break;
		}

		if( hc->cfg.verbosity > 3 ) {
			sdio_rsp( dev, rsp );
		}

		if( ( rsp[0] & CDS_ERROR_MSK ) ) {
			status = EIO; break;
		}

		if( ( rsp[0] & mask ) == val ) {
			status = EOK; break;
		}

		delay( 1 );
	}

	if( !msec ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  mask %x, val %x, card status %x", __FUNCTION__, mask, val, rsp[0] );
		sdio_rsp( dev, rsp );
		status = ETIMEDOUT;
	}

	return( status );
}

uint64_t sdio_erase_timeout( sdio_dev_t *dev, uint32_t etype, uint32_t nlba )
{
	uint64_t		timeout;

	if( dev->dtype == DEV_TYPE_MMC ) {
		timeout = mmc_erase_timeout( dev, etype, nlba );
	}
	else {
		timeout = sd_erase_timeout( dev, etype, nlba );
	}

	return( max( timeout, 1000 ) );
}

int _sdio_lock_unlock( sdio_dev_t *dev, int op, uint8_t *pwd, int pwd_len )
{
	struct sdio_cmd		*cmd;
	sdio_sge_t			sge;
	uint8_t				*ld;
	int					len;
	int					status;
	int					timeout;
	uint32_t			rsp[4];

	len = pwd_len + 2;	// +2 for op field, and pwd_len field

	if( len & 0x1 ) len++;

	timeout = ( op & SD_LU_ERASE ) ? sdio_erase_timeout( dev, SD_ERASE_NORM, dev->csd.sectors ): SDIO_TIME_DEFAULT;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	if( ( ld = sdio_alloc( len ) ) == NULL ) {
		sdio_free_cmd( cmd );
		return( ENOMEM );
	}

	ld[0] = op;
	ld[1] = pwd_len;
	if( pwd_len ) {
		memcpy( &ld[2], pwd, pwd_len );
	}

	if( ( status = _sdio_set_block_length( dev, len ) ) == EOK ) {
		sdio_setup_cmd( cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, MMC_LOCK_UNLOCK, 0 );
		sge.sg_count = len; sge.sg_address = SDIO_DATA_PTR_P( ld );
		sdio_setup_cmd_io( cmd, SCF_DIR_OUT, 1, len, &sge, 1, NULL );

		if( ( status = _sdio_send_cmd( dev, cmd, NULL, timeout, SDIO_CMD_RETRIES ) ) == EOK ) {
		}

		if( ( status = _sdio_set_block_length( dev, SDIO_DFLT_BLKSZ ) ) != EOK ) {
		}
	}

	if( status == EOK && ( status = _sdio_send_status( dev, rsp, SDIO_FALSE ) ) == EOK ) {
		if( ( rsp[0] & CDS_CARD_IS_LOCKED ) ) {
			atomic_set( &dev->flags, DEV_FLAG_LOCKED );
		}

		if( ( dev->flags & DEV_FLAG_LOCKED ) && !( rsp[0] & CDS_CARD_IS_LOCKED ) ) {
			atomic_clr( &dev->flags, DEV_FLAG_LOCKED );
		}
	}

	sdio_free( ld, len );
	sdio_free_cmd( cmd );

	return( status );
}

int sdio_go_idle( sdio_hc_t *hc )
{
	sdio_dev_t			*dev;
	struct sdio_cmd		*cmd;
	int					status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	dev	= &hc->device;

	sdio_setup_cmd( cmd, SCF_CTYPE_BC | SCF_RSP_NONE, MMC_GO_IDLE_STATE, 0 );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, 0 ) ) != EOK ) {
	}

	sdio_free_cmd( cmd );

	return( status );
}

int sdio_all_send_cid( sdio_hc_t *hc, uint32_t *cid )
{
	sdio_dev_t		*dev;
	struct sdio_cmd	*cmd;
	int				status;

	if( ( cmd = sdio_alloc_cmd( ) ) == NULL ) {
		return( ENOMEM );
	}

	dev	= &hc->device;

	sdio_setup_cmd( cmd, SCF_CTYPE_BCR | SCF_RSP_R2, MMC_ALL_SEND_CID, 0 );
	if( ( status = _sdio_send_cmd( dev, cmd, NULL, SDIO_TIME_DEFAULT, SDIO_CMD_RETRIES ) ) == EOK ) {
		cid[0] = cmd->rsp[0]; cid[1] = cmd->rsp[1];
		cid[2] = cmd->rsp[2]; cid[3] = cmd->rsp[3];
	}

	sdio_free_cmd( cmd );

	return( status );
}

int _sdio_reset( sdio_dev_t *dev )
{
	sdio_hc_t	*hc;
	int			retry;
	int			status;

	hc		= dev->hc;
	status	= EOK;
	retry	= SDIO_RESET_RETRIES;

	if( ( hc->flags & HC_FLAG_RST ) ) {
		return( EBUSY );
	}

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s (path %d):  ", __FUNCTION__, hc->path );

	hc->flags		|= HC_FLAG_RST;
	hc->flags		&= ~HC_FLAG_SKIP_PWRUP;
	dev->rca		= 0;
	dev->pactive		= 0;
	dev->flags		&= ~DEV_FLAG_WCE;

	do {
		sdio_power( hc, SDIO_PWR_OFF );
		sdio_power( hc, SDIO_PWR_ON );

		switch( dev->dtype ) {
			case DEV_TYPE_MMC:
				status = mmc_init_device( hc, dev->ocr, SDIO_TRUE ); break;

			case DEV_TYPE_SD:
				status = sd_init_device( hc, dev->ocr, SDIO_TRUE ); break;

			default:
				status = ENXIO; break;
		}

		if( status == ENXIO ) {					// device has been removed
			sdio_hc_event( hc, HC_EV_CD );
			break;
		}

	} while( --retry && status != EOK );

	hc->flags &= ~HC_FLAG_RST;

	return( status );
}


int sdio_retune( sdio_hc_t *hc )
{
	struct itimerspec	value;

	if( hc->tuning_count ) {
		memset( &value, 0, sizeof( value ) );
		value.it_value.tv_sec	= hc->tuning_count;
		if( timer_settime( hc->tuning_timerid, 0, &value, NULL ) == -1 ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: timer_settime", __FUNCTION__ );
		}
	}

	atomic_clr( &hc->flags, HC_FLAG_TUNE );

	sdio_tune( hc, hc->device.dtype == DEV_TYPE_MMC ? MMC_SEND_TUNING_BLOCK : SD_SEND_TUNING_BLOCK );

	return( EOK );
}

int sdio_wait_cmd( sdio_hc_t *hc, struct sdio_cmd *cmd, uint64_t tms )
{
	uint64_t		ct;
	int				status;
	struct timespec	abstime;

	status = EOK;

	ct = _syspage_time( CLOCK_MONOTONIC );
	nsec2timespec( &abstime, ct + SDIO_TIMEOUT_MS_TO_NS( tms ) );

	pthread_mutex_lock( &hc->mutex );
	while( cmd->status == CS_CMD_INPROG ) {
		if( ( status = pthread_cond_timedwait( &hc->cond, &hc->mutex, &abstime ) ) == ETIMEDOUT ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: TIMEOUT %"PRId64"ms (errno %d) CMD %d, flgs 0x%x, arg 0x%x, blks %d, blksz %d",
				__FUNCTION__, tms, status, cmd->opcode, cmd->flags, cmd->arg, cmd->blks, cmd->blksz );
			break;
		}
	}
	pthread_mutex_unlock( &hc->mutex );

	return( status );
}

int sdio_issue_cmd( sdio_dev_t *dev, struct sdio_cmd *cmd, uint64_t tms )
{
	sdio_hc_t		*hc;
	int				status;

	hc				= dev->hc;

#ifdef SDIO_TRACE
	sdio_trace_event( SDIO_TRACE_EVENT, "CMD %d, flgs 0x%x, arg 0x%x, blks %d, blksz %d, timeout %llums", cmd->opcode, cmd->flags, cmd->arg, cmd->blks, cmd->blksz, tms );
#endif

	if( hc->cfg.verbosity > 3 ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 3, "%s: CMD %d, flgs 0x%x, arg 0x%x, blks %d, blksz %d, timeout %" PRId64 "ms", __FUNCTION__, cmd->opcode, cmd->flags, cmd->arg, cmd->blks, cmd->blksz, tms );
	}

	if( ( hc->flags & HC_FLAG_TUNE ) ) {
		sdio_retune( hc );
	}

	pthread_mutex_lock( &hc->mutex );
	hc->wspc.cmd	= cmd;
	pthread_mutex_unlock( &hc->mutex );

	if( ( status = hc->entry.cmd( hc, cmd ) ) == EOK ) {
		status = sdio_wait_cmd( hc, cmd, tms );
	}

	if( status ) {
		hc->entry.abort( hc, cmd );
		pthread_mutex_lock( &hc->mutex );
		hc->wspc.cmd            = NULL;
		cmd->status             = status;
		pthread_mutex_unlock( &hc->mutex );
	}

	return( status );
}

// hc callback for command completion
int sdio_cmd_cmplt( sdio_hc_t *hc, struct sdio_cmd *cmd, int status )
{
	static const char	*name[12] = { 	"IN PROG", "SUCCESS", "ABORTED", "ERR", "CMD IDX ERR",
										"CMD TO ERR", "CMD CRC ERR", "CMD END ERR",
										"DATA TO ERR", "DATA CRC ERR", "DATA END ERR", "CARD REMOVED" };

#ifdef SDIO_TRACE
	sdio_trace_event( SDIO_TRACE_EVENT, "CMD cmplt status %s (%d) ", name[status], status );
#endif

	if( hc->cfg.verbosity > 3 ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 4, "%s: CMD %d, flgs 0x%x, arg 0x%x, blks %d, blksz %d, status %s (%d)", __FUNCTION__, cmd->opcode, cmd->flags, cmd->arg, cmd->blks, cmd->blksz, name[status], status );
	}

	pthread_mutex_lock( &hc->mutex );
	hc->wspc.cmd	= NULL;
	cmd->status		= status;
	pthread_cond_signal( &hc->cond );
	pthread_mutex_unlock( &hc->mutex );

	return( EOK );
}

int _sdio_send_cmd( sdio_dev_t *dev, struct sdio_cmd *cmd,
		void (*func)( struct sdio_device *, sdio_cmd_t *, void *),
		uint32_t timeout, int retries )
{
	sdio_hc_t 	*hc;
	int			status;

	hc = dev->hc;

	if( ( status = _sdio_pwrmgnt( dev, PM_ACTIVE ) ) ) {
		return( status );
	}

	do {
		cmd->status = CS_CMD_INPROG;

		if( ( cmd->flags & SCF_APP_CMD ) ) {
			if( ( status = sd_app_cmd( dev ) ) ) {
				break;
			}
		}

		if( ( cmd->flags & SCF_SBC ) && !( hc->caps & HC_CAP_ACMD23 ) ) {
			if( ( status = _sdio_set_block_count( dev, cmd->blks ) ) ) {
				break;
			}
		}

		if( ( status = sdio_issue_cmd( dev, cmd, timeout ) ) != EOK ) {
			if( ( dev->flags & DEV_FLAG_PRESENT ) ) {
//				_sdio_reset( dev );
			}
			break;
		}

		switch( cmd->status  ) {
			case CS_CMD_CMP:
				status = EOK; break;
			case CS_CARD_REMOVED:
				return( ENXIO );
			default:
				status = EIO; break;
		}

		if( status == EOK ) {
			if( ( cmd->flags & SCF_WAIT_DRDY ) && !( hc->caps & HC_CAP_BSY ) ) {
				if( ( status = _sdio_wait_card_status( dev, NULL, CDS_READY_FOR_DATA | CDS_CUR_STATE_MSK, CDS_READY_FOR_DATA | CDS_CUR_STATE_TRAN, timeout ) ) != EOK ) {
					break;
				}
			}
			break;
		}

		if( ( cmd->rsp[0] & CDS_ILLEGAL_COMMAND ) ) {
			break;
		}

	} while( retries-- );

	return( status );
}

int _sdio_bus_error( sdio_dev_t *dev )
{
	sdio_hc_t	*hc;
	int			status;

	hc		= dev->hc;
	status	= EOK;

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s:  ", __FUNCTION__ );

	if( ++hc->bus_errs > SDIO_MAX_BUS_ERRS ) {
#if 0
// only needed for testing when we simulate bus errors
		_sdio_pwrmgnt( dev, PM_ACTIVE );
#endif

		switch( dev->dtype ) {
			case DEV_TYPE_MMC:
				status = mmc_bus_error( dev ); break;
			case DEV_TYPE_SD:
				status = sd_bus_error( dev ); break;
			default:
				status = ENODEV; break;
		}
		hc->bus_errs = 0;
	}

	return( status );
}

int _sdio_attach( sdio_hc_t *hc )
{
	sdio_dev_t		*dev;
	uint32_t		clk;
	int				dtype;
	int				status;

	memset( &hc->device, 0, sizeof( sdio_dev_t ) );
	dev				= &hc->device;
	dev->hc			= hc;
	hc->bus_errs	= 0;
	status			= ENODEV;
	dtype			= hc->flags & HC_FLAG_DEV_TYPE;

	if( !( hc->flags & HC_FLAG_SKIP_PWRUP ) ) {
		sdio_power( hc, SDIO_PWR_OFF );
	}

	for( clk = SDIO_CLK_INIT; clk; clk -= 100000 ) {
		hc->clk_init	= max( clk, hc->clk_min );

		if( !( hc->flags & HC_FLAG_SKIP_PWRUP ) ) {
			sdio_power( hc, SDIO_PWR_ON );
		}

		if( !dtype || ( dtype & HC_FLAG_DEV_SD ) ) {
			if( ( status = sd_ident( hc ) ) == EOK ) {
				_sdio_pwrmgnt( dev, PM_IDLE );
				dev->flags |= DEV_FLAG_PRESENT;
				dev->dtype = DEV_TYPE_SD;
				return( status );
			}
		}

		if( !dtype || ( dtype & HC_FLAG_DEV_MMC ) ) {
			if( ( status = mmc_ident( hc ) ) == EOK ) {
				_sdio_pwrmgnt( dev, PM_IDLE );
				dev->flags |= DEV_FLAG_PRESENT;
				dev->dtype = DEV_TYPE_MMC;
				return( status );
			}
		}

		hc->flags &= ~HC_FLAG_SKIP_PWRUP;

		sdio_power( hc, SDIO_PWR_OFF );
	}

	return( status );
}

int sdio_cd_info( sdio_hc_t *hc )
{
	sdio_dev_t			*dev;
	sdio_cid_t			*cid;
	sdio_csd_t			*csd;
	sdio_ecsd_t			*ecsd;
	sd_switch_cap_t		*swcaps;
	sdio_hc_cfg_t		*cfg;
	static const char	*name[10] = { "INVALID", "LS", "HS", "HS DDR50", "SDR12", "SDR25", "SDR50", "SDR104", "HS200", "HS400" };

	cfg		= &hc->cfg;
	dev		= &hc->device;
	cid		= &dev->cid;
	csd		= &dev->csd;
	ecsd	= &dev->ecsd;
	swcaps	= &dev->swcaps;

		// CID
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s CID:", dev->dtype == DEV_TYPE_MMC ? "MMC" : "SD" );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  MID 0x%x, OID 0x%x, PNM %s", cid->mid, cid->oid, cid->pnm );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  PRV 0x%x, PSN 0x%x, MDT %d-%d",
				cid->prv, cid->psn, cid->month, cid->year );

		// CSD
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s CSD:", dev->dtype == DEV_TYPE_MMC ? "MMC" : "SD" );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  CSD_STRUCTURE %d, SPEC_VERS %d, CCC 0x%x", csd->csd_structure, csd->spec_vers, csd->ccc );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  TAAC %d, NSAC %d, TRAN_SPEED %d", csd->taac, csd->nsac, csd->tran_speed );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  C_SIZE %d, C_SIZE_MULT %d", csd->c_size, csd->c_size_mult );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  READ_BL_LEN %d, WRITE_BL_LEN %d", csd->read_bl_len, csd->write_bl_len );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  ERASE GRP_SIZE %d, GRP_MULT %d, SIZE %d", csd->erase_grp_size, csd->erase_grp_mult, csd->sector_size );
	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  blksz %d, sectors %d, dtr %d", csd->blksz, csd->sectors, csd->dtr_max );

	if( dev->dtype == DEV_TYPE_SD ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "SD SW CAPS:" );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  bus mode 0x%x, cmd sys 0x%x",
			swcaps->bus_mode, swcaps->cmd_sys );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  drv type 0x%x, curr limit 0x%x",
			swcaps->drv_type, swcaps->curr_limit );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  dtr %d", swcaps->dtr_max_hs );
	}
	else {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "MMC EXT CSD:" );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  DEVICE_TYPE 0x%x, EXT_CSD_REV %d",
			ecsd->card_type, ecsd->ext_csd_rev );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  SEC_COUNT %d, dtr %d", ecsd->sectors, ecsd->dtr_max_hs );
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "  HC_ERASE_GRP_SIZE %d, HC_WP_GRP_SIZE %d",
			ecsd->hc_erase_group_size, ecsd->hc_wp_grp_size );
	}

	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "CFG:  Timing %s, DTR %d, Bus Width %d bit\n", name[hc->timing], hc->clk, hc->bus_width );

	return( EOK );
}


int sdio_event( sdio_hc_t *hc, int ev )
{
	sdio_funcs_t			*funcs;
	struct sdio_device		*device;

	funcs	= &sdio_ctrl.connect_parm.funcs;

	pthread_mutex_lock( &sdio_ctrl.mutex );

		// look up device based on path, generation
	for( device = TAILQ_FIRST( &sdio_ctrl.dlist ); device != NULL; device = TAILQ_NEXT( device, dlink ) ) {
		if( device->instance.path == hc->path && device->instance.generation == hc->generation ) {
			break;
		}
	}

	pthread_mutex_unlock( &sdio_ctrl.mutex );

	if( device && funcs->event != NULL ) {
		funcs->event( (void *)&sdio_ctrl, &device->instance, ev );
	}

	return( EOK );
}

int sdio_cd( sdio_hc_t *hc )
{
	sdio_dev_t				*dev;
	sdio_funcs_t			*funcs;
	sdio_hc_entry_t			*hce;
	struct sdio_device		*device;
	sdio_device_instance_t	inst;
	int						cd;

	dev		= &hc->device;
	hce		= &hc->entry;
	funcs	= &sdio_ctrl.connect_parm.funcs;

	pthread_mutex_lock( &hc->cd_mutex );

	cd = hce->cd( hc );

	if( ( dev->flags & DEV_FLAG_MEDIA_CHANGE ) || !( cd & CD_INS ) ) {
		atomic_clr( &dev->flags, ( DEV_FLAG_MEDIA_CHANGE | DEV_FLAG_INVALID_CARD | DEV_FLAG_WRITE_PROTECT ) );
		if( ( dev->flags & DEV_FLAG_PRESENT ) ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  removal path %d, cd state 0x%x", __FUNCTION__, hc->path, cd );
			atomic_clr( &dev->flags, DEV_FLAG_PRESENT );
			pthread_mutex_lock( &sdio_ctrl.mutex );

				// look up device based on path, generation
			for( device = TAILQ_FIRST( &sdio_ctrl.dlist ); device != NULL; device = TAILQ_NEXT( device, dlink ) ) {
				if( device->instance.path == hc->path && device->instance.generation == hc->generation ) {
					break;
				}
			}

			if( device ) {
					// we can't use _sdio_synchronize to set the
					// pending flag since we already hold the mutex
				atomic_set( &device->flags, DEV_FLAG_RMV_PENDING );

				while( device->usage ) {
					pthread_cond_wait( &sdio_ctrl.cd_cond, &sdio_ctrl.mutex );
				}
			}

			pthread_mutex_unlock( &sdio_ctrl.mutex );

			if( device && funcs->removal != NULL ) {
				funcs->removal( (void *)&sdio_ctrl, &device->instance );
			}

			if( !cd ) {
				sdio_power( hc, SDIO_PWR_OFF );
			}
		}
	}

	if( ( cd & CD_INS ) ) {
		if( !( dev->flags & ( DEV_FLAG_PRESENT | DEV_FLAG_INVALID_CARD ) ) ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  insertion path %d, cd state 0x%x", __FUNCTION__, hc->path, cd );
			if( _sdio_attach( hc ) == EOK ) {
				sdio_cd_info( hc );
				if( ( cd & CD_WP ) ) {
					atomic_set( &dev->flags, DEV_FLAG_WRITE_PROTECT );
				}
				memset( &inst, 0, sizeof( sdio_device_instance_t ) );
				inst.path			= hc->path;
				inst.func			= 0;
				inst.generation		= hc->generation;
				inst.ident.dtype	= dev->dtype;
				if( funcs->insertion ) {
					funcs->insertion( (void *)&sdio_ctrl, &inst );
				}
			}
			else {
				if( !( dev->flags & DEV_FLAG_INVALID_CARD ) ) {
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  Unsupported card inserted ", __FUNCTION__ );
					atomic_set( &dev->flags, DEV_FLAG_INVALID_CARD );
				}
			}
		}
	}

	pthread_mutex_unlock( &hc->cd_mutex );

	return( ( dev->flags & DEV_FLAG_PRESENT ) );
}

// hc callback for card detect, re-tuning
int sdio_hc_event( sdio_hc_t *hc, int ev )
{
	switch( ev ) {
		case HC_EV_CD:					// notify sdio_cd_thread
			atomic_set( &hc->device.flags, DEV_FLAG_MEDIA_CHANGE );
#if ( __PTR_BITS__ == 64 )
			MsgSendPulsePtr( sdio_ctrl.cd_coid, hc->priority, SDIO_EV_CD, hc );
#else
			MsgSendPulse( sdio_ctrl.cd_coid, hc->priority, SDIO_EV_CD, (uintptr_t)hc );
#endif
			break;

		case HC_EV_TUNE:
			atomic_set( &hc->flags, HC_FLAG_TUNE );
			break;

		default:
			break;
	}

	return( EOK );
}

int sdio_timer_settime( int tid, uint32_t sec, uint32_t nsec, int repeat )
{
	struct itimerspec	value;

	memset( &value, 0, sizeof( value ) );

	value.it_value.tv_sec	= sec;
	value.it_value.tv_nsec	= nsec;

	if( repeat ) {
		value.it_interval.tv_sec	= sec;
		value.it_interval.tv_nsec	= nsec;
	}

	if( timer_settime( tid, 0, &value, NULL ) == -1 ) {
		return( errno );
	}

	return( EOK );	
}	

int sdio_cd_enum( sdio_ctrl_t *sc )
{
	pthread_sleepon_lock( );
	while( sc->cd_enum == SDIO_ENUM_DISABLE ) {
		pthread_sleepon_wait( &sc->cd_enum );
	}
	pthread_sleepon_unlock( );
	return( EOK );
}

void *sdio_cd_thread( void *hdl )
{
	sdio_ctrl_t			*sc;
	sdio_hc_t 			*hc;
	int					rid;
	struct _pulse		pulse;
	struct sigevent		event;

	sc		= (sdio_ctrl_t *)hdl;

	if( ( sc->cd_chid = ChannelCreate( _NTO_CHF_PRIVATE | _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK ) ) == -1 ||
			( sc->cd_coid = ConnectAttach( 0, 0, sc->cd_chid, _NTO_SIDE_CHANNEL, 0 ) ) == -1 ) {
		sdio_set_thread_state( &sc->state, SDIO_TSTATE_INIT_FAILURE );
		return( NULL );
	}

	SIGEV_PULSE_INIT( &event, sc->cd_coid, sc->priority, SDIO_EV_TIMER, NULL );
	if( timer_create( CLOCK_REALTIME, &event, &sc->cd_timerid ) == -1 ) {
		sdio_set_thread_state( &sc->state, SDIO_TSTATE_INIT_FAILURE );
		return( NULL );
	}

	sdio_set_thread_state( &sc->state, SDIO_TSTATE_INITIALIZED );

	while( 1 ) {
		if( ( rid = MsgReceivePulse( sc->cd_chid, &pulse, sizeof( pulse ), NULL ) ) == -1 ) {
			break;
		}

		switch( pulse.code ) {
			case SDIO_EV_TIMER:
				for( hc = TAILQ_FIRST( &sdio_ctrl.hlist ); hc; hc = TAILQ_NEXT( hc, hlink ) ) {
					if( !( hc->caps & HC_CAP_CD_INTR ) && !( hc->caps & HC_CAP_SLOT_TYPE_EMBEDDED ) ) {
						sdio_cd_enum( sc );
						sdio_cd( hc );
					}
				}
				break;

			case SDIO_EV_CD:
				sdio_cd_enum( sc );
				sdio_cd( (sdio_hc_t *)pulse.value.sival_ptr );
				break;

			default:
//				sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: unknown pulse rid %d, type %x, subtype %, pulse %x, value %x, scoid %x", __FUNCTION__, rid, pulse.type, pulse.subtype, pulse.code, pulse.value, pulse.scoid );
				break;
		}

	}

	return( NULL );
}

void *sdio_hc_thread( void *hdl )
{
	sdio_hc_t 			*hc;
	sdio_hc_entry_t		*hce;
	int					rid;
	struct _pulse		pulse;
	struct sigevent		event;
	struct itimerspec	value;
	pthread_condattr_t	attr;

	hc		= (sdio_hc_t *)hdl;
	hce		= (sdio_hc_entry_t *)&hc->entry;

	if( pthread_condattr_init( &attr ) ||
			pthread_condattr_setclock( &attr, CLOCK_MONOTONIC ) ||
			pthread_cond_init( &hc->cond, &attr ) ||
			pthread_mutex_init( &hc->mutex, NULL ) ||
			pthread_mutex_init( &hc->cd_mutex, NULL ) ||
			( hc->hc_chid = ChannelCreate( _NTO_CHF_PRIVATE | _NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK ) ) == -1 ||
			( hc->hc_coid = ConnectAttach( 0, 0, hc->hc_chid, _NTO_SIDE_CHANNEL, 0 ) ) == -1 ) {
		sdio_set_thread_state( &hc->state, SDIO_TSTATE_INIT_FAILURE );
		pthread_condattr_destroy( &attr );
		return( NULL );
	}

	pthread_condattr_destroy( &attr );

	if( hc->tuning_count ) {
		memset( &value, 0, sizeof( value ) );
		value.it_value.tv_sec = hc->tuning_count;

		SIGEV_PULSE_INIT( &event, hc->hc_coid, hc->priority, HC_EV_TUNE, NULL );
		if( timer_create( CLOCK_REALTIME, &event, &hc->tuning_timerid ) == -1 ||
				timer_settime( hc->tuning_timerid, 0, &value, NULL ) == -1 ) {
			sdio_set_thread_state( &hc->state, SDIO_TSTATE_INIT_FAILURE );
			return( NULL );
		}
	}

	sdio_set_thread_state( &hc->state, SDIO_TSTATE_INITIALIZED );

	while( 1 ) {
		if( ( rid = MsgReceivePulse( hc->hc_chid, &pulse, sizeof( pulse ), NULL ) ) == -1 ) {
			break;
		}

		switch( pulse.code ) {
			case HC_EV_TUNE:
				sdio_hc_event( hc, HC_EV_TUNE );
				break;

			default:
				if( hce->event ) {
					if( hce->event( hc, &pulse ) != EOK ) {
//						sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: unknown pulse rid %d, type %x, subtype %, pulse %x, value %x, scoid %x", __FUNCTION__, rid, pulse.type, pulse.subtype, pulse.code, pulse.value, pulse.scoid );
					}
				}
				break;
		}
	}

	return( NULL );
}

int sdio_options( sdio_hc_t *hc, char *options )
{
	int					opt;
	int					val;
	uint32_t			*cap;
	uint32_t			ocap;
	uint32_t			ncap;
	int					status;
	char				*value;
	char				*argstr[2];
	sdio_hc_cfg_t		*cfg;
	char				*ltok;
	char				*delims = { ":" };
	static char			*opts[] = {
							"verbose", "priority",

							"hc",
							"vid", "did", "idx",
							"addr", "irq",
							"dma",

							"clk",		// operation clock
							"bw",		// set bus width
							"timing",	// set bus timing
							"~bmstr",	// disable bmstr/dma
							"~ac12",	// disable ac12
							"~ac23",	// disable ac23
							"pm",       // pm idle:sleep time in ms
							"bs",		// board specific options
							"emmc",		// connected to eMMC device
							NULL
						};

	cfg		= &hc->cfg;
	status	= EOK;

	while( options && *options != '\0' && status == EOK ) {
		if( ( opt = getsubopt( &options, opts, &value ) ) == -1 ) {
			//	Encountered an invalid option.
			report_invalid_suboption( __FUNCTION__, value );
			status = EINVAL; break;
		}

		switch( opt ) {
			case 0:			// verbosity
				if( value != NULL && *value != '\0' ) {
					sdio_parse_tuple( value, ':', &argstr[0], NULL );
					if( ( val = sdio_parse_number( argstr[0] ) ) != SDIO_INVALID_NUM ) {
						cfg->verbosity = val;
					}
				}
				else {
					cfg->verbosity++;
				}
				break;

			case 1:
				SDIO_ARG_VAL( opts[opt], value, status );
				val = strtol( value, 0, 0 );
				if( val >= sched_get_priority_min( SCHED_RR ) && val <= sched_get_priority_max( SCHED_RR ) )
					hc->priority = val;
				else {
					sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  Invalid priority '%s'", __FUNCTION__, value );
				}
				break;

			case 2:			// hc - host contoller
				SDIO_ARG_VAL( opts[opt], value, status );
				strlcpy( cfg->name, value, sizeof( cfg->name ) );
				break;

			case 3:			// vid - vender id
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->vid = val;
				}
				break;

			case 4:			// did - device id
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->did = val;
				}
				break;

			case 5:			// idx
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->idx = val;
				}
				break;

			case 6:			// addr
				sdio_parse_tuple( value, ':', &argstr[0], &argstr[1], NULL );
				SDIO_ARG_VAL( opts[opt], argstr[0], status );
				if( ( val = sdio_parse_number( argstr[0] ) ) != SDIO_INVALID_NUM ) {
					cfg->base_addr[cfg->base_addrs] = val;
					if( argstr[1] != NULL && *argstr[1] != '\0' ) {
						if( ( val = sdio_parse_number( argstr[1] ) ) != SDIO_INVALID_NUM ) {
							cfg->base_addr_size[cfg->base_addrs] = val;
						}
					}
					cfg->base_addrs++;
				}
				break;

			case 7:			// irq
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->irq[cfg->irqs++] = val;
				}
				break;

			case 8:			// dma - dma channel
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->dma_chnl[cfg->dma_chnls++] = val;
				}
				break;

			case 9:			// clk
				SDIO_ARG_VAL( opts[opt], value, status );
				if( ( val = sdio_parse_number( value ) ) != SDIO_INVALID_NUM ) {
					cfg->clk = val;
				}
				break;

			case 10:		// bw - bus width
				SDIO_ARG_VAL( opts[opt], value, status );
				ocap	= ncap = 0;
				value	= strtok_r( value, delims, &ltok );
				while( value != NULL ) {
					cap		= &ocap;
					if( value[0] == '~' ) {
						value++; cap = &ncap;
					}
					if( !strcmp( value, "4" ) )				*cap |= HC_CAP_BW4;
					else if( !strcmp( value, "8" ) )		*cap |= HC_CAP_BW8;
					else sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  Invalid bw '%s'", __FUNCTION__, value );
					value = strtok_r( NULL, delims, &ltok );
				}
				if( ocap ) {
					cfg->caps &= ~HC_CAP_BW_MSK; cfg->caps |= ocap;
				}
				if( ncap ) {
					cfg->caps &= ~ncap;
				}
				break;

			case 11:		// timing
				SDIO_ARG_VAL( opts[opt], value, status );
				ocap	= ncap = 0;
				value	= strtok_r( value, delims, &ltok );
				while( value != NULL ) {
					cap		= &ocap;
					if( value[0] == '~' ) {
						value++; cap = &ncap;
					}
					if( !strcmp( value, "hs" ) )			*cap |= HC_CAP_HS;
					else if( !strcmp( value, "ddr" ) )		*cap |= HC_CAP_DDR50;
					else if( !strcmp( value, "sdr" ) )		*cap |= ( HC_CAP_SDR12 | HC_CAP_SDR25 | HC_CAP_SDR50 | HC_CAP_SDR104 | HC_CAP_DDR50 );
					else if( !strcmp( value, "sdr12" ) )	*cap |= HC_CAP_SDR12;
					else if( !strcmp( value, "sdr25" ) )	*cap |= HC_CAP_SDR25;
					else if( !strcmp( value, "sdr50" ) )	*cap |= HC_CAP_SDR50;
					else if( !strcmp( value, "sdr104" ) )	*cap |= HC_CAP_SDR104;
					else if( !strcmp( value, "hs200" ) )	*cap |= HC_CAP_HS200;
					else if( !strcmp( value, "hs400" ) )	*cap |= HC_CAP_HS400;
					else sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s:  Invalid timing '%s'", __FUNCTION__, value );
					value = strtok_r( NULL, delims, &ltok );
				}
				if( ocap ) {
					cfg->caps &= ~HC_CAP_TIMING_MSK; cfg->caps |= ocap;
				}
				if( ncap ) {
					cfg->caps &= ~ncap;
				}
				break;

			case 12:		// ~bmstr
				cfg->caps &= ~HC_CAP_DMA;
				break;

			case 13:		// ~ac12
				cfg->caps &= ~HC_CAP_ACMD12;
				break;

			case 14:		// ~ac23
				cfg->caps &= ~HC_CAP_ACMD23;
				break;


			case 15:		// pm idle:sleep in ms
				sdio_parse_tuple( value, ':', &argstr[0], &argstr[1], NULL );
				SDIO_ARG_VAL( opts[opt], argstr[0], status );
				if( ( val = sdio_parse_number( argstr[0] ) ) != SDIO_INVALID_NUM ) {
					cfg->idle_time = val;
				}
				if( argstr[1] != NULL && *argstr[1] != '\0' ) {
					if( ( val = sdio_parse_number( argstr[1] ) ) != SDIO_INVALID_NUM ) {
						cfg->sleep_time = val;
					}
				}
				break;

			case 16:		// bs options
				SDIO_ARG_VAL( opts[opt], value, status );
				cfg->options = strdup( value );
				break;

			case 17:		// eMMC
				hc->caps |= HC_CAP_SLOT_TYPE_EMBEDDED;
				hc->flags |= HC_FLAG_DEV_MMC;
				break;

			default:
				break;

		}
	}

	if( !cfg->vid && cfg->name[0] == '\0' && cfg->idx == -1 ) {
		sdio_ctrl.flags		|= SDIO_CFLAG_SCAN;
		sdio_ctrl.verbosity	= cfg->verbosity;
		if( hc->priority != SDIO_PRIORITY ) {
			sdio_ctrl.priority = hc->priority;
		}
	}

	return( status );
}

int sdio_hc_dinit( sdio_hc_t *hc )
{
//	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: hc %p", __FUNCTION__, hc );

	if( hc->entry.dinit ) {
		hc->entry.dinit( hc );
	}

	if( hc->cfg.options ) {
		free( hc->cfg.options );
		hc->cfg.options = NULL;
	}

	if( hc->hc_coid != -1 ) {
		ConnectDetach( hc->hc_coid );
	}

	if( hc->hc_chid != -1 ) {
		ChannelDestroy( hc->hc_chid );
	}

	if( hc->tuning_timerid != -1 ) {
		timer_delete( hc->tuning_timerid );
	}

	if( hc->hc_tid != -1 ) {
//		pthread_cancel( hc->hc_tid );
		pthread_join( hc->hc_tid, NULL );
	}

#ifdef SDIO_PCI_SUPPORT
	sdio_pci_detach_device( hc );
#endif

	pthread_cond_destroy( &hc->cond );
	pthread_mutex_destroy( &hc->mutex );
	pthread_mutex_destroy( &hc->cd_mutex );

	return( EOK );
}

sdio_product_t *sdio_hc_lookup( int vid, int did, int class, char *name )
{
	sdio_vendor_t	*vend;
	sdio_product_t	*prod;


		// check for a match on vid, did, class
	if( vid && did ) {
		class &= ~0xff;
		for( vend = sdio_vendors; vend->vid; vend++ ) {
			if( vend->vid == vid || vend->vid == SDIO_VENDOR_ID_WILDCARD ) {
				for( prod = vend->chipsets; prod->did; prod++ ) {
					if( ( prod->did == SDIO_DEVICE_ID_WILDCARD &&
							prod->class == class ) || prod->did == did ) {
						return( prod );
					}
				}
			}
		}
	}

		// check for a match on product name
	if( name[0] != '\0' ) {
		for( vend = sdio_vendors; vend->vid; vend++ ) {
			for( prod = vend->chipsets; prod->did; prod++ ) {
				if( !strcmp( prod->name, name ) ) {
					return( prod );
				}
			}
		}
	}

	return( ( !vid && !did && name[0] == '\0' ) ? sdio_vendors[0].chipsets : NULL );
}

int sdio_hc_init( sdio_hc_t *hc )
{
	sdio_hc_cfg_t	*cfg;
	sdio_product_t	*prod;
	int				status;

	status			= EOK;
	hc->device.hc	= hc;
	cfg				= &hc->cfg;

	if( ( prod = sdio_hc_lookup( cfg->vid, cfg->did, cfg->class, cfg->name ) ) ) {
		if( ( status = sdio_create_thread( &hc->hc_tid, NULL, sdio_hc_thread, hc, hc->priority, &hc->state, "sdio_hc_thread" ) ) == EOK ) {
			if( ( status = prod->init( hc ) ) == EOK ) {
				if( cfg->name[0] == '\0' ) {
					strlcpy( cfg->name, prod->name, sizeof( cfg->name ) );
				}
				hc->flags |= HC_FLAG_INITIALIZED;
				return( EOK );
			}
			else {
				sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s: hc init failure %s", __FUNCTION__, prod->name );
			}
		}
		else {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s: sdio_hc_thread creation failure", __FUNCTION__ );
		}
	}
	else {
		if( cfg->name[0] != '\0' ) {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s sdio_hc_lookup failure %s", __FUNCTION__, cfg->name );
		}
		else {
			sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, cfg->verbosity, 0, "%s sdio_hc_lookup failure vid %x, did %x", __FUNCTION__, cfg->vid, cfg->did );
		}
		status = ENODEV;
	}

	return( status ? status : errno );
}

int sdio_cd_dinit( sdio_ctrl_t *sc )
{
//	sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: sc %p", __FUNCTION__, sc );

	if( sc->cd_timerid != -1 ) {
		timer_delete( sc->cd_timerid );
	}

	if( sc->cd_coid != -1 ) {
		ConnectDetach( sc->cd_coid );
	}

	if( sc->cd_chid != -1 ) {
		ChannelDestroy( sc->cd_chid );
	}

	if( sc->cd_tid != -1 ) {
//		pthread_cancel( sc->cd_tid );
		pthread_join( sc->cd_tid, NULL );
	}

	sc->cd_coid = sc->cd_chid = sc->cd_tid = sc->cd_timerid = -1;

	return( EOK );
}

int sdio_dinit( sdio_ctrl_t *sc )
{
	sdio_hc_t			*hc;
	sdio_hc_t			*nhc;
	struct sdio_cmd		*cmd;
	struct sdio_cmd		*ncmd;

	sdio_cd_dinit( sc );

	for( hc = TAILQ_FIRST( &sc->hlist ); hc; hc = nhc ) {
		nhc	= TAILQ_NEXT( hc, hlink );
		sdio_hc_dinit( hc );
		sdio_hc_free( hc );
	}

	for( cmd = TAILQ_FIRST( &sc->clist ); cmd; cmd = ncmd ) {
		ncmd	= TAILQ_NEXT( cmd, clink );
		TAILQ_REMOVE( &sc->clist, cmd, clink );
		free( cmd );
	}

	pthread_mutex_destroy( &sc->mutex );
	pthread_cond_destroy( &sc->cd_cond );

	return( EOK );
}

int sdio_init( sdio_ctrl_t *sc )
{
	int					status;

	TAILQ_INIT( &sc->hlist );
	TAILQ_INIT( &sc->dlist );
	TAILQ_INIT( &sc->clist );
	sc->cd_coid		= sc->cd_chid = sc->cd_tid = sc->cd_timerid = -1;
	sc->priority	= SDIO_PRIORITY;

	if( ( status = pthread_mutex_init( &sc->mutex, NULL ) ) == EOK ) {
		if( ( status = pthread_cond_init( &sc->cd_cond, NULL ) ) == EOK ) {
			if( ( status = sdio_create_thread( &sc->cd_tid, NULL, sdio_cd_thread, sc, sc->priority, &sc->state, "sdio_cd_thread" ) ) == EOK ) {
				return( EOK );
			}
		}
	}

	sdio_dinit( sc );

	return( status ? status : errno );
}

int _sdio_disconnect( )
{
	struct sdio_device	*device;

	sdio_cd_dinit( &sdio_ctrl );	// cleanup/destry cd thread

	pthread_mutex_lock( &sdio_ctrl.mutex );
	device = TAILQ_FIRST( &sdio_ctrl.dlist );
	memset( &sdio_ctrl.connect_parm, 0, sizeof( sdio_connect_parm_t ) );
	pthread_mutex_unlock( &sdio_ctrl.mutex );

	if( device ) {
		return( EBUSY );
	}

	sdio_dinit( &sdio_ctrl );

#ifdef SDIO_PCI_SUPPORT
	sdio_pci_dinit( );
#endif

	return( EOK );
}

int _sdio_connect( sdio_connect_parm_t *parm, struct sdio_connection **connection )
{
	sdio_hc_t		*hc;
	char			*margs;
	int				tmr;
	int				status;
	int				occurence;

	if( ( status = sdio_init( &sdio_ctrl ) ) != EOK ) {
		sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 0, 0, "%s: sdio_init failure", __FUNCTION__ );
		return( status );
	}

#ifdef SDIO_PCI_SUPPORT
	sdio_pci_init( );
#endif

		// parse command line args
	occurence = 1;
	while( ( margs = sdio_module_args( "sdio", parm->argc, parm->argv, occurence++ ) ) ) {
			// ignore args if we are scanning
		if( !( sdio_ctrl.flags & SDIO_CFLAG_SCAN ) ) {
			if( ( hc = sdio_hc_alloc( ) ) ) {
				status = sdio_options( hc, margs );
			}
			else {
				status = ENOMEM;
			}
		}
		free( margs );
		if( status ) {
			sdio_dinit( &sdio_ctrl );
			return( status );
		}
	}

		// no args or scan required
	if( TAILQ_FIRST( &sdio_ctrl.hlist ) == NULL || ( sdio_ctrl.flags & SDIO_CFLAG_SCAN ) ) {
#ifdef SDIO_PCI_SUPPORT
		sdio_pci_scan( );
#endif
#ifdef SDIO_SOC_SUPPORT
		sdio_soc_scan( );
#endif
	}
	else {
		for( hc = TAILQ_FIRST( &sdio_ctrl.hlist ); hc; hc = TAILQ_NEXT( hc, hlink ) ) {
#ifdef SDIO_PCI_SUPPORT
			if( ( status = sdio_pci_device( hc ) ) == EOK ) {
				continue;
			}
#endif

#ifdef SDIO_SOC_SUPPORT
			if( ( status = sdio_soc_device( hc ) ) == EOK ) {
				continue;
			}
#endif
			sdio_dinit( &sdio_ctrl );
			return( status );
		}
	}

	for( hc = TAILQ_FIRST( &sdio_ctrl.hlist ), tmr = 0; hc; hc = TAILQ_NEXT( hc, hlink ) ) {
		if( ( status = sdio_hc_init( hc ) ) != EOK ) {
			sdio_dinit( &sdio_ctrl );
			return( status );
		}

		if( ( hc->caps & HC_CAP_CD_INTR ) ) {
#if ( __PTR_BITS__ == 64 )
			MsgSendPulsePtr( sdio_ctrl.cd_coid, hc->priority, SDIO_EV_CD, hc );
#else
			MsgSendPulse( sdio_ctrl.cd_coid, hc->priority, SDIO_EV_CD, (uintptr_t)hc );
#endif
		}
		else {
			sdio_cd( hc );
		}

		if( !( hc->caps & HC_CAP_CD_INTR ) && !( hc->caps & HC_CAP_SLOT_TYPE_EMBEDDED ) ) {
			tmr = SDIO_TRUE;
		}
	}

	if( tmr ) {
		sdio_timer_settime( sdio_ctrl.cd_timerid, SDIO_CD_INTERVAL, 0, SDIO_TRUE );
	}

	return( TAILQ_FIRST( &sdio_ctrl.hlist ) ? EOK : ENODEV );
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/7.0.0/beta/hardware/devb/sdmmc/sdiodi/base.c $ $Rev: 813652 $")
#endif
