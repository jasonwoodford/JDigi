/*
 * kiss.h
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
 * $Id: kiss.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup kiss <kiss.h>: KISS
 * \code #include <kiss.h> \endcode
 * <h3>Introduction to the KISS Module</h3>
 *  
 * This defines KISS functions for applications.
 */

#ifndef __KISS_H__
#define __KISS_H__

#include <stdio.h>

/** \ingroup kiss */
/* @{ */
#define KISS_FEND   (uint8_t) 0xC0    // Frame End
#define KISS_FESC   (uint8_t) 0xDB    // Frame Escape
#define KISS_TFEND  (uint8_t) 0xDC    // Transposed Frame End
#define KISS_TFESC  (uint8_t) 0xDD    // Transposed Frame Escape

#define KISS_FLAG_FEND  (uint8_t) (1)
#define KISS_FLAG_FESC  (uint8_t) (1 << 1)
#define KISS_FLAG_CMD   (uint8_t) (1 << 2)
#define KISS_FLAG_VALID (uint8_t) (1 << 3)
#define KISS_FLAG_DONE  (uint8_t) (1 << 4)

#define KISS_CMD_DATA       (uint8_t) 0
#define KISS_CMD_TXDELAY    (uint8_t) 1
#define KISS_CMD_PERSIST    (uint8_t) 2
#define KISS_CMD_SLOTTIME   (uint8_t) 3
#define KISS_CMD_TXTAIL     (uint8_t) 4
#define KISS_CMD_FULLDUP    (uint8_t) 5
#define KISS_CMD_SETHW      (uint8_t) 6

#define KISS_ERROR_BADRX    (-1)
#define KISS_ERROR_BADFRAME (-2)
#define KISS_ERROR_BUFOVFL  (-3)

int16_t  kissGet (uint8_t *buf, uint16_t size, FILE *stream);
uint16_t kissPut (uint8_t *buf, uint16_t size, uint8_t porttype, FILE *stream);

/* @} */
#endif /* __KISS_H__ */
