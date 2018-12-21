/*
 * thermistor.h
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
 * $Id: thermistor.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup thermistor <thermistor.h>: Thermistor
 * \code #include <thermistor.h> \endcode
 * <h3>Introduction to the Thermistor Module</h3>
 *  
 * This module is based on the typical "Radio Shack" (part# 271-110A)
 * thermistor. It has a typical resistance of 10k ohms at 25C, but it does not
 * have a linear response when biased with a resistor. This module corrects
 * this non-linearity using a lookup table and two-point interpolation to 
 * adjust the returned ADC value to fit the linear Celcuis and Fahrenheit
 * translation functions.
 * 
 * A typical circuit would be a simple voltage divider of the thermistor and
 * a 33k ohm resistor - the thermistor is tied to Vcc and the resistor to
 * ground. The junction between the two is tied to an ADC input.
 */

#ifndef __THERMISTOR_H_
#define __THERMISTOR_H_

#include "atom.h"
#include "adc.h"
#include "math.h"

/** \ingroup thermistor */
/* @{ */
#define THERMISTOR_PIN              7

#define THERMISTOR_NOMINAL_T        298.15  ///< 25.00C = 298.15K
#define THERMISTOR_NOMINAL_RT       10000.0 ///< 10k ohm
#define THERMISTOR_NOMINAL_RS       33000.0 ///< 33k ohm
#define THERMISTOR_B_COEFFICIENT    3950.0  ///< originally 3950.0

#define THERMISTOR_OPT_FAHRENHEIT   (1 << 7)

#define THERMISTOR_COMPTABLE_SIZE   33
#define THERMISTOR_CONV_CSLOPE      9.9f
#define THERMISTOR_CONV_CINT        560
#define THERMISTOR_CONV_FSLOPE      5.5f
#define THERMISTOR_CONV_FINT        384

uint16_t thermistorRead (uint8_t opts);
double   thermistorGetTempL (uint8_t opts);
double   thermistorGetTempS (uint8_t opts);

/* @} */
#endif /* __THERMISTOR_H_ */
