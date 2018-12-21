/*
 * threadRx.c
 * 
 * Copyright (c) 2018, Jason Woodford, VO1JWW. All rights reserved.
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
 * $Id: threadRx.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"

static uint8_t bufRx[RX_STATIC_SIZE];   ///< Receive thread static buffer.

extern FILE fstrUart1;  ///< Radio modem.

/** Receive thread.
 * Rapid loop that processes a stdio stream for KISS/AX.25 packets.
 * Any validated data is assigned to an AX.25 frame slot, if available.
 */
void threadRx (uint32_t bufsz) {
    int8_t slot;
    int16_t i, j;
    //
    while (TRUE) {
        j = kissGet(bufRx, RX_STATIC_SIZE, &fstrUart1);
        if ((j > 0) && (j < RX_STATIC_SIZE)) {
            // Good KISS packet.
            slot = ax25Alloc();
            if (slot >= 0) {
                // AX.25 frame slot allocated.
                i = ax25Decode(slot, bufRx, j);
                if (i < 0) {
                    // Packet does not comply with system requirements.
                    ax25Free(slot);
                    fputs_P(PSTR("\nAX.25: "), stderr);
                    switch (i) {
                        case AX25_ERROR_DSTADDR:
                            fputs_P(PSTR("Bad destination address."), stderr);
                            break;
                        case AX25_ERROR_SRCADDR:
                            fputs_P(PSTR("Bad source address."), stderr);
                            break;
                        case AX25_ERROR_VIAADDR:
                            fputs_P(PSTR("Bad via address."), stderr);
                            break;
                        case AX25_ERROR_OVADDR:
                            fputs_P(PSTR("Too many via addresses."), stderr);
                            break;
                        case AX25_ERROR_INFOBUF:
                            fputs_P(PSTR("Insufficient memory."), stderr);
                            break;
                    }
                }
            } else {
                fputs_P(PSTR("\nAX.25: No slots available."), stderr);
            }
        } else {
            fputs_P(PSTR("\nKISS: "), stderr);
            switch (j) {
                case KISS_ERROR_BADFRAME:
                    fputs_P(PSTR("Bad frame."), stderr);
                    break;
                case KISS_ERROR_BADRX:
                    fputs_P(PSTR("Bad stream."), stderr);
                    break;
                case KISS_ERROR_BUFOVFL:
                    fputs_P(PSTR("Packet too large."), stderr);
                    break;
            }
        }
    }
}
