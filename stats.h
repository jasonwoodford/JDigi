/*
 * stats.h
 * 
 * Copyright (C)2016 Jason Woodford. All rights reserved.
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
 * $Id: stats.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup stats <stats.h>: Stats
 * \code #include <stats.h> \endcode
 * <h3>Introduction to the Stats Module</h3>
 *  
 * This defines APRS stat functions for applications.
 */

#ifndef __STATS_H__
#define __STATS_H__

#include "atom.h"
#include "aprs.h"
#include "ax25.h"
#include "time.h"
#include <stdio.h>
#include <string.h>

/** \ingroup stats */
/* @{ */
#define STATS_SLOTS 12      ///< Maximum of 127.
#define STATS_HOURS 8       ///< Maximum of 8.

#define STATS_FLAG_USED     (uint8_t) (1 << 7)
#define STATS_FLAG_E        (uint8_t) (1 << 6)

#define STATS_ERROR_NOSLOTS (-1)

typedef struct Stats {
    uint8_t     callsign[7];
    uint8_t     flags;
    uint8_t     hourlyDirect[STATS_HOURS];
    uint8_t     hourlyHeard[STATS_HOURS];
    time_t      hourlyUpdate;
    time_t      lastHeard;
} Stats;

void    statsInit (void);
void    statsRefresh (void);
void    statsUpdate (uint8_t slot, char *buf);
int8_t  statsGetSlot (uint8_t *addr);
uint8_t statsGetNextSlot (uint8_t ss);

/* @} */
#endif /* __STATS_H__ */
