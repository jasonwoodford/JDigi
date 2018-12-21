/*
 * ds18b20.h
 * 
 * Copyright (c) 2017, Jason Woodford, VO1JWW. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. No personal names or organizations' names associated with the
 *    Atomthreads project may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JDIGI PROJECT AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ds18b20.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup ds18b20 <ds18b20.h>: DS18B20
 * \code #include <ds18b20.h> \endcode
 * <h3>Introduction to the DS18B20 Module</h3>
 *  
 * This defines DS18B20 functions for applications.
 */

#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "atom.h"
#include "onewire.h"
#include <stdio.h>

/** \ingroup ds18b20 */
/* @{ */
#define DS18B20_MAX_DEV     4   ///< Configurable.

#define DS18B20_ERROR_DEV   (-16)
#define DS18B20_ERROR_FAM   (-17)
#define DS18B20_FAMILY      0x28
#define DS18B20_FLAG_VALID  (1 << 7)
#define DS18B20_ROMCMD_CONV 0x44
#define DS18B20_ROMCMD_WSP  0x4E
#define DS18B20_ROMCMD_RSP  0xBE
#define DS18B20_ROMCMD_CSP  0x48
#define DS18B20_ROMCMD_REE  0xB8
#define DS18B20_ROMCMD_RPS  0xB4
#define DS18B20_UNIT_C      0
#define DS18B20_UNIT_F      1
#define DS18B20_UNIT_K      2

typedef struct DS18B20 {
    uint8_t  flags;
    uint8_t  onewireIndex;
    uint8_t  sp[9];
} DS18B20;

uint8_t ds18b20Init (void);
int8_t  ds18b20GetIndex (uint8_t owindex);
int8_t  ds18b20GetTemp (char *devid);
double  ds18b20ConvTemp (char *devid, uint8_t unit);
char*   ds18b20PrintError (char *buf, int8_t errno);

/* @} */
#endif /* __DS18B20_H__ */
