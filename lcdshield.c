/*
 * lcdshield.c
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
 * $Id: lcdshield.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "lcdshield.h"

extern LCD lcd;

/** Backlight toggle.
 * 
 * Toggles the backlight on/off.
 */
void lcdshieldBacklightToggle (void) {
    if (lcd.flags & LCD_FLAGS_BACKLIGHT) {
        lcdshieldBacklightSet(0);
    } else {
        lcdshieldBacklightSet(1);
    }
}

/** Return the 'translated' button value.
 */
uint8_t lcdshieldButtonRead (void) {
    int16_t val;
    
    val = adcRead(LCDSHIELD_BTN_PIN);
    // Evaluate to a specific button.
    if ((val <= LCDSHIELD_BTN_SEL_A + LCDSHIELD_BTN_DEV_A) && (val >= LCDSHIELD_BTN_SEL_A - LCDSHIELD_BTN_DEV_A)) return LCDSHIELD_BTN_SEL;
    if ((val <= LCDSHIELD_BTN_LEFT_A + LCDSHIELD_BTN_DEV_A) && (val >= LCDSHIELD_BTN_LEFT_A - LCDSHIELD_BTN_DEV_A)) return LCDSHIELD_BTN_LEFT;
    if ((val <= LCDSHIELD_BTN_DOWN_A + LCDSHIELD_BTN_DEV_A) && (val >= LCDSHIELD_BTN_DOWN_A - LCDSHIELD_BTN_DEV_A)) return LCDSHIELD_BTN_DOWN;
    if ((val <= LCDSHIELD_BTN_UP_A + LCDSHIELD_BTN_DEV_A) && (val >= LCDSHIELD_BTN_UP_A - LCDSHIELD_BTN_DEV_A)) return LCDSHIELD_BTN_UP;
    if ((val <= LCDSHIELD_BTN_RIGHT_A + LCDSHIELD_BTN_DEV_A)) return LCDSHIELD_BTN_RIGHT;
    return 0;
}
