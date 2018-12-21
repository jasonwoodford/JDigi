/*
 * beacon.c
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
 * $Id: beacon.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "beacon.h"

Beacon beacon[BEACON_TABLE_SIZE];

extern APRSConfig   config;
extern AX25Frame    frame[];
extern DHT11        dht11;
extern APRSPosit    position;

/** Initializes the beacon table.
 *
 * Sets all beacon flags to zero.
 */
void beaconInit (void) {
    uint8_t i;
    //
    for (i = 0; i < BEACON_TABLE_SIZE; ++i) {
        beacon[i].flags = 0;
    }
}

/** Processes the beacon table.
 * 
 * Transmit any beacons that are ready. A frame slot must be available.
 */
void beaconProcess (uint8_t const *buf) {
    int8_t  slot;
    uint8_t i;
    char    *p;
    char    devid[11];
    time_t  timer;
    struct tm clk;
    //
    timer = time(NULL);
    for (i = 0; i < BEACON_TABLE_SIZE; ++i) {
        // Compile and queue a beacon once ready.
        if ((beacon[i].flags & BEACON_FLAG_USED) && (beacon[i].flags & BEACON_FLAG_E) && (beacon[i].timeout <= timer)) {
            slot = ax25Alloc();
            if (slot >= 0) {
                aprsSetFrameAddr(slot);
                localtime_r(&timer, &clk);
                dht11Read();
                strcpy_P(devid, PSTR("18B20_00"));
                p = (char*) buf; // NO BOUNDS CHECKING!
                switch (beacon[i].type) {
                    case BEACON_TYPE_BEACON:
                        p += sprintf_P(p, PSTR(">Temp: %01.1fC "), ds18b20ConvTemp(devid, DS18B20_UNIT_C));
                        break;
                    case BEACON_TYPE_WX:
                        p += sprintf_P(p, PSTR("_%02d%02d%02d%02dc...s...g...t%03.0fJD09"),
                                clk.tm_mon + 1, clk.tm_mday, clk.tm_hour, clk.tm_min,
                                ds18b20ConvTemp(devid, DS18B20_UNIT_F));
                        break;
                    case BEACON_TYPE_POSIT:
                    default:
                        p += sprintf_P(p, PSTR("@%02d%02d%02d/"), clk.tm_hour, clk.tm_min, clk.tm_sec);
                        p += aprsPrintLat(p, position.latitudeD, position.latitudeM, APRS_PRINTFMT_APRS);
                        *p++ = position.icon1;
                        p += aprsPrintLong(p, position.longitudeD, position.longitudeM, APRS_PRINTFMT_APRS);
                        *p++ = position.icon2;
                }
                if (beacon[i].str != NULL) strcpy(p, beacon[i].str);
                p = (char*) buf;
                ax25SetInfo(slot, p, strlen(p));
                frame[slot].lifetime = 0;
                frame[slot].queueflag |= (uint8_t) AX25_QUEUEFLAG_E | AX25_QUEUEFLAG_T;
                // Reset the beacon as needed.
                if (beacon[i].flags & BEACON_FLAG_ONCE) {
                    beacon[i].flags = 0;
                } else {
                    beacon[i].timeout = timer + beacon[i].interval;
                }
            }
        }
    }
}

/** Reset any expired beacon timeouts with their defined offsets.
 */
void beaconReset (void) {
    time_t  timer;
    int32_t timed;
    int8_t  i, j;
    //
    timer = time(NULL);
    for (i = 0; i < BEACON_TABLE_SIZE; ++i) {
        if (!(beacon[i].flags & BEACON_FLAG_E)) continue;
        timed = beacon[i].timeout - timer;
        if ((timed < 0) || (timed > ONE_HOUR)) {
            beacon[i].timeout = timer + beacon[i].offset;
        }
        for (j = i + 1; j < BEACON_TABLE_SIZE; j++) {
            if (!(beacon[j].flags & BEACON_FLAG_E)) continue;
            timed = beacon[j].timeout - beacon[i].timeout;
            if ((timed < BEACON_MIN_SEP) && (timed > -BEACON_MIN_SEP)) {
                beacon[j].timeout = timer + beacon[j].offset;
            }
        }
    }
}

/** Allocates and sets a beacon slot.
 * 
 * If the interval (seconds) is zero, make it a one-time beacon.
 * Returns an error if no beacon slots are available.
 */
int8_t beaconSet (uint16_t seconds, uint16_t offset, uint8_t type, char *str) {
    int8_t i;
    //
    for (i = 0; i < BEACON_TABLE_SIZE; ++i) {
        if (!(beacon[i].flags & BEACON_FLAG_USED)) break;
    }
    if (i == BEACON_TABLE_SIZE) return BEACON_ERROR_SLOT;
    beacon[i].flags |= BEACON_FLAG_USED;
    if (seconds == 0) beacon[i].flags |= BEACON_FLAG_ONCE;
    beacon[i].interval = seconds;    
    beacon[i].offset = offset;
    beacon[i].timeout = time(NULL) + offset;
    beacon[i].type = type;
    beacon[i].str = str;
    beacon[i].flags |= BEACON_FLAG_E;
    return i;
}
