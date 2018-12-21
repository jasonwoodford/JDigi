/*
 * chains.h
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
 * $Id: chains.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup chains <chains.h>: Chains
 * \code #include <chains.h> \endcode
 * <h3>Introduction to the Chains Module</h3>
 *  
 * This module processes AX25 frames in a chained sequence.
 */

#ifndef __CHAINS_H_
#define __CHAINS_H_

#include "atom.h"
#include "atommutex.h"
#include "aprs.h"
#include <avr/pgmspace.h>

/** \ingroup chains */
/* @{ */
#define CHAINFLAGS_INPUT_DENY       (1 << 7)
#define CHAINFLAGS_FORWARD_DENY     (1 << 6)
#define CHAINFLAGS_OUTPUT_DENY      (1 << 5)
#define CHAIN_PRINTFRAME_E          (1 << 7)    ///< Enable frame messages.
#define CHAIN_PRINTFRAME_CRC        (1 << 2)    ///< Print the CRC.
#define CHAIN_PRINTFRAME_INFO       (1 << 1)    ///< Print the info.
#define CHAIN_PRINTFRAME_VIA        (1)         ///< Print the via paths.
#define CHAIN_PRINTFRAME_MASK       (CHAIN_PRINTFRAME_VIA | CHAIN_PRINTFRAME_INFO | CHAIN_PRINTFRAME_CRC)
#define CHAIN_REPEATFLAG_LEGACY     (1)
#define CHAIN_REPEATFLAG_WIDE1      (1 << 1)
#define CHAIN_REPEATFLAG_WIDE2      (1 << 2)
#define INPUT_SOURCE_ALLOW_SIZE     2
#define INPUT_SOURCE_REJECT_SIZE    0
#define INPUT_DEST_ALLOW_SIZE       26
#define INPUT_DEST_REJECT_SIZE      0

void chainInput (uint8_t slot);
void chainForward (uint8_t slot);
void chainOutput (uint8_t slot, uint8_t *buf);
void chainDequeue (uint8_t slot);
void chainPrintFrame (FILE *stream, uint8_t slot, uint8_t opts);

/* @} */
#endif /* __CHAINS_H_ */
