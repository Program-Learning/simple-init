/*
 *
 * Copyright (C) 2021 BigfootACA <bigfoot@classfun.cn>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#ifndef _COMPATIBLE_H
#define _COMPATIBLE_H
#include<sys/types.h>

#ifdef ENABLE_UEFI
extern int efi_status_to_errno(EFI_STATUS st);
#endif

#endif
