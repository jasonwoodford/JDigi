/*
 * aprs.c
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
 * $Id: aprs.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "aprs.h"
#include "jdigi-pgmstr.h"   // Flash Program Data Defaults.
#include "command-extern.h" // Program memory string pointers.

APRSConfig config;
APRSPosit position;

extern APRSPosit statPosit[];
extern AX25Frame frame[];
extern AX25FrameInfo frameinfo;
extern Beacon beacon[];
extern LCD lcd;
extern Stats statTable[];

/** Parses the contents of a frame for APRS and JDigi commands.
 * Uses buf as a temporary string buffer.
 */
void aprsParseInfo (uint8_t slot, char *buf) {
    uint8_t argc, flags, ssid;
    int16_t i, infosize;
    char    *addr, **argv, *bufnew, *p, *q;
    //
    flags = frame[slot].queueflag;
    if (!((flags & AX25_QUEUEFLAG_E) && (flags & AX25_QUEUEFLAG_F) && !(flags & AX25_QUEUEFLAG_T) && !(flags & AX25_QUEUEFLAG_Z))) return;
    // Is this an AX.25 APRS type frame?
    if ((frame[slot].ctrl != 0x03) || (frame[slot].pid != 0xF0)) return;
    // Copy the info into our buffer.
    infosize = ax25GetInfo(slot, buf, MAIN_STATIC_SIZE);
    if (infosize < 1) return;
    // Allocate room for following functions.
    argv = (char**) buf + infosize;
    bufnew = buf + infosize + (8 * sizeof(char**));
    //// Directed Message Handler
    //
    if ((*buf == ':') && (*(buf+10) == ':') && (infosize >= 11)) {
        addr = buf+1;
        i = aprsParseAddr(addr, &ssid);
        //// Is this message explicitly for us?
        //
        if (!strcmp(config.srcCall, addr) && (config.srcSSID == ssid)) {
            // Put this frame in purgatory; no need to repeat.
            // Only reference this frame from this point on.
            frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
            // Address with SSID.
            q = bufnew;
            q += ax25FmtAddr(q, frame[slot].addr[1]);
            *q++ = ':';
            // Message body.
            p = buf+11;
            for (i = infosize-11; i > 1; --i) {
                if (*p == '{') {
                    *p++ = 0;
                    break;
                }
                *q++ = *p++;
            }
            *q = 0;
            atomQueuePut(&lcd.msgqueue, 10, (uint8_t*) bufnew);
            //// Does this message require an ACK?
            //
            if ((i > 2) && (i < 6)) {
                // Get message ID.
                argv[0] = p;
                while (i > 0) {
                    if (!(aprsIsNum(p) || aprsIsUalpha(p) || aprsIsLalpha(p))) break;
                    --i;
                    ++p;
                }
                *p = 0;
                // Build new message, starting with callsign, then ACK w/ ID.
                q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                q += sprintf_P(q, PSTR("ack%s"), argv[0]);
                aprsGetSetTx(bufnew, AX25_FRAME_LIFETIME_TX);
            }
            //// Is this a directed query?
            //
            if (*(buf+11) == '?') {
                argc = cmdParseLine(buf+12, infosize-12, argv);
                if (argc == 0) {
                    // do nothing.
                } else if (strcmp_P(argv[0], PSTR("APRSD")) == 0) {
                    // Directed - Stations heard direct.
                    q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                    cmdAPRSHeardDirect(q, argc, argv);
                    aprsGetSetTx(bufnew, 2 * AX25_FRAME_LIFETIME_TX);
                    //
                    q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                    cmdAPRSHeardAll(q, argc, argv);
                    aprsGetSetTx(bufnew, 3 * AX25_FRAME_LIFETIME_TX);
                } else if (strcmp_P(argv[0], PSTR("APRSH")) == 0) {
                    // Directed - Position of heard station w/ hourly stats.
                    q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                    cmdAPRSHeardStatsDirect(q, argc, argv);
                    aprsGetSetTx(bufnew, 2 * AX25_FRAME_LIFETIME_TX);
                    //
                    q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                    cmdAPRSHeardStatsAll(q, argc, argv);
                    aprsGetSetTx(bufnew, 3 * AX25_FRAME_LIFETIME_TX);
                } else if (strcmp_P(argv[0], PSTR("APRSM")) == 0) {
                    // Directed - Outstanding messages.
                } else if (strcmp_P(argv[0], PSTR("APRSO")) == 0) {
                    // Directed - Station objects.
                } else if (strcmp_P(argv[0], PSTR("APRSP")) == 0) {
                    // Directed - Station position.
                    beaconSet(0, 5, BEACON_TYPE_POSIT, config.infoComment);
                } else if (strcmp_P(argv[0], PSTR("APRSS")) == 0) {
                    // Directed - Station status.
                    beaconSet(0, 5, BEACON_TYPE_BEACON, config.infoComment);
                } else if ((strcmp_P(argv[0], PSTR("APRST")) == 0) || (strcmp_P(argv[0], PSTR("PING")) == 0)) {
                    // Directed - Route trace.
                    q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                    cmdAPRSRouteTrace(q, slot);
                    aprsGetSetTx(bufnew, 2 * AX25_FRAME_LIFETIME_TX);
                }
                return;
            }
            //// Is this a JDigi command?
            //
            if (*(buf+11) == '%') {
                // These are the JDigi builtin commands.
                argc = cmdParseLine(buf+12, infosize-12, argv);
                strupr(argv[0]);
                q = aprsMsgReply(bufnew, frame[slot].addr[1]);
                //
                if (argc == 0) {
                    sprintf_P(q, PSTR("This is JDigi"));
                } else if (strcmp_P(argv[0], pstrCmdBeaconList) == 0) {
                    cmdBeaconList(q);
                } else if (strcmp_P(argv[0], pstrCmdBeaconClear) == 0) {
                    cmdBeaconClear(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdBeaconSet) == 0) {
                    cmdBeaconSet(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetClock) == 0) {
                    cmdClockSet(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetCall) == 0) {
                    cmdConfigSetcall(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetDest) == 0) {
                    cmdConfigSetdest(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetVia1) == 0) {
                    cmdConfigSetvia1(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetVia2) == 0) {
                    cmdConfigSetvia2(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetPos) == 0) {
                    cmdConfigSetpos(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetIcon) == 0) {
                    cmdConfigSeticon(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetComment) == 0) {
                    cmdConfigSetcomment(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetPosint) == 0) {
                    cmdConfigSetposint(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetWXint) == 0) {
                    cmdConfigSetwxint(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetBeaconint) == 0) {
                    cmdConfigSetbeaconint(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdSetChainflags) == 0) {
                    cmdConfigSetchainflags(q, argc, argv);
                } else if (strcmp_P(argv[0], pstrCmdWriteEeprom) == 0) {
                    cmdConfigWriteEeprom(q, argc, argv);
                } else {
                    sprintf_P(q, PSTR("?Unrecognized command"));
                }
                aprsGetSetTx(bufnew, 2 * AX25_FRAME_LIFETIME_TX);
            }
            return;
        }
        // Is this a group message?
        i = FALSE;
        if (strcmp_P(addr, PSTR("ALL")) == 0) { i = TRUE; }
        else if (strcmp_P(addr, PSTR("CQ")) == 0) { i = TRUE; }
        else if (strcmp_P(addr, PSTR("QST")) == 0) { i = TRUE; }
        else if (strcmp_P(addr, PSTR("NWS")) >= 0) { i = TRUE; }
        else if (strcmp_P(addr, PSTR("BLN")) >= 0) { i = TRUE; }
        if (i) {
            frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
            // Print the message.
            sprintf_P(bufnew, PSTR("%s:%s"), addr, buf+11);
            atomQueuePut(&lcd.msgqueue, 10, (uint8_t*) bufnew);
            return;
        }
    }
    //// General Query Handler
    //
    if (*buf == '?') {
        frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
        argc = cmdParseLine(buf+1, infosize-1, argv);
        if (argc == 0) {
            // do nothing.
        } else if (strcmp_P(argv[0], PSTR("APRS")) >= 0) {
            // General - All stations query.
            beaconSet(0, 5, BEACON_TYPE_POSIT, config.infoComment);
        } else if (strcmp_P(argv[0], PSTR("WX")) >= 0) {
            // General - Weather report.
            beaconSet(0, 5, BEACON_TYPE_WX, config.infoComment);
        } else if (strcmp_P(argv[0], PSTR("IGATE")) >= 0) {
            // General - IGATE capabilities.
        }
    }
}

