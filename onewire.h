/*
 * onewire.h
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
 * $Id: onewire.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup onewire <onewire.h>: Onewire
 * \code #include <onewire.h> \endcode
 * <h3>Introduction to the Onewire Module</h3>
 *  
 * This defines Onewire functions for applications.
 */

#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include "atom.h"
#include <stdio.h>
#include <string.h>

/** \ingroup onewire */
/* @{ */
#define ONEWIRE_MAX_DEV         8
#define ONEWIRE_RESET_TIME      480 ///< in usec.
#define ONEWIRE_PRESENCE_TIME   480 ///< in usec.
#define ONEWIRE_RW_PULSETIME    6   ///< in usec.
#define ONEWIRE_RW_HOLDTIME     54  ///< in usec.
#define ONEWIRE_RW_RECTIME      10  ///< in usec.
#define ONEWIRE_RW_SAMPLETIME   9   ///< in usec.
#define ONEWIRE_ROMCMD_SEARCH   0xF0
#define ONEWIRE_ROMCMD_READ     0x33
#define ONEWIRE_ROMCMD_MATCH    0x55
#define ONEWIRE_ROMCMD_SKIP     0xCC
#define ONEWIRE_ROMCMD_ALARM    0xEC
#define ONEWIRE_FLAG_VALID      (1 << 7)
#define ONEWIRE_ERROR_BUS       (-1)
#define ONEWIRE_ERROR_NORESP    (-2)
#define ONEWIRE_ERROR_RESP      (-3)
#define ONEWIRE_ERROR_NODEV     (-4)
#define ONEWIRE_ERROR_NOID      (-5)

typedef struct ONEWIRE {
    uint8_t flags;
    uint8_t devcount;
} ONEWIRE;

typedef struct ONEWIREDev {
    uint8_t  flags;
    uint8_t  rom[8];
    char     id[11];
} ONEWIREDev;

void    onewireWriteByte (uint8_t val);
uint8_t onewireReadByte (void);
int8_t  onewireEnumerate (void);
int8_t  onewireGetIndex (char *str);
int8_t  onewireMatchRom (uint8_t index);
char*   onewirePrintError (char *buf, int8_t errno);

// Arch specific prototypes.
void    onewireInit (void);
int8_t  onewireReset (void);
void    onewireWrite (uint8_t val);
uint8_t onewireRead (void);

/* @} */
#endif /* __ONEWIRE_H__ */
