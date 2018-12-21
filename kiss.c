/*
 * kiss.c
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
 * $Id: kiss.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "kiss.h"

/** Reads an incoming KISS stream and writes the data contents to buf.
 * 
 * size is the size of the buffer, to prevent buffer overflow.
 * 
 * Our return value is a signed 16-bit value:
 * MSB:sxppppnnnnnnnnnn:LSB
 * if (s), then error
 * x = don't care
 * p = 4-bit port number
 * n = 10-bit bytes received
 */
int16_t kissGet (uint8_t *buf, uint16_t size, FILE *stream) {
    int n; // Bytes received, 10 bits (0-1023)
    int c; // temp
    uint8_t kiss_flags;
    uint8_t kiss_port;
    
    n = 1;
    kiss_flags = 0;
    kiss_port = 0;
    while (!(kiss_flags & KISS_FLAG_DONE)) {
        c = fgetc(stream);
        if (c == _FDEV_ERR) {
            // bad RX.
            n = KISS_ERROR_BADRX;
            kiss_flags |= KISS_FLAG_DONE;
            continue;
        }
        if (kiss_flags & KISS_FLAG_FEND) {
            // "inside" the frame
            if (kiss_flags & KISS_FLAG_CMD) {
                // one byte only
                kiss_flags &= ~KISS_FLAG_CMD;
                kiss_port = (c >> 4) & 0x0F;
                if ((c & 0x0F) == 0) {
                    // command type 0 = data
                    kiss_flags |= KISS_FLAG_VALID;
                } else {
                    // discard all remaining data
                    n = KISS_ERROR_BADFRAME;
                    kiss_flags |= KISS_FLAG_DONE;
                }
                continue;
            }
            if (kiss_flags & KISS_FLAG_FESC) {
                // transpose rules
                if (kiss_flags & KISS_FLAG_VALID) {
                    switch (c) {
                        case KISS_TFEND:
                            *buf = KISS_FEND;
                            ++buf;
                            ++n;
                            break;
                        case KISS_TFESC:
                            *buf = KISS_FESC;
                            ++buf;
                            ++n;
                            break;
                    }
                }
                // always exit transpose after one byte
                kiss_flags &= ~KISS_FLAG_FESC;
            } else {
                // is this the end of the frame?
                if (c == KISS_FEND) {
                    kiss_flags |= KISS_FLAG_DONE;
                } else {
                    if (n == size) {
                        // buffer overflow
                        kiss_flags |= KISS_FLAG_DONE;
                        n = KISS_ERROR_BUFOVFL;
                    } else {
                        if (c == KISS_FESC) {
                            kiss_flags |= KISS_FLAG_FESC;
                        } else if (kiss_flags & KISS_FLAG_VALID) {
                            *buf = (uint8_t) c;
                            ++buf;
                            ++n;
                        }
                    }
                }
            }
        } else {
            // "outside" the frame
            if (c == KISS_FEND) {
                // now the frame starts
                kiss_flags |= (KISS_FLAG_FEND | KISS_FLAG_CMD);
            }
            // discard anything else
        }
    }
    // our return value is a signed 16-bit value:
    // MSB:sxppppnnnnnnnnnn:LSB
    // if (s), then error
    // x = don't care
    // p = 4-bit port number
    // n = 10-bit bytes received
    if (n > 0) {
        n |= (kiss_port << 10);
    }
    return n;
}

/** Takes data from buf and sends an outgoing KISS stream.
 *
 * size is the amount of data to encode.
 * porttype is a combination of the port number (4MSB) and command (4LSB)
 * Returns number of bytes sent.
 */
uint16_t kissPut (uint8_t *buf, uint16_t size, uint8_t porttype, FILE *stream) {
    uint16_t i, j;
    
    j = 3;  // This includes the minimum of three extra KISS bytes.
    fputc(KISS_FEND, stream);
    fputc(porttype, stream);
    for (i = 0; i < size; ++i) {
        switch (*buf) {
            case KISS_FEND:
                fputc(KISS_FESC, stream);
                fputc(KISS_TFEND, stream);
                ++j;
                break;
            case KISS_FESC:
                fputc(KISS_FESC, stream);
                fputc(KISS_TFESC, stream);
                ++j;
                break;
            default:
                fputc(*buf, stream);
        }
        ++buf;
    }
    fputc(KISS_FEND, stream);
    return (i + j);
}
