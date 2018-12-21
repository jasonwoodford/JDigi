/*
 * adc.h
 * 
 * Copyright (C)2018 Jason Woodford. All rights reserved.
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
 * $Id: adc.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup adc <adc.h>: ADC
 * \code #include <adc.h> \endcode
 * <h3>Introduction to the ADC Module</h3>
 *  
 * Each architecture needs to provide it's own dependent functions, but
 * this is the core part of JDigi's ADC mechanism.
 */

#ifndef __ADC_H__
#define __ADC_H__

#include "atom.h"

/** \ingroup adc */
/* @{ */
#define ADC_INPUT_RANGE 1024    ///< Range of 0 to 1023.

#define ADC_ERROR_INPUT (-1)    ///< Bad input pin number.

/* Architecture specific prototypes. */

/**
 * This is an architecture specific function that will initialize an ADC port.
 */
void    adcInit (void);

/**
 * This is an architecture specific function that will read a value from an
 * ADC input pin.
 */
int16_t adcRead (uint8_t pin);

/* @} */
#endif /* __ADC_H__ */
