/*
 * ax25.h
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
 * $Id: ax25.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup ax25 <ax25.h>: AX25
 * \code #include <ax25.h> \endcode
 * <h3>Introduction to the AX25 Module</h3>
 *  
 * This defines a complete AX25 network stack for applications.
 */

#ifndef __AX25_H__
#define __AX25_H__

#include "atom.h"
#include "atommutex.h"
#include "crc.h"
#include <stdio.h>

/** \ingroup ax25 */
/* @{ */
#define AX25_QUEUEFLAG_USED (1 << 7)  ///< Frame is allocated.
#define AX25_QUEUEFLAG_MOVE (1 << 6)  ///< Frame info buffer is being moved.
#define AX25_QUEUEFLAG_E    (1 << 5)  ///< Frame is valid.
#define AX25_QUEUEFLAG_I    (1 << 4)  ///< Frame in INPUT chain.
#define AX25_QUEUEFLAG_F    (1 << 3)  ///< Frame in FORWARD chain.
#define AX25_QUEUEFLAG_T    (1 << 2)  ///< Frame in TRANSMIT chain.
#define AX25_QUEUEFLAG_Z    (1 << 1)  ///< Frame in DEQUEUE chain.
#define AX25_ERROR_DSTADDR  (-1)    ///< Bad destination address.
#define AX25_ERROR_SRCADDR  (-2)    ///< Bad source address.
#define AX25_ERROR_OVADDR   (-3)    ///< Too many via paths.
#define AX25_ERROR_VIAADDR  (-4)    ///< Bad VIA address.
#define AX25_ERROR_INFOBUF  (-5)    ///< Cannot allocate enough info memory.
#define AX25_ERROR_NOSLOTS  (-6)    ///< Cannot allocate frame.
#define AX25_ERROR_MUTEX    (-7)    ///< Cannot create frame mutex.

/** Maximum of 127 slots, though memory will lower this limit much more.
 * Each frame slot consumes 87 bytes of RAM, so be conservative!
 * The info buffer is dynamically allocated, so treat the average size
 * as a "worst case" maximum for all frames.
 */
#define AX25_FRAME_SLOTS        8   ///< Frame table size.
#define AX25_FRAME_AVGINFOSIZE  80  ///< Frame size ("worst case" maximum average).
#define AX25_FRAME_INFOBUFSIZE  (AX25_FRAME_SLOTS * AX25_FRAME_AVGINFOSIZE)
#define AX25_FRAME_TICKS        (SYSTEM_TICKS_PER_SEC / 10) ///< 0.1 seconds.
#define AX25_FRAME_LIFETIME     250 ///< Frame TTL, in 0.1 seconds.
#define AX25_FRAME_LIFETIME_TX  20  ///< Frame TTL, in 0.1 seconds.

#define AX25_ADDR_SIZE  7

typedef struct AX25Frame {
    uint8_t  addr[10][AX25_ADDR_SIZE];  ///< Frame address table.
    uint8_t  hops;      ///< Number of vias.
    uint8_t  ctrl;      ///< Control byte.
    uint8_t  pid;       ///< Protocol ID.
    char     *info;     ///< Info pointer.
    uint16_t infosize;  ///< Info size.
    uint16_t infocrc;   ///< Info CRC.
    uint8_t  cmdresp;   ///< 1=cmd, 2=resp (v2.0); 0=undef (pre v2.0)
    volatile uint8_t queueflag; ///< Frame queue flags.
    volatile uint8_t lifetime;  ///< Frame lifetime, in 0.1 seconds.
    FILE     *stream;   ///< Frame source/destination stream.
} AX25Frame;  // 87 bytes per frame

typedef struct AX25FrameInfo {
    ATOM_MUTEX mutex;   ///< Global AX25 frame mutex.
    uint16_t free;      ///< Bytes left free in the info buffer.
    volatile uint8_t ticks; ///< Timer ticks.
    char info[AX25_FRAME_INFOBUFSIZE]; ///< AX25 frame info array.
} AX25FrameInfo;

int8_t  ax25InitQueue (void);
int8_t  ax25Alloc (void);
int16_t ax25Decode (uint8_t slot, uint8_t *buf, uint16_t len);
int16_t ax25Encode (uint8_t slot, uint8_t *buf);
uint8_t ax25GetAddr (uint8_t *buf, uint8_t *addr);
uint8_t ax25CheckAddrc (uint8_t c);
uint8_t ax25FmtAddr (char *buf, uint8_t *addr);
uint8_t ax25CmpAddr (char *buf, uint8_t *addr);
uint8_t ax25SetAddr (char *buf, uint8_t ssid, uint8_t *addr);
int16_t ax25SetInfo (uint8_t slot, char *buf, uint16_t len);
int16_t ax25GetInfo (uint8_t slot, char *buf, uint16_t len);
void    ax25FreeInfo (uint8_t slot);
void    ax25Free (uint8_t slot);
void    ax25Tick (void);

/* @} */
#endif /* __AX25_H__ */
