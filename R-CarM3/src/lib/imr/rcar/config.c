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

#define PARSE_ERROR(line) { \
	fprintf(stderr, "Invalid imrlx4.conf: line %d \r\n", line); \
	return -1; \
}


int parse_config (rcar_imr_t *imr, char *opt, int line)
{
	char *str, *str1, *str2, *substr;
	static int opt_setup_begin = 0;
	static int opt_coord_begin = 0;
	static int offs = 0;
	static int numVertex = 0;

	if((str = strstr (opt, "begin"))){
		str1 = str;
		if((str = strstr(str1, "setup")))
			opt_setup_begin = 1;
		else if((str = strstr(str1, "coordinates")))
			opt_coord_begin = 1;
		return 0;
	}

	if(opt_setup_begin && (str = strstr(opt, "end"))){
		str1 = str;
		if((str = strstr(str1, "setup")))
		{
			opt_setup_begin = 0;
			imr->img_conf.coords = calloc(1, numVertex * 4);
		}
		return 0;
	}
	if(opt_setup_begin && (str = strstr(opt, "square-size"))){
		str1 = str;
		if((str = strstr(str1, "="))){
			str2 = str;
			str2++;
			if((substr = strstr(str2, "x")))
				*substr = '\0';
			else PARSE_ERROR(line);
			imr->img_conf.squareW = atoi(str2);
			str2 = substr + 1;
			imr->img_conf.squareH = atoi(str2);
			//One square (squareW x squareH) will be divided into two triangles (each has 3 vertexes)
			numVertex = (imr->img.dw / imr->img_conf.squareW) *
						(imr->img.dh / imr->img_conf.squareH) *
						 2 * 3;
			if (!(numVertex>= 3 && numVertex <= 65535))
			{
				fprintf(stderr, "Invalid imrlx4.conf: Number of vertex %d is out of range (3 <= N <= 65535) \r\n", numVertex);
				return -1;
			}
		}
		else PARSE_ERROR(line);
		return 0;
	}
	if(opt_setup_begin && (str = strstr(opt, "Y"))){
		str1 = str;
		if((str = strstr(str1, "="))){
			str+= 2;
			if((str2 = strtok(str, " ")))
				imr->img_conf.Y.min = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.Y.max = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.Y.scal = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.Y.offs = atoi(str2);
			else PARSE_ERROR(line);
		}
		else PARSE_ERROR(line);
		return 0;
	}
	if(opt_setup_begin && (str = strstr(opt, "U"))){
		str1 = str;
		if((str = strstr(str1, "="))){
			str+= 2;
			if((str2 = strtok(str, " ")))
				imr->img_conf.U.min = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.U.max = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.U.scal = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.U.offs = atoi(str2);
			else PARSE_ERROR(line);
		}
		else PARSE_ERROR(line);
		return 0;
	}
	if(opt_setup_begin && (str = strstr(opt, "V"))){
		str1 = str;
		if((str = strstr(str1, "="))){
			str+= 2;
			if((str2 = strtok(str, " ")))
				imr->img_conf.V.min = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.V.max = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.V.scal = atoi(str2);
			else PARSE_ERROR(line);
			if((str2 = strtok(NULL, " ")))
				imr->img_conf.V.offs = atoi(str2);
			else PARSE_ERROR(line);
		}
		else PARSE_ERROR(line);
		return 0;
	}

	if(opt_coord_begin && (str = strstr(opt, "end"))){
		str1 = str;
		if((str = strstr (str1, "coordinates"))){
			opt_coord_begin = 0;
			if(numVertex != offs){
				fprintf(stderr, "Invalid imrlx4.conf: There are %d/%d vertexes were declared \r\n", offs, numVertex);
				return -1;
			}
		}
		return 0;
	}
	if(opt_coord_begin && (str = strstr(opt, "("))){
		uint32_t tmp = 0;
		if((str1 = strtok(str, " ,()")))
			tmp |= ((uint16_t)atoi(str1) << 16);
		else PARSE_ERROR(line);
		if((str1 = strtok(NULL, " ,()")))
			tmp |= ((uint16_t)atoi(str1));
		else PARSE_ERROR(line);
		*(imr->img_conf.coords + (offs++)) = tmp;
		if(offs > numVertex){
			fprintf(stderr, "Invalid imrlx4.conf: There are %d/%d vertexes were declared \r\n", offs, numVertex);
			return -1;
		}

		tmp = 0;
		if((str1 = strtok(NULL, " ,()")))
			tmp |= ((uint16_t)atoi(str1) << 16);
		else PARSE_ERROR(line);
		if((str1 = strtok(NULL, " ,()")))
		tmp |= ((uint16_t)atoi(str1));
		else PARSE_ERROR(line);
		*(imr->img_conf.coords + (offs++)) = tmp;

		tmp = 0;
		if((str1 = strtok(NULL, " ,()")))
			tmp |= ((uint16_t)atoi(str1) << 16);
		else PARSE_ERROR(line);
		if((str1 = strtok(NULL, " ,()")))
			tmp |= ((uint16_t)atoi(str1));
		else PARSE_ERROR(line);
		*(imr->img_conf.coords + (offs++)) = tmp;

		return 0;
	}

	return 0;
}

int get_config_data(rcar_imr_t *imr, const char *filename)
{
	char buf[256];
	char *opt, *c;
	int lineno = 0;

	FILE *fd = fopen(filename, "r");

    if(fd == NULL){
        /* No config file, use what we have */
    	fprintf(stderr, "Can't locate %s file", filename);
        return -1;
    }

    while(fgets(buf, sizeof (buf), fd) != NULL){
    	lineno++;
        c = buf;
        while((*c == ' ') || (*c == '\t'))
            c++;
        if((*c == '\015') || (*c == '\032') || (*c == '\0') || (*c == '\n') || (*c == '#'))
            continue;
        opt = c;
        while((*c == '\015') || (*c== '\032') || ((*c != '\0') && (*c != '\n') && (*c != '#')))
            c++;
        *c = '\0';
		if(parse_config(imr, opt, lineno) == -1)
		{
			fclose(fd);
			return -1;
		}
    }
    fclose(fd);
    return 0;
}

int parse_device_config(rcar_imr_t *imr)
{

    if(get_config_data(imr, "/etc/imrlx4.conf") == -1) 
		return -1;
	
	return 0;
}


