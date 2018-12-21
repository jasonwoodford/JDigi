/*
 * lcdshield.h
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
 * $Id: lcdshield.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup lcdshield <lcdshield.h>: LCDshield
 * \code #include <lcdshield.h> \endcode
 * <h3>Introduction to the LCDshield Module</h3>
 *  
 * These are the definitions for JDigi's LCDshield module.
 */

#ifndef __LCDSHIELD_H
#define __LCDSHIELD_H

#include "atom.h"
#include "atommutex.h"
#include "adc.h"
#include "aprs.h"
#include "hd44780.h"

/** \ingroup lcdshield */
/* @{ */
#define LCDSHIELD_BTN_PIN   0
#define LCDSHIELD_BTN_SEL   5
#define LCDSHIELD_BTN_LEFT  4
#define LCDSHIELD_BTN_DOWN  3
#define LCDSHIELD_BTN_UP    2
#define LCDSHIELD_BTN_RIGHT 1
#define LCDSHIELD_BTN_SEL_A     719
#define LCDSHIELD_BTN_LEFT_A    478
#define LCDSHIELD_BTN_DOWN_A    305
#define LCDSHIELD_BTN_UP_A      130
#define LCDSHIELD_BTN_RIGHT_A   0
#define LCDSHIELD_BTN_DEV_A     5       ///< Allowable deviation of button values.

/* Read-only macro during LCD writes. */
#define lcdshieldOutcmd(b)      lcdshieldOutbyte(b, 0)
#define lcdshieldOutdata(b)     lcdshieldOutbyte(b, 1)

void    lcdshieldBacklightToggle (void);
uint8_t lcdshieldButtonRead (void);

/* Architecture dependent function prototypes. */
void    lcdshieldInit (void);
void    lcdshieldOutnibble (uint8_t n, uint8_t rs);
void    lcdshieldOutbyte (uint8_t b, uint8_t rs);
void    lcdshieldBacklightSet (uint8_t state);

/* @} */
#endif /* __LCDSHIELD_H */
