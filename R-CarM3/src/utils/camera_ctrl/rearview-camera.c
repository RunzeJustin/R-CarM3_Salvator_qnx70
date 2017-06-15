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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <screen/screen.h>
#include <vcapture/capture.h>
#include <vcapture/capture-adv-ext.h>
#include <sys/mman.h>
#include <sys/pps.h>
#include <hw/inout.h>
#include <errno.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
static int app_abort = 1;
pthread_mutex_t pps_mutex;
pthread_cond_t pps_cond;
int pps_cw, pps_wg;
pps_attrib_t	info;
pps_status_t    rc;
int visible = 0;
int zorder = 0;
char	buf[1024];
char 	value[1024];
typedef struct _pps_data
{
    screen_window_t screen_video;
    screen_window_t screen_app;
    capture_context_t ctx;
    int32_t dst_height;
}pps_data;
pps_data ppsdata;
void *signal_thread (void *arg)
{
    pthread_t t1, t2;
    sigset_t signals;
    int sig; 
    int result;
    t1 = pthread_self();
    pthread_setname_np(t1, "signal_thread");
    t2 = pthread_self();
    pthread_detach(t2);
    sigaddset (&signals,  SIGINT ); 
    sigaddset (&signals,  SIGTERM );	
    do
        result = sigwait (&signals, &sig);
    while ( result );
    app_abort = 0;
    return 0;
}
int thread_pps(void* data)
{
    int		fd_cw, fd_wg;
    fd_set	rfd;
    int		nread = -1;
    int result;
    pps_data *pdata = (pps_data*)data;
    screen_window_t screen_video = pdata->screen_video;
    pthread_mutex_init(&pps_mutex, 0);
    pthread_cond_init(&pps_cond, 0);
    char* reqbuf;
    pps_cw = 0;
    pps_wg = 0;

    while ( access("/pps/system/navigator/command?wait,delta", R_OK)
       || access("/pps/system/navigator/windowgroup?wait,delta", R_OK) )
    sleep(1);
    fd_cw = open("/pps/system/navigator/command?wait,delta", 0, 438);
    if ( fd_cw != -1 )
    {
        fd_wg = open("/pps/system/navigator/windowgroup?wait,delta", 0, 438);
        if ( fd_wg  != -1 )
        {
            memset(value,0,1024);
            slogf(_SLOG_SETCODE(_SLOGC_TEST, 1),_SLOG_INFO,
            "%s: camera startup, will render until a window group is published by HMI, regardless of other PPS interactions",
            "rearview-camera");
            while ( 1 )
            {
                FD_ZERO( &rfd );
                FD_SET( fd_cw, &rfd );
                FD_SET( fd_wg, &rfd );
                result = select(1 + max( fd_cw, fd_wg ), &rfd, NULL, NULL, NULL);
                if ( result == -1 )
                {
                    break;
                }
                if((result >0) && FD_ISSET(fd_cw,&rfd))
                {	
                    memset(&info,0, sizeof(pps_attrib_t));
                    memset(buf,0,1024);
                    nread = read(fd_cw, buf, sizeof(buf)-1);
                    if ( nread <= 0 )
                    {
                        return 0;
                    }
                    if (nread > 1)
                        buf[nread] = '\0';
                    reqbuf = buf;
                    while((rc = ppsparse(&reqbuf, NULL, NULL, &info, 0)) != PPS_END) 	
                    {				
                        if( (rc == PPS_ATTRIBUTE) && !strcmp(info.attr_name, "rearview_camera") )
                        {
                            pthread_mutex_lock(&pps_mutex);
                            if ( !strcmp(info.value, "{\"action\": \"pause\"}") )
                            {
                                pps_cw = 0;	
                                visible = 0;
                                slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: stop rendering message received from HMI", "rearview-camera");
                                screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_VISIBLE, &visible);
                            }
                            else
                            {
                                pps_cw = 1;	
                                visible = 1;
                                zorder = -1;
                                slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: start rendering message received from HMI: pdata->dst_height: %d", "rearview-camera", pdata->dst_height);
                                screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_VISIBLE, &visible);
                                slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: start rendering message received from HMI: info.value: %s", "rearview-camera",info.value);
                                pthread_cond_signal(&pps_cond);
                            }
                            pthread_mutex_unlock(&pps_mutex);
                        }
                        if(rc==PPS_ERROR)
                            break;
                    }
                }
                result = select(1 + max( fd_cw, fd_wg ), &rfd, NULL, NULL, NULL);
                if ( result == -1 )
                {
                    slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: select window group failed", "rearview-camera");
                    break;
                }
                if((result >0) && FD_ISSET(fd_wg,&rfd))
                {
                    memset(&info,0, sizeof(pps_attrib_t));	
                    memset(buf,0,1024);
                    nread = read(fd_wg, buf, sizeof(buf)-1);
                    if ( nread <= 0 )
                    {
                        slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: read window group failed", "rearview-camera");
                        return 0;
                    }	
                    if (nread > 1)
                        buf[nread] = '\0';
                    reqbuf = buf;
                    while((rc = ppsparse(&reqbuf, NULL, NULL, &info, 0)) != PPS_END) 	
                    {
                        if( (rc == PPS_ATTRIBUTE) && !strcmp(info.attr_name, "rearview_camera") )
                        {
                            if ( !strcmp(value, info.value) )
                            {
                                slogf(_SLOG_SETCODE(_SLOGC_TEST, 1),_SLOG_INFO,"%s: will not join window group, same as previous (%s = %s)","rearview-camera",
                                    value,info.value);
                            }
                            else
                            {
                                strcpy(value, info.value);	
                                pthread_mutex_lock(&pps_mutex);
                                    if ( !screen_join_window_group(screen_video, value) )
                                    {
                                        pps_cw = 0;
                                        pps_wg = 1;
                                    slogf(_SLOG_SETCODE(_SLOGC_TEST, 1), _SLOG_INFO, "%s: joining window group -> %s", "rearview-camera", info.value);
                                }	
                                pthread_mutex_unlock(&pps_mutex);
                            }
                            
                        }
                        if(rc==PPS_ERROR)
                            break;
                    }
                }
            }
        }
    }
    return 0;
}
int initialize_pps(void *data)
{ 
    int result; 
    pthread_attr_t att;
    pthread_attr_init(&att);
    pthread_attr_setdetachstate(&att, 1);
    if ( pthread_create(0, &att, (void *)thread_pps, data) )
    {
        printf("ERROR: Unable to create PPS monitoring thread...\n");
        result = -1;
    }
    else
    {		
        result = 0;
    }
    return result;
}

