/*
 * threadShell.c
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
 * $Id: threadShell.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"

static uint8_t bufGPS[GPS_STATIC_SIZE]; ///< GPS static buffer.

extern FILE fstrUart2;  ///< GPS UART.
extern nmeaINFO    nmea_info;
extern nmeaSATINFO nmea_satinfo;

/** GPS thread.
 * This infinite loop reads a NMEA sentence (GPS unit, etc.) and parses it.
 * All sentences should end with CR-LF (\r\n) and be error-free, else
 * the buffer is reset and the loop starts over.
 */
void threadGPS (uint32_t bufsz) {
    uint8_t argc, flag, len;
    int8_t i;
    char **argv, *p;
    int c;
    //
    while (TRUE) {
        p = (char*) bufGPS;
        flag = 0;
        len = 0;
        while (flag != NMEA_RXFLAG_CRLF) {
            // Capture input.
            c = fgetc(&fstrUart2);
            switch (c) {
                case _FDEV_EOF:
                    fputs_P(PSTR("\nGPS: RX EOF"), stderr);
                    flag = NMEA_RXFLAG_RESET;
                    break;
                case _FDEV_ERR:
                    fputs_P(PSTR("\nGPS: RX ERR"), stderr);
                    flag = NMEA_RXFLAG_RESET;
                    break;
                case '\r':
                    // a CR signifies first end of sentence char.
                    flag = NMEA_RXFLAG_CR;
                    break;
                case '\n':
                    // a LF signifies second end of sentence char.
                    if (flag != NMEA_RXFLAG_CR) {
                        fputs_P(PSTR("\nGPS: Bad CR/LF"), stderr);
                        flag = NMEA_RXFLAG_RESET;
                    } else {
                        // Ready to parse.
                        flag = NMEA_RXFLAG_CRLF;
                        *(p + len++) = 0;
                    }
                    break;
                default:
                    // fill buffer with data.
                    *(p + len++) = (char) c;
            }
            if (len > (NMEA_STR_MAX + 1)) {
                fputs_P(PSTR("\nGPS: RX Buffer Overflow"), stderr);
                flag = NMEA_RXFLAG_RESET;
            }
            // All RX errors are reset here.
            if (flag == NMEA_RXFLAG_RESET) {
                flag = 0;
                len = 0;
            }
        }
        // Parse the received sentence.
        if (len > 0) {
            // ECHO FOR DEBUG.
//            fputs_P(PSTR("\n"), stderr);
//            fputs(p, stderr);
            // Set aside room for the arg pointers.
            argv = (char**) p + len;
            flag = FALSE;
            // Parse the sentence to get all arguments.
            i = nmeaParseSentence(p, len, argv);
            if (i < 0) { flag = TRUE; }
            else {
                argc = (uint8_t) i;
                // Set aside room for the structure to be populated.
                p += len + (argc * sizeof(argv));
                // Compare the first arg with a sentence type.
                if (strcmp_P(argv[0], PSTR("GPGGA")) == 0) {
                    i = nmeaParseGPGGA(argc, argv, (nmeaGPGGA*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPGGAInfo((nmeaGPGGA*) p, i);
                        nmeaInfoPosTime(i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPGSA")) == 0) {
                    i = nmeaParseGPGSA(argc, argv, (nmeaGPGSA*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPGSAInfo((nmeaGPGSA*) p, i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPGSV")) == 0) {
                    i = nmeaParseGPGSV(argc, argv, (nmeaGPGSV*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPGSVInfo((nmeaGPGSV*) p, i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPGLL")) == 0) {
                    i = nmeaParseGPGLL(argc, argv, (nmeaGPGLL*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPGLLInfo((nmeaGPGLL*) p, i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPRMC")) == 0) {
                    i = nmeaParseGPRMC(argc, argv, (nmeaGPRMC*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPRMCInfo((nmeaGPRMC*) p, i);
                        nmeaInfoPosTime(i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPVTG")) == 0) {
                    i = nmeaParseGPVTG(argc, argv, (nmeaGPVTG*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaGPVTGInfo((nmeaGPVTG*) p, i);
                    }
                }
                else if (strcmp_P(argv[0], PSTR("GPZDA")) == 0) {
                    i = nmeaParseGPZDA(argc, argv, (nmeaTIME*) p);
                    if (i < 0) { flag = TRUE; }
                    else {
                        nmeaTimeInfo((nmeaTIME*) p, i);
                        nmeaInfoPosTime(i);
                    }
                }
            }
            if (flag) {
                *p = '\n';
                nmeaPrintError(p+1, i);
                fputs(p, stderr);
            }
        }
    }
}
