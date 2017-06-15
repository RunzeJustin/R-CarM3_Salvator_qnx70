/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <devctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/slogcodes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <hw/i2c.h>

#define MAX_DAT_NUMBER 256
#define MAX_SUBADDR_LEN 2
#define BUS_SPEED_MIN	50000
#define BUS_SPEED_MAX	400000

typedef struct
{  
    char            *device;
    unsigned char   slave;
    unsigned char   data[MAX_DAT_NUMBER];
	unsigned char   subaddr[MAX_SUBADDR_LEN];
    int 			read_count;
	int 			data_len;
	int 			subaddr_len;
    char            *mode;
    int 			speed;
    int 	        view;
}i2c_control_info;

int Parse_commandline (i2c_control_info *str_info, char *args[])
{
    int i=0;
    int opt = 0;
    char    *value;
    char    *opts[] = {"device","slave","count","data","mode","speed","-v","subaddr", NULL};

    str_info->device = 0;
    str_info->slave = 0;
    str_info->read_count = 1;
	str_info->data_len = 0;
	str_info->subaddr_len = 0;
    str_info->mode = "write";
    str_info->data[0]= 0;
	str_info->subaddr[0]= 0;
    str_info->speed = BUS_SPEED_MIN;
    str_info->view = 0;
	
    /* Sets options to each values*/
    while (args[i] != NULL && args[0] != 0)
    {

        switch ((opt = getsubopt (&args[i], opts, &value)))
        {
        case 0: //device
            if (value != NULL)
            {
                str_info->device = value;				
            }
            break;
        case 1: //slave
            if (value != NULL)
            {
                 str_info->slave = strtoul(value, NULL, 16);				
            }
           break;
        case 2: //count
            if (value != NULL)
            {
                str_info->read_count = atoi(value);
				if (str_info->read_count > MAX_DAT_NUMBER)
				{
					printf ("Read count exceeds %d\n", MAX_DAT_NUMBER);
					return -1;
				}					
            }
            break;
        case 3: //data
            if (value != NULL)
            {
				char *substr, *str;
				str = value;
				int idx=0;
				while (1)
				{	
					if (substr = strstr (str, ":"))
						*substr = '\0';
					 str_info->data[idx++] = strtoul(str, NULL, 16);
					/* reach end of string */
					if (idx >= MAX_DAT_NUMBER || !substr) break;
					str = substr + 1;
				}
				str_info->data_len = idx;
				if (str_info->data_len > MAX_DAT_NUMBER)
				{
					printf ("Data length exceeds %d\n", MAX_DAT_NUMBER);
					return -1;
				}				
            }
            break;
        case 4: //mode
        	if (value != NULL)
            {
                str_info->mode = value;
            }            
            break;
        case 5:
            if (value != NULL)
            {
                str_info->speed = (atoi(value) < BUS_SPEED_MIN) ? BUS_SPEED_MIN : atoi(value);
				str_info->speed = (str_info->speed > BUS_SPEED_MAX) ? BUS_SPEED_MAX : str_info->speed;
            }            
            break;
        case 6:
     	    str_info->view = 1;            
            break;
        case 7:	//sub address in read mode
            if (value != NULL)
            {
				char *substr, *str;
				str = value;
				int idx=0;
				while (1)
				{	
					if (substr = strstr (str, ":"))
						*substr = '\0';
					 str_info->subaddr[idx++] = strtoul(str, NULL, 16);
					/* reach end of string */
					if (idx >= MAX_DAT_NUMBER || !substr) break;
					str = substr + 1;
				}
				str_info->subaddr_len = idx;
				if (str_info->subaddr_len > MAX_SUBADDR_LEN)
				{
					printf ("Sub address exceeds %d\n", MAX_SUBADDR_LEN);
					return -1;
				}
            }
            break;			
        default:
            break;
        };

        i++;
    }
	
	return 0;
}

static int i2c_write (int fd, unsigned int slave_addr, const unsigned char *buffer, int bytes, int stop)
{
	typedef struct {
	    i2c_send_t header;
	    unsigned char buffer[16];
	} i2c_send_buffer;
   i2c_send_buffer send;
   int rc, retry = 10;

   //Removed code causing compiler warnings
   if (bytes > (int)sizeof(send.buffer))
   {
		printf ("write i2c: Send too large (%d bytes)\n", bytes);
		return -1;
   }

   send.header.slave.addr  = slave_addr;
   send.header.slave.fmt   = I2C_ADDRFMT_7BIT;;
   send.header.len         = bytes;
   send.header.stop        = stop;            /* send stop when complete? (0=no, 1=yes) */

   memcpy(send.buffer, buffer, bytes);
	
   do
   {
      if ( (rc = devctl(fd, DCMD_I2C_SEND, &send, sizeof(send), NULL)) != 0)
	  {
			delay(5);	// 5 milli seconds delay in each loop
	  }
    } while (retry-- && (rc != EOK));
	
    return rc;
}

