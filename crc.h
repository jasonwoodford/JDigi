/*
 * crc.h
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
 * $Id: crc.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup crc <crc.h>: CRC
 * \code #include <crc.h> \endcode
 * <h3>Introduction to the CRC Module</h3>
 *  
 * This defines CRC functions for applications.
 */

#ifndef __CRC_H__
#define __CRC_H__

#include "atom.h"

/** \ingroup crc */
/* @{ */
uint16_t crcCalc_16 (uint8_t *buf, int16_t len);
int16_t  crcCheck_16 (uint8_t *buf, int16_t len);
uint16_t crcCalc_ccitt (uint16_t crc, uint8_t *buf, int16_t len);
int16_t  crcCheck_ccitt (uint8_t *buf, int16_t len);
uint16_t crcCalc_flex (uint8_t *buf, int16_t len);
int16_t  crcCheck_flex (uint8_t *buf, int16_t len);
uint8_t  crcCalc_8 (uint8_t *buf, uint8_t len);

/* @} */
#endif /* __CRC_H__ */
