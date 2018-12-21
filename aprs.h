/*
 * aprs.h
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
 * $Id: aprs.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup aprs <aprs.h>: APRS
 * \code #include <aprs.h> \endcode
 * <h3>Introduction to the APRS Module</h3>
 *  
 * This provides the APRS related application code.
 */

#ifndef __APRS_H__
#define __APRS_H__

#include "threads.h"
#include <math.h>

/** \ingroup aprs */
/* @{ */
#define APRS_CONFIG_BASE_SIZE       71      ///< Size of 'base' configuration.
#define APRS_CONFIG_TOTAL_SIZE      128     ///< Total configuration size.
#define APRS_CONFIG_EXT_SIZE        19
#define APRS_CONFIG_COMMENT_SIZE    (APRS_CONFIG_TOTAL_SIZE - (APRS_CONFIG_BASE_SIZE + APRS_CONFIG_EXT_SIZE))
#define APRS_PRINTFMT_APRS      0
#define APRS_PRINTFMT_DEGMIN    1
#define APRS_POSITFLAG_E        (1 << 7)    ///< Assumes valid position data.
#define APRS_POSITFLAG_CS       (1 << 6)
#define APRS_POSITFLAG_PHG      (1 << 5)
#define APRS_POSITFLAG_RNG      (1 << 4)

#define aprsIsNum(p)    ((*p >= '0') && (*p <= '9'))
#define aprsIsUalpha(p) ((*p >= 'A') && (*p <= 'Z'))
#define aprsIsLalpha(p) ((*p >= 'a') && (*p <= 'z'))

/** APRSConfig structure.
 * 
 * This defines the config object for all situations.
 */
typedef struct APRSConfig {
// Base BEGIN.
    char     srcCall[7];    ///< Local station callsign, null-terminated.
    uint8_t  srcSSID;       ///< Local station SSID.
    char     destCall[7];   ///< Default destination callsign, null-terminated.
    uint8_t  destSSID;      ///< Default destination SSID.
    char     viaCall1[7];   ///< Default via1 callsign, null-terminated.
    uint8_t  viaSSID1;      ///< Default via1 SSID.
    char     viaCall2[7];   ///< Default via2 callsign, null-terminated.
    uint8_t  viaSSID2;      ///< Default via2 SSID.
    uint8_t  chainFlags;    ///< Defines chain allow and reject rules.
    uint16_t txIntPos;      ///< Posit interval, in seconds.
    uint16_t txIntWX;       ///< WX interval, in seconds.
    uint16_t txIntBeacon;   ///< Text beacon interval, in seconds.
    uint16_t txOffPos;      ///< Posit interval offset, in seconds.
    uint16_t txOffWX;       ///< WX interval offset, in seconds.
    uint16_t txOffBeacon;   ///< Text beacon interval offset, in seconds.
    int8_t   posLatD;       ///< Current latitude degrees, signed.
    float    posLatM;       ///< Current latitude minutes, decimal.
    int16_t  posLongD;      ///< Current longitude degrees, signed.
    float    posLongM;      ///< Current longitude minutes, decimal.
    char     icon1;         ///< APRS icon char #1.
    char     icon2;         ///< APRS icon char #2.
    uint16_t uart0Baud;     ///< BPS for USB.
    uint16_t uart1Baud;     ///< BPS for KISS modem.
    uint16_t uart2Baud;     ///< BPS for GPS.
    uint8_t  lcdMode;       ///< Default LCD mode.
    uint8_t  debugFlags;    ///< Defines aprsPrintFrame() verbosity.
    uint8_t  repeatFlags;   ///< Defines what should be repeated.
    int32_t  utcOffset;     ///< Time zone offset, in seconds.
// Base END. Total of 71 bytes.
// Extended BEGIN.
    uint16_t crc0;          ///< Base configuration CRC.
    uint16_t crc1;          ///< Comment CRC.
    uint32_t lastTime;      ///< Holds the "last time".
    char     devTemp[11];   ///< Device IDs have a max of 10 chars.
// Extended END. Total of 19 bytes.
    char     infoComment[APRS_CONFIG_COMMENT_SIZE];  ///< Comment buffer.
} APRSConfig;

typedef struct APRSPosit {
    uint8_t  flags;
    int8_t   latitudeD;      ///< Current latitude degrees, signed.
    double   latitudeM;      ///< Current latitude minutes, decimal, signed.
    int16_t  longitudeD;     ///< Current longitude degrees, signed.
    double   longitudeM;     ///< Current longitude minutes, decimal, signed.
    char     icon1;          ///< APRS icon char #1.
    char     icon2;          ///< APRS icon char #2.
    uint16_t course;
    uint16_t speed;
    char     power;
    char     height;
    char     gain;
    char     direction;
    uint16_t range;
} APRSPosit;

void    aprsParseInfo (uint8_t slot, char *buf);
int8_t  aprsParseAddr (char *str, uint8_t *ssid);
void    aprsSetFrameAddr (uint8_t slot);
int8_t  aprsGetSetTx (char *str, uint8_t lifetime);
char*   aprsMsgReply (char *str, uint8_t *addr);
void    aprsMakeAddr (char *str, uint8_t *addr);
void    aprsParsePos (char *p, uint8_t ss);
uint8_t aprsPrintLat (char *buf, int8_t lat, double min, uint8_t fmt);
uint8_t aprsPrintLong (char *buf, int16_t lon, double min, uint8_t fmt);

/* @} */
#endif /* __APRS_H__ */
