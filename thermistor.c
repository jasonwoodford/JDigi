/*
 * thermistor.c
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
 * $Id: thermistor.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "thermistor.h"

static uint16_t const thermistorADCTbl[THERMISTOR_COMPTABLE_SIZE] PROGMEM = {
     93, 120, 152, 190, 233, 282, 334, 390, 447, 504, 559, 612, 662, 707, 
    748, 785, 817, 845, 869, 890, 908, 924, 937, 948, 958, 966, 974, 980, 
    982, 990, 994, 997, 1000};
static int16_t const thermistorDeltaTbl[THERMISTOR_COMPTABLE_SIZE] PROGMEM = {
    -28,  -6,  12,  23,  30,  30,  28,  21,  14,   7,   1,  -3,  -3,   1,
     10,  23,  40,  62,  87, 115, 147, 181, 217, 255, 295, 336, 378, 422,
    469, 511, 556, 602, 649};

/**
 * 
 */
uint16_t thermistorRead (uint8_t opts) {
    uint8_t index;
    int16_t val, i, j, k, l;
    //
    val = adcRead(opts);
    for (index = 0; index < THERMISTOR_COMPTABLE_SIZE; index++) {
        // Scan the CompTable for an interval.
        if (val < pgm_read_word(&thermistorADCTbl[index])) break;
    }
    if ((index > 0) && (index < THERMISTOR_COMPTABLE_SIZE)) {
        // Within the comp table.
        i = pgm_read_word(&thermistorADCTbl[index-1]);
        k = pgm_read_word(&thermistorADCTbl[index]) - i;
        l = pgm_read_word(&thermistorDeltaTbl[index-1]);
        j = pgm_read_word(&thermistorDeltaTbl[index]) - l;
        return val + (((val - i) * j) / k) + l;
    }
    return val;
}

/** Gets the temperature via thermistorRead() and converts it using a linear
 * interpolated correction table then a linear algorithm.
 * 
 * Will convert to either Celcuis or Fahrenheit depending on opt flag.
 * Returns a signed double floating point value.
 */
double thermistorGetTempL (uint8_t opts) {
    if (opts & THERMISTOR_OPT_FAHRENHEIT) {
        // Fahrenheit.
        return ((double) (thermistorRead(opts) - THERMISTOR_CONV_FINT) / THERMISTOR_CONV_FSLOPE);
    } else {
        // Celsius.
        return ((double) (thermistorRead(opts) - THERMISTOR_CONV_CINT) / THERMISTOR_CONV_CSLOPE);
    }
}

/** Gets the temperature via thermistorRead() and converts it using a the
 * Steinhart algorithm.
 * 
 * Will convert to either Celcuis or Fahrenheit depending on opt flag.
 * Returns a signed double floating point value.
 */
double thermistorGetTempS (uint8_t opts) {
    double  temp;
    // convert the thermistor reading to a resistance.
    temp = (((float) (ADC_INPUT_RANGE - 1) / (float) adcRead(opts)) - 1.0) * THERMISTOR_NOMINAL_RS;
    temp /= THERMISTOR_NOMINAL_RT;      // (R/Ro)
    temp = log(temp);                   // ln(R/Ro)
    temp /= THERMISTOR_B_COEFFICIENT;   // 1/b * ln(R/Ro)
    temp += 1.0 / THERMISTOR_NOMINAL_T; // 1/b * ln(R/Ro) + (1/To)
    temp = 1.0 / temp;                  // Invert
    temp -= 273.15;                     // Convert to C
    if (opts & THERMISTOR_OPT_FAHRENHEIT) temp = ((9.0 * temp) / 5.0) + 32.0;
    return temp;
}
