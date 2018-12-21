/*
 * clock.c
 * 
 * Copyright (C)2017 Jason Woodford. All rights reserved.
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
 * $Id: clock.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "clock.h"

volatile uint8_t    __system_ticks;
volatile time_t     __system_time;

/** Set System Clock.
 * 
 * This is a wrapper for any structures that depend on the system clock.
 * Use this function instead of set_system_time().
 */
void clockSet (time_t timer) {
    set_system_time(timer);
    beaconReset();
}

/** System Clock Timer.
 *
 * This is called from an ISR (eg. TIMER1_COMPA_vect) which is the system
 * tick interrupt. See atomport.c
 */
void clockTick (void) {
    if (--__system_ticks == 0) {
        __system_ticks = SYSTEM_TICKS_PER_SEC;
        // Call the GNU 1 Hz system timer.
        system_tick();
    }
}