/** Parses str for a complete address (ie XY1ABC-12).
 *
 * Places the result in str and ssid.
 * Returns the number of chars in str, or -1 if an error.
 */
int8_t aprsParseAddr (char *str, uint8_t *ssid) {
    int8_t i;
    //
    i = 0;
    while (aprsIsUalpha(str) || aprsIsNum(str)) {
        if (i == 6) {
            *str = 0;
            return -1;
        }
        ++str;
        ++i;
    }
    if (*str == '-') {
        *str++ = 0;
        *ssid = atoi(str);
        if (*ssid > 15) return -1;
    } else {
        *str = 0;
        *ssid = 0;
    }
    return i;
}

/** Sets the frame addresses using the current config for an APRS transmission.
 */
void aprsSetFrameAddr (uint8_t slot) {
    ax25SetAddr(config.srcCall, config.srcSSID, frame[slot].addr[1]);
    ax25SetAddr(config.destCall, config.destSSID, frame[slot].addr[0]);
    frame[slot].hops = 0;
    if (config.viaCall1[0] != 0) {
        ax25SetAddr(config.viaCall1, config.viaSSID1, frame[slot].addr[2]);
        ++frame[slot].hops;
        if (config.viaCall2[0] != 0) {
            ax25SetAddr(config.viaCall2, config.viaSSID2, frame[slot].addr[3]);
            ++frame[slot].hops;
        }
    }
    frame[slot].ctrl = 0x03;
    frame[slot].pid = 0xF0;
    frame[slot].cmdresp = 0;
}