int main(int argc, char **argv)
{
    int j, count;
    char *tmp1, *tmp2, *value, *tmp3;
    capture_context_t ctx = NULL;
    const char *info = NULL;
    int32_t pipeline;
    int32_t device, n_device, source_type, source;
    int32_t window_width, window_height, window_x, window_y;
    int32_t view_width, view_height, view_x, view_y;
    int32_t window_pos[2];
    int32_t view_pos[2];
    int32_t window_size[2];
    int32_t view_size[2] ;
    int32_t dst_size[2];
    int rect[4] = { 0, 0, 0, 0 };
    int rect_buf[4] = { 0, 0, 0, 0 };
    int32_t dst_format;
    int32_t crop_height,crop_width, crop_x, crop_y;
    int32_t src_height;
    int32_t src_width;
    int32_t dst_stride;
    int32_t dst_height;
    int32_t dst_width;
    int32_t thread_priority;
    int32_t frame_nbuffers;
    int frame_times [4], frame_flags [4];
    int32_t color_saturation;
    int32_t color_hue;
    int32_t brightness;
    int32_t contrast;
    int32_t nbufs;
    int32_t  type, val, id = 0;
	bool quit_if_no_video = false;
    char *disp = NULL;
    screen_display_t *screen_displays;
    screen_display_t screen_disp = NULL;
    screen_buffer_t screen_buf[4];
    screen_buffer_t screen_pbuf = NULL;
    unsigned char *frame_buffers[4];
    unsigned char *p_ptr = NULL;
    char str_group[50];
    int parent_stride = 0;
    int32_t screen_format;
    pthread_attr_t attr;
    sigset_t signals;
    app_abort = 1;
    thread_priority = -1;
    device = 0;
    n_device = 1;
    nbufs = 4;
    screen_format = SCREEN_FORMAT_RGB888;
    frame_nbuffers = 0;
    source_type = -1;
    source = 1;
    src_width = -1;
    src_height = -1;
    window_width = -1; 
    window_height = -1;
    view_width = -1;
    view_height = -1;
    view_x = 0;
    view_y = 0;
    dst_width = -1;
    dst_height = -1;
    crop_height = -1;
    crop_width = -1;
    crop_x = -1;
    crop_y = -1;
    window_x = 0; 
    window_y = 0;
    dst_format = SCREEN_FORMAT_UYVY;
    pipeline = -1;
    color_saturation = 4095;
    brightness = 4095;
    contrast = 4095;
    color_hue = 4095;
    id = 0;
	
    pthread_attr_init (&attr); 
    sigfillset (&signals);
    pthread_sigmask (SIG_BLOCK, &signals, NULL);
    pthread_create (NULL, &attr, signal_thread, NULL);
    
    while(--argc > 0) 
    {
        tmp1 = argv[1];
        tmp2 = argv[1];
        ++argv;
        if ( !strncmp(tmp2, "-format=", 8) )
        {
            tmp1 += 8;
            if ( !strcmp((const char *)tmp1, "rgb888") )
            {
                dst_format = SCREEN_FORMAT_RGBA8888;
            }
            else if ( !strcmp((const char *)tmp1, "rgb565") )
            {
                dst_format = SCREEN_FORMAT_RGB565;	
            }
            else if ( !strcmp((const char *)tmp1, "rgba5551") )
            {
                dst_format = SCREEN_FORMAT_RGBA5551;	
            }
            else if ( !strcmp((const char *)tmp1, "uyvy") )
            {
                dst_format = SCREEN_FORMAT_UYVY;	
            }
            else
            {
                printf("rearview-camera: invalid format %s\n",tmp1);
            }
        }
		if ( !strncmp((const char *)tmp2, "-quit-if-no-video", strlen("-quit-if-no-video")))
		{
			quit_if_no_video = true;
		}	
        if ( !strncmp((const char *)tmp2, "-device=", 8) )
        {
            device = atoi((const char *)(tmp2 + 8));
        }	
        if ( !strncmp((const char *)tmp2, "-source=", 8) )
        {
            source = atoi((const char *)(tmp2 + 8));
        }
        
        if ( !strncmp((const char *)tmp2, "-size=", 6) )
        {
          value = (tmp2 + 6);
          window_width = atoi(value);
          do
            tmp3 = value++;
          while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
          window_height = atoi(value);
        }
        else if ( !strncmp((const char *)tmp2, "-display=", strlen("-display=")) )
        {
            disp = (char *)(tmp2 + 9);
        }
        else if ( !strncmp((const char *)tmp2, "-ssize=", 7) )
        {
            value = (tmp2 + 7);
            view_width = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            view_height = atoi(value);
        }
        else if ( !strncmp((const char *)tmp2, "-csize=", 7) )
        {
            value = (tmp2 + 7);
            crop_width = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            crop_height = atoi(value);
        }
        else if ( !strncmp((const char *)tmp2, "-bsize=", 7) )
        {
            value = (tmp2 + 7);
            dst_width = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            dst_height = atoi(value);			
        }
        else if ( !strncmp((const char *)tmp2, "-pos=", 5) )
        {
            value = (tmp2 + 5);
            window_x = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            window_y = atoi(value);
        }
        else if ( !strncmp((const char *)tmp2, "-cpos=", 6) )
        {
            value = (tmp2 + 6);
            crop_x = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            crop_y = atoi(value);		
        }
        else if ( !strncmp((const char *)tmp2, "-spos=", 6) )
        {
            value = (tmp2 + 6);
            view_x = atoi(value);
            do
            tmp3 = value++;
            while ( (unsigned int)(uint8_t)*tmp3 - 48 <= 9 );
            view_y = atoi(value);
        }
        else
        {
            if ( !strncmp((const char *)tmp2, "-pipeline=", 0xA) )
            {
                pipeline = atoi((const char *)(tmp2 + 10));
            }
            if ( !strncmp((const char *)tmp2, "-brightness=", 0xC) )
            {
                brightness = atoi((const char *)(tmp2 + 12));
            }
            if ( !strncmp((const char *)tmp2, "-contrast=", 0xA) )
            {
                contrast = atoi((const char *)(tmp2 + 10));
            }
            if ( !strncmp((const char *)tmp2, "-hue=", 5) )
            {
                color_hue = atoi((const char *)(tmp2 + 5));
            }
            if ( !strncmp((const char *)tmp2, "-saturation=", 0xC) )
            {
                color_saturation = atoi((const char *)(tmp2 + 12));
            }
            if ( !strncmp((const char *)tmp2, "-zorder=", 8) )
            {
                zorder = atoi((const char *)(tmp2 + 8));
            }
        }
    }
    if ( dst_width == -1 || dst_height == -1 )
    {
        printf("error: buffer is not specified!\n");
        return 0;
    }		
    screen_context_t screen_ctx;
    screen_window_t screen_video;
    screen_window_t screen_app;
    screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
    screen_create_window_type( &screen_video,screen_ctx, SCREEN_APPLICATION_WINDOW);
    /* Get the number of supported displays with this context. */
    count = 0;
    if(screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT,&count))
    {
        printf("screen_get_context_property_iv(SCREEN_PROPERTY_DISPLAY_COUNT) failed\n");
        goto screen_failed;
    }
    if( count > 0)
    {
        screen_displays = calloc(count, sizeof(screen_display_t));
        if ( !screen_displays )
        {
            printf("could not allocate memory for display list\n");
            goto screen_failed;
        }
        if ( screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)screen_displays) )
        {
            printf("screen_get_context_property_pv(SCREEN_PROPERTY_DISPLAYS) failed\n");
            goto screen_failed;
        }
        if(!disp){
            screen_disp = screen_displays[0];
        }/* Otherwise, determine which display has been requested for the screen shot. */
        else 
        {
            if (isdigit(*disp)) 
            {
                id = strtoul(disp, 0, NULL);
                for (j = 0; j < count; j++) 
                {
                    screen_get_display_property_iv(screen_displays[j], SCREEN_PROPERTY_ID,
                                                        &val);
                    if (val == id) 
                    {
                        screen_disp = screen_displays[j];
                        break;
                    }
                }
            } 
            else 
            {
                if (!strcmp(disp, "internal")) 
                {
                    type = SCREEN_DISPLAY_TYPE_INTERNAL;
                } 
                else if (!strcmp(disp, "rgb")) 
                {
                    type = SCREEN_DISPLAY_TYPE_COMPONENT_RGB;
                } 
                else if (!strcmp(disp, "dvi")) 
                {
                    type = SCREEN_DISPLAY_TYPE_DVI;
                } 
                else if (!strcmp(disp, "hdmi")) 
                {
                    type = SCREEN_DISPLAY_TYPE_HDMI;
                } 
                else 
                {
                    type = SCREEN_DISPLAY_TYPE_OTHER;
                }

                printf("%s\n", disp);
                
                for (j = 0; j < count; j++) 
                {
                    screen_get_display_property_iv(screen_displays[j], SCREEN_PROPERTY_TYPE,
                                                        &val);
                    if (val == type) 
                    {
                        screen_disp = screen_displays[j];
                        break;
                    }
                }
            }
        }

        free(screen_displays);
    }
    if (!screen_disp) 
    {
        printf("no displays\n");
        return 1;
    }
    if ( screen_set_window_property_pv(screen_video, SCREEN_PROPERTY_DISPLAY, (void**)&screen_disp))
    {
      printf("screen_set_window_property_ptr(SCREEN_PROPERTY_DISPLAY) failed\n\r");
      goto screen_failed;
    }
    int usage = SCREEN_USAGE_CAPTURE | SCREEN_USAGE_VIDEO | SCREEN_USAGE_WRITE | SCREEN_USAGE_READ;
    if(pipeline != -1)
    {
        usage = SCREEN_USAGE_CAPTURE | SCREEN_USAGE_OVERLAY | SCREEN_USAGE_VIDEO | SCREEN_USAGE_WRITE;
    }
    if(screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_USAGE, &usage) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed.\n");
        goto screen_failed;
    }
    if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_FORMAT, &dst_format) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT) failed.\n");
        goto screen_failed;
    }
    if( (window_width == -1) || (window_height == -1) ){
        if ( screen_get_window_property_iv(screen_video, SCREEN_PROPERTY_SIZE, window_size) )
        {
            printf("screen_get_window_property_iv(SCREEN_PROPERTY_SIZE) failed.\n");
            goto screen_failed;
        }
        rect[2] = window_size[0];
        rect[3] = window_size[1];
    }
    else
    {
        rect[2] = window_width;
        rect[3] = window_height;
        window_size[0] = window_width;
        window_size[1] = window_height;
    }
    if ( (dst_width != 0) || (dst_height != 0) )
    {
        dst_size[0] = dst_width;
        dst_size[1] = dst_height;
        rect_buf[2] = dst_width;
        rect_buf[3] = dst_height;
        if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_BUFFER_SIZE, dst_size) ){
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_BUFFER_SIZE) failed.\n");
            goto screen_failed;
        }
    }
    
    if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_SIZE, rect + 2) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) failed.\n");
        goto screen_failed;
    }
    if ( (window_x != 0) || (window_y != 0) )
    {
        window_pos[0] = window_x;
        window_pos[1] = window_y;
        if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_POSITION, window_pos) )
        {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_POSITION) failed.\n");
            goto screen_failed;
        }
    }
    if ( pipeline != -1 )
    { 
        if(screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_PIPELINE, &pipeline) )
        {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_PIPELINE)failed.\n");
            goto screen_failed;
        }	
    }
    if (zorder)
    { 
        if(screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_ZORDER, &zorder) )
        {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_ZORDER)failed.\n");
            goto screen_failed;
        }	
    }
    screen_set_window_property_cv(screen_video, SCREEN_PROPERTY_ID_STRING,15,"rearview-camera");
    if ( screen_create_window_buffers(screen_video, nbufs) )
    {
        printf("screen_create_window_buffers with screen_video failed.\n");
        goto screen_failed;
    }
    if ( screen_get_window_property_iv(screen_video, SCREEN_PROPERTY_RENDER_BUFFER_COUNT, &frame_nbuffers) )
    {
        printf("screen_get_window_property_iv(SCREEN_PROPERTY_RENDER_BUFFER_COUNT) failed.\n");
        goto screen_failed;
    }
    if ( screen_get_window_property_pv(screen_video, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)screen_buf) )
    {
        printf("screen_get_window_property_pv(SCREEN_PROPERTY_RENDER_BUFFERS) failed.\n");
        goto screen_failed;
    }
    for ( j = 0; j < frame_nbuffers; ++j )
    {
        if ( screen_get_buffer_property_pv(screen_buf[j], SCREEN_PROPERTY_POINTER, (void**)&frame_buffers[j]) )
        {
            printf("screen_get_window_property_pv(SCREEN_PROPERTY_POINTER) failed.\n");
            goto screen_failed;
        }
    }	
    if ( screen_get_buffer_property_iv(screen_buf[0], SCREEN_PROPERTY_STRIDE, &dst_stride) )
    {
        printf("screen_get_buffer_property_iv(SCREEN_PROPERTY_STRIDE) failed.\n");
        goto screen_failed;
    }
    if ( screen_create_window_type(&screen_app, screen_ctx, SCREEN_APPLICATION_WINDOW) )
    {
        printf("screen_create_window_type(SCREEN_APPLICATION_WINDOW) failed\n");
        goto screen_failed;
    }
    
    ctx = capture_create_context(0);
    if(!ctx)
    {
        printf("capture_create_context failed \n");
        return 0;
    }
    if ( capture_get_property_i(ctx, CAPTURE_PROPERTY_NDEVICES, &n_device) )
    {
        printf("capture_get_property_i(CAPTURE_PROPERTY_NDEVICES failed.\n");
        goto failed;
    }
    if ( n_device >= 0)
    {
        /* Encode both capture source and screen ID into "device" variable */
        /* 0th byte: capture source ID */
        /* 1st byte: screen ID */
        device = (device & 0xF) | ((id & 0xF) << 4);
        if(capture_set_property_i(ctx, CAPTURE_PROPERTY_DEVICE, device) ) 
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_DEVICE) failed\n");
            goto failed;
        }
    }
    capture_get_property_p(ctx, CAPTURE_PROPERTY_DEVICE_INFO, (void**)&info);
    if ( capture_is_property(ctx, CAPTURE_ENABLE)
    && capture_is_property(ctx, CAPTURE_PROPERTY_FRAME_NBUFFERS)
    && capture_is_property(ctx, CAPTURE_PROPERTY_FRAME_BUFFERS)
    && capture_is_property(ctx, CAPTURE_PROPERTY_DST_FORMAT) )
    {
        
        if ( source_type >= 0 && capture_is_property(ctx, CAPTURE_PROPERTY_ADV_SRC_TYPE) && capture_set_property_i(ctx, CAPTURE_PROPERTY_ADV_SRC_TYPE, source_type) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_ADV_SRC_TYPE failed\n");
            goto failed;
        }
        if ( source >= 0 && capture_set_property_i(ctx, CAPTURE_PROPERTY_SRC_INDEX, source) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_SRC_INDEX failed\n");
            goto failed;
        }
        if ( color_saturation != 4095 && capture_is_property(ctx, CAPTURE_PROPERTY_SATURATION) && capture_set_property_i(ctx, CAPTURE_PROPERTY_SATURATION, color_saturation) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_SATURATION failed\n");
            goto failed;
        }
        if ( brightness != 4095 && capture_is_property(ctx, CAPTURE_PROPERTY_BRIGHTNESS) && capture_set_property_i(ctx, CAPTURE_PROPERTY_BRIGHTNESS, brightness) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_BRIGHTNESS failed\n");
            goto failed;
        }
        if ( contrast != 4095 && capture_is_property(ctx, CAPTURE_PROPERTY_CONTRAST) && capture_set_property_i(ctx, CAPTURE_PROPERTY_CONTRAST, contrast) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_CONTRAST failed \n");
            goto failed;
        }
        if ( color_hue != 4095 && capture_set_property_i(ctx, CAPTURE_PROPERTY_HUE, color_hue) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_HUE failed \n");
            goto failed;
        }
        if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_FRAME_NBUFFERS, frame_nbuffers) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_FRAME_NBUFFERS failed\n");
            goto failed;
        }
        if ( capture_set_property_p(ctx, CAPTURE_PROPERTY_FRAME_BUFFERS, frame_buffers) )
        {
            printf("capture_set_property_p(CAPTURE_PROPERTY_FRAME_BUFFERS failed\n");
            goto failed;
        }
        if ( capture_is_property(ctx, CAPTURE_PROPERTY_DST_STRIDE) && capture_set_property_i(ctx, CAPTURE_PROPERTY_DST_STRIDE, dst_stride) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_DST_STRIDE) failed\n");
            goto failed;
        }
        if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_DST_FORMAT, dst_format) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_DST_FORMAT) failed\n");
            goto failed;
        }
        if ( capture_is_property(ctx, CAPTURE_PROPERTY_SRC_WIDTH) && 
            capture_is_property(ctx, CAPTURE_PROPERTY_SRC_HEIGHT) && 
                            (src_width != -1) && (src_height != -1) )
        {
            if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_SRC_WIDTH, src_width) )
            {
                printf("capture_set_property_i(CAPTURE_PROPERTY_SRC_WIDTH) failed\n");
                goto failed;
            }
            if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_SRC_HEIGHT, src_height) )
            {
                printf("capture_set_property_i(CAPTURE_PROPERTY_SRC_HEIGHT) failed\n");
                goto failed;
            }
        }
        if ( capture_is_property(ctx, CAPTURE_PROPERTY_CROP_WIDTH)
            && capture_is_property(ctx, CAPTURE_PROPERTY_CROP_HEIGHT)
            && capture_is_property(ctx, CAPTURE_PROPERTY_CROP_X)
            && capture_is_property(ctx, CAPTURE_PROPERTY_CROP_Y) )
        {
            if ( (crop_width != -1) && (crop_height != -1) )
            {
                if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_CROP_WIDTH, crop_width) )
                {
                    printf("capture_set_property_i(CAPTURE_PROPERTY_CROP_WIDTH) failed\n");
                    goto failed;
                }
                if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_CROP_HEIGHT, crop_height) )
                {
                    printf("capture_set_property_i(CAPTURE_PROPERTY_CROP_HEIGHT) failed\n");
                    goto failed;
                }
            
            }
            if ( (crop_x != -1) && (crop_y != -1) )
            {
                if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_CROP_X, crop_x) )
                {
                    printf("capture_set_property_i(CAPTURE_PROPERTY_CROP_X) failed\n");
                    goto failed;
                }
                if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_CROP_Y, crop_y) )
                {
                    printf("capture_set_property_i(CAPTURE_PROPERTY_CROP_Y) failed\n");
                    goto failed;
                }
            }
        }
        if ( capture_is_property(ctx, CAPTURE_PROPERTY_DST_WIDTH) && capture_is_property(ctx, CAPTURE_PROPERTY_DST_HEIGHT) )
        {
            if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_DST_WIDTH, dst_width) )
            {
                printf("capture_set_property_i(CAPTURE_PROPERTY_DST_WIDTH) failed\n");
                goto failed;
            }
            if ( capture_set_property_i(ctx, CAPTURE_PROPERTY_DST_HEIGHT, dst_height) )
            {
                printf("capture_set_property_i(CAPTURE_PROPERTY_DST_HEIGHT) failed\n");
                goto failed;
            }
        }
        if ( (view_width == -1) || (view_height == -1) || 
                     (view_width + view_x > dst_width) ||
                     (view_height + view_y > dst_height) ) 
        {
                        
            printf("adjusting the source viewport size from %d x %d to %d x %d\n",
                view_width, view_height,dst_width - view_x, dst_height - view_y );
            view_width = dst_width - view_x;
            view_height = dst_height - view_y;
        }
        view_size[0] = view_width;
        view_size[1] = view_height;
        if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_SOURCE_SIZE, view_size) )
        {
            printf("screen_set_window_property_iv(SCREEN_PROPERTY_SOURCE_SIZE) failed\n");
            goto failed;
        }
        if ( (view_x !=0 ) || (view_y != 0) )
        {
            view_pos[0] = view_x;
            view_pos[1] = view_y;
            if ( screen_set_window_property_iv(screen_video, SCREEN_PROPERTY_SOURCE_POSITION, view_pos) )
            {
                printf("screen_set_window_property_iv(SCREEN_PROPERTY_SOURCE_POSITION) failed\n");
                goto failed;
            }
        }
        if ( (thread_priority != -1) && capture_set_property_i(ctx, CAPTURE_PROPERTY_THREAD_PRIORITY, thread_priority) )
        {
            printf("capture_set_property_i(CAPTURE_PROPERTY_THREAD_PRIORITY) failed\n");
            goto failed;
        }
        memset(frame_flags,4,4);
        memset(frame_times,0,4);
        capture_set_property_p(ctx, CAPTURE_PROPERTY_FRAME_TIMESTAMP, frame_times);
        if ( capture_is_property(ctx, CAPTURE_PROPERTY_FRAME_FLAGS) && capture_set_property_p(ctx, CAPTURE_PROPERTY_FRAME_FLAGS, frame_flags) )
        {
          printf("capture_set_property_i(CAPTURE_PROPERTY_FRAME_FLAGS failed\n");
          goto failed;
        }
        capture_set_property_i(ctx, CAPTURE_ENABLE, 1);
        if ( capture_update(ctx, 0) == -1 )
        {
            printf("capture_update failed.\n");
			goto failed;
        }
    
    }
    int index = 0; 
    if ( screen_set_window_property_pv(screen_app, SCREEN_PROPERTY_DISPLAY, (void**)&screen_disp))
    {
      printf("screen_set_window_property_ptr(SCREEN_PROPERTY_DISPLAY) failed\n\r");
      goto screen_failed;
    }
    usage = SCREEN_USAGE_WRITE;
    if ( screen_set_window_property_iv(screen_app, SCREEN_PROPERTY_USAGE, &usage) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed\n");
        goto screen_failed;
    }
    screen_format = SCREEN_FORMAT_RGBA8888;
    if ( screen_set_window_property_iv(screen_app, SCREEN_PROPERTY_FORMAT, &screen_format) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed\n");
        goto screen_failed;
    }
    if ( screen_set_window_property_iv(screen_app, SCREEN_PROPERTY_SIZE, window_size) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) failed\n");
        goto screen_failed;
    }
    if ( screen_create_window_buffers(screen_app, 1) )
    {
        printf("screen_create_window_buffers) failed\n");
        goto screen_failed;
    }
    if ( screen_get_window_property_pv(screen_app, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)&screen_pbuf) )
    {
        printf("screen_set_window_property_iv(SCREEN_PROPERTY_RENDER_BUFFERS) for parent buffer failed\n");
        goto screen_failed;
    }
    if ( screen_get_buffer_property_pv(screen_pbuf, SCREEN_PROPERTY_POINTER, (void **)&p_ptr) )
    {
        printf("screen_get_buffer_property_pv(SCREEN_PROPERTY_POINTER) for parent buffer failed\n");
        goto screen_failed;
    }
    if ( screen_get_buffer_property_iv(screen_pbuf, SCREEN_PROPERTY_STRIDE, &parent_stride) )
    {
        printf("screen_get_buffer_property_iv(SCREEN_PROPERTY_STRIDE) for parent buffer failed\n");
        goto screen_failed;
    }
    memset(p_ptr, 0, window_height * parent_stride);
    if ( screen_post_window(screen_app, screen_pbuf, 1, rect,0) )
    {
        printf("screen_post_window(screen_parent_win) failed\n");
        goto screen_failed;
    }
    snprintf(str_group, sizeof(str_group), "rearview-camera-parent");
    strcat(str_group,disp);
    screen_create_window_group(screen_app, str_group);
    screen_join_window_group(screen_video, str_group);
    ppsdata.screen_video = screen_video;
    ppsdata.ctx = ctx;
    ppsdata.screen_app = screen_app;
    ppsdata.dst_height = dst_height;
    initialize_pps((void*)&ppsdata);
    while(app_abort)
    {
		if(quit_if_no_video) {
			char *cur_norm = NULL;
			if(capture_is_property(ctx, CAPTURE_PROPERTY_CURRENT_NORM)) {
				capture_get_property_p(ctx, CAPTURE_PROPERTY_CURRENT_NORM, (void **)&cur_norm);
				if(cur_norm && (strcmpi(cur_norm, CAPTURE_NORM_NONE) == 0)) {
					sleep(1);
					// It takes decoders some time to detect whether the input video exists or not.
					// Check the input status again after 1 second, to make sure the video input is really not detected.
					capture_get_property_p(ctx, CAPTURE_PROPERTY_CURRENT_NORM, (void **)&cur_norm);
					if(strcmpi(cur_norm, CAPTURE_NORM_NONE) == 0) {
						fprintf(stderr, "no video input.\n");
						goto failed;
					}
				}
			}
		}
		
        index = capture_get_frame(ctx, 100000000, CAPTURE_FLAG_LATEST_FRAME);
        if(index != -1)
        {
            screen_post_window(screen_video, screen_buf[index], 1, rect_buf , 0);
            capture_release_frame(ctx, index);
        }
    }
    capture_destroy_context(ctx);
    if(screen_video)
        screen_destroy_window(screen_video);
    if(screen_app)	
        screen_destroy_window(screen_app);
    screen_destroy_context(screen_ctx);
    return 1;
failed:
    capture_destroy_context(ctx);
screen_failed:
    if(screen_video)
        screen_destroy_window(screen_video);
    if(screen_app)	
        screen_destroy_window(screen_app);	
    screen_destroy_context(screen_ctx);		
    return 1;
}