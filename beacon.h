/*
 * beacon.h
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
 * $Id: beacon.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup beacon <beacon.h>: Beacon
 * \code #include <beacon.h> \endcode
 * <h3>Introduction to the Beacon Module</h3>
 *  
 * This defines the beaconing subsystem.
 */

#ifndef __BEACON_H_
#define __BEACON_H_

#include "atom.h"
#include "ax25.h"
#include "aprs.h"
#include "dht11.h"
#include "thermistor.h"
#include "time.h"
#include <stdio.h>

/** \ingroup beacon */
/* @{ */
#define BEACON_TABLE_SIZE   8   ///< Size of beacon table.
#define BEACON_FLAG_USED    (uint8_t) (1 << 7)  ///< Beacon is allocated.
#define BEACON_FLAG_E       (uint8_t) (1 << 6)  ///< Beacon is valid.
#define BEACON_FLAG_ONCE    (uint8_t) (1 << 5)  ///< Beacon is transmitted once.
#define BEACON_TYPE_BEACON  0
#define BEACON_TYPE_POSIT   1
#define BEACON_TYPE_WX      2
#define BEACON_MIN_SEP      15  ///< Minimum beacon separation in seconds.
#define BEACON_ERROR_SLOT   (-1)

typedef struct Beacon {
    uint8_t  flags;     ///< Beacon flags.
    uint8_t  type;      ///< Beacon type.
    uint16_t interval;  ///< Transmit frequency, in seconds.
    uint16_t offset;    ///< Transmit offset, in seconds.
    char     *str;      ///< Static comment text.
    time_t   timeout;   ///< Timeout to transmit.
} Beacon;

void beaconInit (void);
void beaconProcess (uint8_t const *buf);
void beaconReset (void);
int8_t beaconSet (uint16_t seconds, uint16_t offset, uint8_t type, char *str);

/* @} */
#endif /* __BEACON_H_ */