static int i2c_read( int fd, unsigned int slave_addr, unsigned char *buffer, int bytes)
{
	typedef struct i2c_des_msg_get
	{
		i2c_recv_t hdr;
		unsigned char data[16];
	} i2c_receive_buffer;
	
	i2c_receive_buffer msg;
	int rc, retry = 10;

	if (fd < 0)
		return -1;

	msg.hdr.slave.addr = slave_addr;
	msg.hdr.slave.fmt  = I2C_ADDRFMT_7BIT;
	msg.hdr.len        = bytes;
	msg.hdr.stop       = 1;

	do
	{
		if( (rc=devctl (fd, DCMD_I2C_RECV, &msg, sizeof(msg.hdr)+bytes, NULL)) != 0 )
        {
            delay(5);	// 5 milli seconds delay in each loop
        }
	} while (retry-- && (rc != 0));

	memcpy(buffer,msg.data,bytes);

	return rc;
}

int
main(int argc, char *argv[])
{
    int     fd, i;

    i2c_control_info i2c_info;    

    if (-1 == Parse_commandline(&i2c_info, argv))
	{
		return -1;
	}	
	
    //Print input information
    if (i2c_info.view == 1)
        printf("device = %s, slave_add = 0x%x, speed = %d, mode = %s\n",
                i2c_info.device,
                i2c_info.slave,
                i2c_info.speed,
                i2c_info.mode);
    //check channel condition
    if(i2c_info.device == NULL)
    {
        printf("Device is not specified\n");
        exit(0);
    }
    //check slave addr condition
    if(i2c_info.slave == 0)
    {
        printf("Slave address is not specified\n");
        exit(0);
    }

    //check mode condition
    if(strcmp(i2c_info.mode, "write") && strcmp(i2c_info.mode, "read"))
    {
        printf("Mode does not accept ""%s"". ""read"" or ""write"" is accepted \n", i2c_info.mode);
        exit(0);
    }

    //Connect to the I2C driver
    fd = open(i2c_info.device, O_RDWR);
    if (fd < 0) 
    {
		perror ("i2c open");
        exit(-1);
    }

    //Set the bus speed for this connection
    if (i2c_info.view == 1)
        printf("Set bus speed to %uHz\n",  i2c_info.speed);
	
    if (EOK != devctl(fd, DCMD_I2C_SET_BUS_SPEED, &i2c_info.speed, 
            sizeof(i2c_info.speed), NULL))
    {
		perror ("i2c set bus speed");
		close (fd);
        exit(-1);
    }   

    if (strcmp(i2c_info.mode, "write") == 0)//write
    {
		int total_len = i2c_info.data_len + i2c_info.subaddr_len;
		char *write_buffer;
		if (!(write_buffer = (char *)malloc (total_len)))
		{
			perror("malloc");
			close(fd);
			return -1;
		}
		
		if (i2c_info.subaddr_len)
			memcpy (write_buffer, i2c_info.subaddr, i2c_info.subaddr_len);
		if (i2c_info.data_len)
			memcpy (write_buffer + i2c_info.subaddr_len, i2c_info.data, i2c_info.data_len);
		
        if (i2c_info.view == 1)
            printf("Send %d bytes of data\n", i2c_info.data_len);

		if (!i2c_info.data_len)
		{
			printf("There no data specified\n");
			close(fd);
			return -1;
		}
		
		if (EOK != i2c_write (fd, i2c_info.slave, write_buffer, total_len, 1))
		{
			printf ("i2c-control: Send I2C error\n");
			close (fd);
			return -1;
		}
		
        // Output sent data
        if (i2c_info.view == 1){ 
            printf("   Data sent:");
                
            for (i = 0; i < total_len; i++)
            {
                printf(" %xh", write_buffer[i]);
            }
            printf("\n");
        }
		
		free (write_buffer);
    }
    else if (strcmp(i2c_info.mode, "read") == 0)//read
    {
		if (i2c_info.subaddr_len)
		{
			/* if sub address is specified */
			if (EOK != i2c_write (fd, i2c_info.slave, i2c_info.subaddr, i2c_info.subaddr_len, 0))
			{
				printf ("i2c-control: Send I2C error\n");
				close (fd);
				return -1;
			}
		}
		if (i2c_info.read_count)
		{		
			if (EOK != i2c_read (fd, i2c_info.slave, i2c_info.data, i2c_info.read_count))
			{
				printf ("i2c-control: Receive I2C error\n");
				close (fd);
				return -1;
			}
			// Output received data
			printf("   Data received:");
			for (i = 0; i < i2c_info.read_count; i++)
			{
				printf(" %xh", i2c_info.data[i]);
			}
			printf("\n");
		}
		else
		{
			printf ("No read count is specified\n");
			close (fd);
			return -1;
		}
    }
	
    close(fd);
    exit(0);
}