/** Reserve an AX.25 slot, populate it, and set for transmission.
 * 
 * Returns the AX.25 slot number.
 */
int8_t aprsGetSetTx (char *str, uint8_t lifetime) {
    int8_t slot;
    //
    slot = ax25Alloc();
    if (slot >= 0) {
        aprsSetFrameAddr(slot);
        ax25SetInfo(slot, str, strlen(str));
        frame[slot].lifetime = lifetime;
        frame[slot].queueflag |= (uint8_t) (AX25_QUEUEFLAG_E | AX25_QUEUEFLAG_T);
    }
    return slot;
}

/**
 */
char* aprsMsgReply (char *str, uint8_t *addr) {
    uint8_t i;
    //
    *str++ = ':';
    i = ax25FmtAddr(str, addr);
    str += i;
    while (i++ < 9) *str++ = ' ';
    *str++ = ':';
    *str = 0;
    return str;
}

/**
 */
void aprsMakeAddr (char *str, uint8_t *addr) {
    uint8_t ssid;
    uint8_t i;
    char    *p;
    //
    i = 0;
    while ((aprsIsNum(str) || aprsIsUalpha(str)) && (i < 6)) {
        *(addr + i) = *str++;
        ++i;
    }
    while (i < 6) {
        *(addr + i) = ' ';
        ++i;
    }
    if (*str++ == '-') {
        p = str;
        ssid = atoi(p);
        if (ssid > 15) ssid = 0;
    } else {
        ssid = 0;
    }
    *(addr+6) = (char) ssid;
}

/**
 */
