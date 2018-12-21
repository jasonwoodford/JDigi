/*
 * dht11.h
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
 * $Id: dht11.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup dht11 <dht11.h>: DHT11
 * \code #include <dht11.h> \endcode
 * <h3>Introduction to the DHT11 Module</h3>
 *  
 * This defines DHT11 functions for applications.
 */

#ifndef __DHT11_H__
#define __DHT11_H__

#include "atom.h"

/** \ingroup dht11 */
/* @{ */
#define DHT11_DATA_SIZE     5   ///< in bytes.

#define DHT11_ERROR_BUS     -1
#define DHT11_ERROR_NORESP  -2
#define DHT11_ERROR_RESP    -3
#define DHT11_ERROR_NODATA  -4
#define DHT11_ERROR_DATA    -5

typedef struct DHT11 {
    uint8_t raw[DHT11_DATA_SIZE];
    uint8_t humd;
    int16_t temp;
} DHT11;

/* Architecture dependent prototypes. */

/** Architecture dependent DHT11 initialization.
 * 
 */
void dht11Init (void);

/** Architecture dependent DHT11 device read.
 * Returns the number of bytes read, or an error.
 */
int8_t dht11Read (void);

/* @} */
#endif /* __DHT11_H__ */
