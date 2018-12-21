/*
 * stats.c
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
 * $Id: stats.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "stats.h"

Stats       statTable[STATS_SLOTS];
APRSPosit   statPosit[STATS_SLOTS];

extern AX25Frame frame[];

/** Reset all stat slot flags.
 */
void statsInit (void) {
    uint8_t i;
    //
    for (i = 0; i < STATS_SLOTS; ++i) statTable[i].flags = 0;
}

/** Check to see if any stat slots need an hourly update.
 */
void statsRefresh (void) {
    uint8_t i, j;
    time_t  timer;
    //
    timer = time(NULL);
    for (i = 0; i < STATS_SLOTS; ++i) {
        // Check for a valid slot.
        if (!(statTable[i].flags & STATS_FLAG_E)) continue;
        // Release this slot if too old.
        if ((timer - statTable[i].lastHeard) >= (STATS_HOURS * ONE_HOUR)) {
            statTable[i].flags = 0;
            continue;
        }
        // Skip if less than 1 hour old.
        if ((timer - statTable[i].hourlyUpdate) < ONE_HOUR) continue;
        // Otherwise shift the heard history.
        for (j = (STATS_HOURS - 1); j > 0; --j) {
            statTable[i].hourlyDirect[j] = statTable[i].hourlyDirect[j-1];
            statTable[i].hourlyHeard[j] = statTable[i].hourlyHeard[j-1];
        }
        statTable[i].hourlyDirect[0] = 0;
        statTable[i].hourlyHeard[0] = 0;
        statTable[i].hourlyUpdate = timer;
    }
}

/** Check an AX.25 frame for potential stat update.
 */
void statsUpdate (uint8_t slot, char *buf) {
    uint8_t infosize;
    uint8_t flags;
    int8_t  ss;
    int8_t  i, j;
    char    *p;
    //
    // Test frame slot context.
    flags = frame[slot].queueflag;
    if (!((flags & AX25_QUEUEFLAG_E) && (flags & AX25_QUEUEFLAG_F) && !(flags & AX25_QUEUEFLAG_T) && !(flags & AX25_QUEUEFLAG_Z))) return;
    // Find a matching callsign or get a new slot.
    ss = statsGetSlot(frame[slot].addr[1]);
    // Only update when a valid slot exists.
    if (ss >= 0) {
        statTable[ss].lastHeard = time(NULL);
        ++statTable[ss].hourlyHeard[0];
        // Was this packet repeated?
        j = 0;
        for (i = 0; i < frame[slot].hops; ++i) {
            if (frame[slot].addr[i+2][6] & 0x80) ++j;
        }
        if (j == 0) ++statTable[ss].hourlyDirect[0];
        // Parse the info data.
        infosize = ax25GetInfo(slot, buf, MAIN_STATIC_SIZE);
        if (infosize > 25) {
            p = buf;
            switch (*p++) {
                case '/':
                case '@':
                    // with timestamp.
                    p += 7; // skip it.
                case '!':
                case '=':
                    // without timestamp.
                    aprsParsePos(p, ss);
            }
        }
    }
}

/** Attempt to get a referenced stat slot, or return a new slot.
 * 
 * Returns a slot, or an error.
 */
int8_t statsGetSlot (uint8_t *addr) {
    int8_t i, j;
    //
    // Get the referenced stat slot if it exists.
    for (i = 0; i < STATS_SLOTS; ++i) {
        if (!(statTable[i].flags & STATS_FLAG_E)) continue;
        if (memcmp(statTable[i].callsign, addr, 7) == 0) return i;
    }
    // Otherwise attempt to get and reset a new slot.
    for (i = 0; i < STATS_SLOTS; ++i) {
        if (statTable[i].flags & STATS_FLAG_USED) continue;
        statTable[i].flags |= STATS_FLAG_USED;
        memcpy(statTable[i].callsign, addr, 7);
        statTable[i].hourlyUpdate = time(NULL);
        for (j = 0; j < STATS_HOURS; ++j) {
            statTable[i].hourlyDirect[j] = 0;
            statTable[i].hourlyHeard[j] = 0;
        }
        statTable[i].flags |= STATS_FLAG_E;
        return i;
    }
    return STATS_ERROR_NOSLOTS;
}

/** Given a stat slot, attempt to find the next valid slot.
 * 
 * Returns the next slot if successful, or the provided slot.
 */
uint8_t statsGetNextSlot (uint8_t ss) {
    uint8_t i;
    //
    for (i = 0; i < STATS_SLOTS; ++i) {
        ++ss;
        if (ss == STATS_SLOTS) ss = 0;
        if (statTable[ss].flags & STATS_FLAG_E) break;
    }
    return ss;
}
