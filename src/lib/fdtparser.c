/*
 *
 * Copyright (C) 2022 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#ifdef ENABLE_FDT
#ifdef ENABLE_UEFI
#include<comp_libfdt.h>
#else
#include<stdbool.h>
#include<stdint.h>
#include<libfdt.h>
#endif
#include"param.h"
#include"fdtparser.h"

const uint32_t fdt_magic=0xedfe0dd0;

fdt get_fdt_from_pointer(void*fdt){
	if(!fdt)return NULL;
	int r=fdt_check_header(fdt);
	if(r!=0)return NULL;
	return fdt;
}

int32_t fdt_get_address_cells(fdt*fdt){
	const int32_t*prop;
	int32_t length=0,ret=1;
	prop=fdt_getprop(fdt,0,"#address-cells",&length);
	if(length==4)ret=fdt32_to_cpu(*prop);
	return ret;
}

int32_t fdt_get_size_cells(fdt*fdt){
	const int32_t*prop;
	int32_t length=0,ret=1;
	prop=fdt_getprop(fdt,0,"#size-cells",&length);
	if(length==4)ret=fdt32_to_cpu(*prop);
	return ret;
}

bool fdt_get_memory(fdt*fdt,int index,uint64_t*base,uint64_t*size){
	const int32_t*prop;
	int32_t ac,sc,node,item,len=0;
	if(!fdt||!base||!size)return false;

	// load address cells and size cells
	ac=fdt_get_address_cells(fdt);
	sc=fdt_get_size_cells(fdt);
	*base=0,*size=0;

	// fetch memory node
	node=fdt_path_offset(fdt,"/memory");
	prop=fdt_getprop(fdt,node,"reg",&len);
	item=(ac+sc)*sizeof(int32_t);
	if(len<item){
		node=-1;
		return false;
	}

	// in range
	if(index>=len/item)return false;
	prop+=(ac+sc)*index;

	// calc memory base
	*base=fdt32_to_cpu(prop[0]);
	if(ac>1)*base=((*base)<<32)|fdt32_to_cpu(prop[1]);
	prop+=ac;

	// calc memory size
	*size=fdt32_to_cpu(prop[0]);
	if(sc>1)*size=((*size)<<32)|fdt32_to_cpu(prop[1]);
	prop+=sc;

	return true;
}

int fdt_get_cmdline(fdt*fdt,char**cmdline,int*length){
	int node;
	if(!fdt||!cmdline||!length)ERET(EINVAL);
	node=fdt_path_offset(fdt,"/chosen");
	if(node<0)ERET(ENOENT);
	*cmdline=(char*)fdt_getprop(fdt,node,"bootargs",length);
	if(!*cmdline||*length<=0)ERET(ENOENT);
	return 0;
}

keyval**fdt_get_cmdline_items(fdt*fdt,size_t*length){
	int len=0;
	char*cmdline=NULL;
	if(fdt_get_cmdline(fdt,&cmdline,&len)!=0)return NULL;
	if(len<=0||!cmdline)return NULL;
	return param_s_parse_items(cmdline,(size_t)len,length);
}
#endif