void aprsParsePos (char *p, uint8_t ss) {
    int16_t  i, j;
    uint32_t x, y;
    double   lat, lon;
    char     c;
    //
    statPosit[ss].flags = APRS_POSITFLAG_E;
    if (aprsIsNum(p)) {
        //// Plain text.
        // latitude.
        sscanf_P(p, PSTR("%4u.%2u%c%c"), &i, &j, &c, &statPosit[ss].icon1);
        statPosit[ss].latitudeD = (i / 100);
        statPosit[ss].latitudeM = (double) (i % 100) + ((double) j / 100.0);
        if (c == 'S') {
            statPosit[ss].latitudeD *= -1;
            statPosit[ss].latitudeM *= -1.0;
        }
        p += 9;
        // longitude.
        sscanf_P(p, PSTR("%5u.%2u%c%c"), &i, &j, &c, &statPosit[ss].icon2);
        statPosit[ss].longitudeD = (i / 100);
        statPosit[ss].longitudeM = (double) (i % 100) + ((double) j / 100.0);
        if (c == 'W') {
            statPosit[ss].longitudeD *= -1;
            statPosit[ss].longitudeM *= -1.0;
        }
        p += 10;
        // data extensions.
        if (*(p+3) == '/') {
            // course and speed.
            statPosit[ss].flags |= APRS_POSITFLAG_CS;
            sscanf_P(p, PSTR("%3u/%3u"), &i, &j);
            statPosit[ss].course = i;
            statPosit[ss].speed = j;
        }
        else if ((*p == 'P') && (*(p+1) == 'H') && (*(p+2) == 'G')) {
            // power, height, gain and direction.
            statPosit[ss].flags |= APRS_POSITFLAG_PHG;
            statPosit[ss].power = *(p+3);
            statPosit[ss].height = *(p+4);
            statPosit[ss].gain = *(p+5);
            statPosit[ss].direction = *(p+6);
        }
        else if ((*p == 'R') && (*(p+1) == 'N') && (*(p+2) == 'G')) {
            // range.
            statPosit[ss].flags |= APRS_POSITFLAG_RNG;
            sscanf_P(p+3, PSTR("%4u"), &i);
            statPosit[ss].range = i;
        }
    } else {
        //// Compressed.
        statPosit[ss].icon1 = *p++;
        y = ((*p-33) * (91^3)) + ((*(p+1)-33) * (91^2)) + ((*(p+2)-33) * 91) + (*(p+3)-33);
        p += 4;
        x = ((*p-33) * (91^3)) + ((*(p+1)-33) * (91^2)) + ((*(p+2)-33) * 91) + (*(p+3)-33);
        p += 4;
        statPosit[ss].icon2 = *p++;
        lat = 90.0 - ((double) y / 380926.0);
        lon = -180.0 + ((double) x / 190463.0);
        statPosit[ss].latitudeD = (int8_t) lat;
        statPosit[ss].latitudeM = fmod(lat, 1.0) * 0.6;
        statPosit[ss].longitudeD = (int16_t) lon;
        statPosit[ss].longitudeM = fmod(lon, 1.0) * 0.6;
        if (*p == ' ') return;
        if (*p == '{') {
            statPosit[ss].flags |= APRS_POSITFLAG_RNG;
            statPosit[ss].range = (uint16_t) 2 * powf(1.08f, (float) (*(p+1)-33));
        } else {
            statPosit[ss].flags |= APRS_POSITFLAG_CS;
            statPosit[ss].course = (uint16_t) 4 * (*p-33);
            statPosit[ss].speed = (uint16_t) powf(1.08f, (float) (*(p+1)-33));
        }
    }
}

/**
 */
uint8_t aprsPrintLat (char *buf, int8_t lat, double min, uint8_t fmt) {
    char    sign;
    uint8_t len;
    //
    sign = 'N';
    if (lat < 0) {
        lat *= -1;
        min *= -1.0;
        sign = 'S';
    }
    len = 0;
    switch (fmt) {
        case APRS_PRINTFMT_DEGMIN:
            len = sprintf_P(buf, PSTR("%02d%c%02.2f%c"), lat, LCD_SYM_DEGREE, min, sign);
            break;
        case APRS_PRINTFMT_APRS:
            len = sprintf_P(buf, PSTR("%02d%02.2f%c"), lat, min, sign);
            break;
    }
    return len;
}

/**
 */
uint8_t aprsPrintLong (char *buf, int16_t lon, double min, uint8_t fmt) {
    char    sign;
    uint8_t len;
    //
    sign = 'E';
    if (lon < 0) {
        lon *= -1;
        min *= -1.0;
        sign = 'W';
    }
    len = 0;
    switch (fmt) {
        case APRS_PRINTFMT_DEGMIN:
            len = sprintf_P(buf, PSTR("%03d%c%02.2f%c"), lon, LCD_SYM_DEGREE, min, sign);
            break;
        case APRS_PRINTFMT_APRS:
            len = sprintf_P(buf, PSTR("%03d%02.2f%c"), lon, min, sign);
            break;
    }
    return len;
}
