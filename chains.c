/*
 * chains.c
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
 * $Id: chains.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "chains.h"

extern AX25Frame frame[];
extern AX25FrameInfo frameinfo;
extern APRSConfig config;
extern FILE fstrUart1;

static char const idaPStr00[] PROGMEM = "AIR";
static char const idaPStr01[] PROGMEM = "ALL";
static char const idaPStr02[] PROGMEM = "AP";
static char const idaPStr03[] PROGMEM = "BEACON";
static char const idaPStr04[] PROGMEM = "CQ";
static char const idaPStr05[] PROGMEM = "GPS";
static char const idaPStr06[] PROGMEM = "DF";
static char const idaPStr07[] PROGMEM = "DGPS";
static char const idaPStr08[] PROGMEM = "DRILL";
static char const idaPStr09[] PROGMEM = "DX";
static char const idaPStr10[] PROGMEM = "ID";
static char const idaPStr11[] PROGMEM = "JAVA";
static char const idaPStr12[] PROGMEM = "MAIL";
static char const idaPStr13[] PROGMEM = "MICE";
static char const idaPStr14[] PROGMEM = "QST";
static char const idaPStr15[] PROGMEM = "QTH";
static char const idaPStr16[] PROGMEM = "RTCM";
static char const idaPStr17[] PROGMEM = "SKY";
static char const idaPStr18[] PROGMEM = "SPACE";
static char const idaPStr19[] PROGMEM = "SPC";
static char const idaPStr20[] PROGMEM = "SYM";
static char const idaPStr21[] PROGMEM = "TEL";
static char const idaPStr22[] PROGMEM = "TEST";
static char const idaPStr23[] PROGMEM = "TLM";
static char const idaPStr24[] PROGMEM = "WX";
static char const idaPStr25[] PROGMEM = "ZIP";

static char const isaPStr00[] PROGMEM = "VO1GFR";
static char const isaPStr01[] PROGMEM = "VO1BLR";

static PGM_P const input_dest_allow[] PROGMEM =
  { idaPStr00, idaPStr01, idaPStr02, idaPStr03, idaPStr04,
    idaPStr05, idaPStr06, idaPStr07, idaPStr08, idaPStr09,
    idaPStr10, idaPStr11, idaPStr12, idaPStr13, idaPStr14,
    idaPStr15, idaPStr16, idaPStr17, idaPStr18, idaPStr19,
    idaPStr20, idaPStr21, idaPStr22, idaPStr23, idaPStr24,
    idaPStr25 };

static PGM_P const input_source_allow[] PROGMEM =
  { isaPStr00, isaPStr01 };

static PGM_P const input_source_reject[] PROGMEM =
  {    };

static PGM_P const input_dest_reject[] PROGMEM =
  {    };

/** The filtering for the input chain is probably the most stringent.
 *  Any packet received from radio starts in the input chain.
 *  Depending on the value of the CHAINFLAGS_INPUT_DENY flag
 *  frames are either forwarded or dropped.
 */
void chainInput (uint8_t slot) {
    char    buf[7];
    uint8_t flag, i, k;
    //
    flag = frame[slot].queueflag;
    if ((flag & AX25_QUEUEFLAG_E) && !(flag & AX25_QUEUEFLAG_F) && !(flag & AX25_QUEUEFLAG_T) && !(flag & AX25_QUEUEFLAG_Z)) {
        chainPrintFrame(stderr, slot, config.debugFlags);
        // Do we already have this packet in queue?
        for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
            // Skip ourselves and any invalid frames.
            if ((slot == i) || !(frame[i].queueflag & AX25_QUEUEFLAG_E)) continue;
            if (frame[slot].infocrc == frame[i].infocrc) {
                // Yes we do. Dequeue this frame.
                frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
                frame[slot].lifetime = 0;
                return;
            }
        }
        // 
        if (config.chainFlags & CHAINFLAGS_INPUT_DENY) {
            // Default: deny
            k = FALSE;
            for (i = 0; i < INPUT_SOURCE_ALLOW_SIZE; ++i) {
                strcpy_P(buf, input_source_allow[i]);
                if (ax25CmpAddr(buf, frame[slot].addr[1])) { k = TRUE; break; }
            }
            for (i = 0; i < INPUT_DEST_ALLOW_SIZE; ++i) {
                strcpy_P(buf, input_dest_allow[i]);
                if (ax25CmpAddr(buf, frame[slot].addr[0])) { k = TRUE; break; }
            }
        } else {
            // Default: allow
            k = TRUE;
            for (i = 0; i < INPUT_SOURCE_REJECT_SIZE; ++i) {
                strcpy_P(buf, input_source_reject[i]);
                if (ax25CmpAddr(buf, frame[slot].addr[1])) { k = FALSE; break; }
            }
            for (i = 0; i < INPUT_DEST_REJECT_SIZE; ++i) {
                strcpy_P(buf, input_dest_reject[i]);
                if (ax25CmpAddr(buf, frame[slot].addr[0])) { k = FALSE; break; }
            }
        }
        // If the source is us, drop the packet.
        if ((ax25CmpAddr(config.srcCall, frame[slot].addr[1])) &&
            ((frame[slot].addr[1][6] & 0x0F) == config.srcSSID)) k = FALSE;
        // If the destination is us, let aprsParseInfo() handle it.
        if ((ax25CmpAddr(config.srcCall, frame[slot].addr[0])) &&
            ((frame[slot].addr[0][6] & 0x0F) == config.srcSSID)) k = TRUE;
        if (k) {
            // send frame to forward chain.
            frame[slot].queueflag |= AX25_QUEUEFLAG_F;
        } else {
            // set to dequeue frame.
            frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
            frame[slot].lifetime = 0;
        }
    }
    return;
}

/** The forward chain is where frames are inspected for repeating. They are
 *  either repeated and sent to the output/TX chain, or put into purgatory.
 */
void chainForward (uint8_t slot) {
    char    buf[7];
    int8_t  i, j, k;
    uint8_t flag;
    //
    flag = frame[slot].queueflag;
    if ((flag & AX25_QUEUEFLAG_E) && (flag & AX25_QUEUEFLAG_F) && !(flag & AX25_QUEUEFLAG_T) && !(flag & AX25_QUEUEFLAG_Z)) {
        chainPrintFrame(stderr, slot, config.debugFlags);
        /* Check VIA Addresses. */
        for (j = 1; j <= frame[slot].hops; ++j) {
            // Did we repeat this already?
            if (ax25CmpAddr(config.srcCall, frame[slot].addr[j+1]) && (frame[slot].addr[j+1][6] == (config.srcSSID | 0x80))) break;
            // Is this a repeated via?
            if (frame[slot].addr[j+1][6] & 0x80) continue;
            // Is our address w/SSID in the via list?
            if (ax25CmpAddr(config.srcCall, frame[slot].addr[j+1]) && (frame[slot].addr[j+1][6] == config.srcSSID)) {
                // If so, set the repeated flag and re-transmit.
                frame[slot].addr[j+1][6] |= 0x80;
                goto chainForwardTx;
            }
            // Do we have a 'relay' via that has not been repeated?
            k = 0;
            strcpy_P(buf, PSTR("RELAY"));
            if (ax25CmpAddr(buf, frame[slot].addr[j+1])) k = 1;
            strcpy_P(buf, PSTR("TRACE"));
            if (ax25CmpAddr(buf, frame[slot].addr[j+1])) k = 1;
            strcpy_P(buf, PSTR("WIDE "));   // 'pure' WIDE, not WIDEn-N
            if (ax25CmpAddr(buf, frame[slot].addr[j+1])) k = 1;
            if (k && (config.repeatFlags & CHAIN_REPEATFLAG_LEGACY)) {
                // Replace this via with ourselves.
                ax25SetAddr(config.srcCall, config.srcSSID | (uint8_t) 0x80, frame[slot].addr[j+1]);
                goto chainForwardTx;
            }
            if (config.repeatFlags & CHAIN_REPEATFLAG_WIDE1) {
                // Is this a WIDE1-N w/ a SSID > 0, and do we have via room?
                strcpy_P(buf, PSTR("WIDE1"));
                if (ax25CmpAddr(buf, frame[slot].addr[j+1]) && (frame[slot].addr[j+1][6] > 0) && (j < 8)) {
                    // copy any forward vias to make room for us
                    for (k = frame[slot].hops; k > j; --k) {
                        for (i = 0; i < 7; ++i) frame[slot].addr[k+2][i] = frame[slot].addr[k+1][i];
                    }
                    // copy via address with a decremented SSID to the next via
                    for (i = 0; i < 6; ++i) frame[slot].addr[j+2][i] = frame[slot].addr[j+1][i];
                    frame[slot].addr[j+2][6] = (uint8_t) frame[slot].addr[j+1][6] - 1;
                    // set repeated bit if new WIDE SSID is zero
                    if (frame[slot].addr[j+2][6] == 0) frame[slot].addr[j+2][6] |= 0x80;
                    // put our address in the via w/ repeated flag
                    ax25SetAddr(config.srcCall, (uint8_t) config.srcSSID | 0x80, frame[slot].addr[j+1]);
                    //
                    ++frame[slot].hops;
                    goto chainForwardTx;
                }
            }
            if (config.repeatFlags & CHAIN_REPEATFLAG_WIDE2) {
                // Is this a WIDE1-N w/ a SSID > 0, and do we have via room?
                strcpy_P(buf, PSTR("WIDE2"));
                if (ax25CmpAddr(buf, frame[slot].addr[j+1]) && (frame[slot].addr[j+1][6] > 0) && (j < 8)) {
                    // copy any forward vias to make room for us
                    for (k = frame[slot].hops; k > j; --k) {
                        for (i = 0; i < 7; ++i) frame[slot].addr[k+2][i] = frame[slot].addr[k+1][i];
                    }
                    // copy via address with a decremented SSID to the next via
                    for (i = 0; i < 6; ++i) frame[slot].addr[j+2][i] = frame[slot].addr[j+1][i];
                    frame[slot].addr[j+2][6] = (uint8_t) frame[slot].addr[j+1][6] - 1;
                    // set repeated bit if new WIDE SSID is zero
                    if (frame[slot].addr[j+2][6] == 0) frame[slot].addr[j+2][6] |= 0x80;
                    // put our address in the via w/ repeated flag
                    ax25SetAddr(config.srcCall, (uint8_t) config.srcSSID | 0x80, frame[slot].addr[j+1]);
                    //
                    ++frame[slot].hops;
                    goto chainForwardTx;
                }
            }
        }
        // Otherwise, put frame in purgatory.
        frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
    }
    return;
chainForwardTx:
    // Set frame to transmit.
    frame[slot].lifetime = AX25_FRAME_LIFETIME_TX;
    frame[slot].queueflag |= AX25_QUEUEFLAG_T;
    return;
}

/** Any frames in the ouput chain with a lifetime of zero are transmitted.
 */
void chainOutput (uint8_t slot, uint8_t *buf) {
    int16_t i;
    uint8_t flag;
    //
    flag = frame[slot].queueflag;
    if ((flag & AX25_QUEUEFLAG_E) && (flag & AX25_QUEUEFLAG_T) && !(flag & AX25_QUEUEFLAG_Z) && (frame[slot].lifetime == 0)) {
        chainPrintFrame(stderr, slot, config.debugFlags);
        i = ax25Encode(slot, buf);
        i = kissPut(buf, i & 0x3FF, (uint8_t) 0, &fstrUart1);
        // Refresh the lifetime and put the frame into purgatory.
        frame[slot].lifetime = AX25_FRAME_LIFETIME;
        frame[slot].queueflag |= AX25_QUEUEFLAG_Z;
    }
    return;
}

/** Any frames in the dequeue chain with a lifetime of zero are dequeued.
 *  However, this will affect info data and cause any current thread pointers
 *  to be incorrect. So, wait until all frames are dequeued before releasing.
 */
void chainDequeue (uint8_t slot) {
    uint8_t flag, i;
    //
    flag = frame[slot].queueflag;
    if ((flag & AX25_QUEUEFLAG_E) && (flag & AX25_QUEUEFLAG_Z) && (frame[slot].lifetime == 0)) {
        for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
            if (i == slot) continue;
            flag = frame[i].queueflag;
            if (!(flag & AX25_QUEUEFLAG_E)) continue;
            if (!(flag & AX25_QUEUEFLAG_Z)) break;
        }
        if (i == AX25_FRAME_SLOTS) {
            // Consider the info data corrupted. Do not print.
            chainPrintFrame(stderr, slot, config.debugFlags & ~CHAIN_PRINTFRAME_INFO);
            ax25Free(slot);
        }
    }
    return;
}

/** Prints the formatted contents of a frame. Useful for debugging.
 * 'opts' determines how much frame information is printed.
 */
void chainPrintFrame (FILE *stream, uint8_t slot, uint8_t opts) {
    uint16_t i;
    uint8_t flag;
    char *p, str_buf[12];
    //
    if (opts & CHAIN_PRINTFRAME_E) {
        flag = frame[slot].queueflag;
        strcpy_P(str_buf, PSTR("    "));
        if (flag & AX25_QUEUEFLAG_I) str_buf[0] = 'I';
        if (flag & AX25_QUEUEFLAG_F) str_buf[1] = 'F';
        if (flag & AX25_QUEUEFLAG_T) str_buf[2] = 'T';
        if (flag & AX25_QUEUEFLAG_Z) str_buf[3] = 'Z';
        fprintf_P(stream, PSTR("\n%d[%s]:"), slot, str_buf);
        ax25FmtAddr(str_buf, frame[slot].addr[1]);  // Source
        fputs(str_buf, stream);
        fputc('>', stream);
        ax25FmtAddr(str_buf, frame[slot].addr[0]);  // Destination
        fputs(str_buf, stream);
        if (opts & CHAIN_PRINTFRAME_VIA) {
            for (i = 0; i < frame[slot].hops; ++i) {
                ax25FmtAddr(str_buf, frame[slot].addr[i+2]);
                fputc(',', stream);
                fputs(str_buf, stream);
            }
        }
        if (opts & CHAIN_PRINTFRAME_INFO) {
            fputc(':', stream);
            while (atomMutexGet(&frameinfo.mutex, 0) != ATOM_OK) atomTimerDelay(1);
            p = frame[slot].info;
            for (i = 0; i < frame[slot].infosize; ++i) {
                if (*p != '\r') fputc(*p, stream);
                ++p;
            }
            atomMutexPut(&frameinfo.mutex);
        }
        if (opts & CHAIN_PRINTFRAME_CRC) {
            fprintf_P(stream, PSTR(":%04X"), frame[slot].infocrc);
        }
    }
    return;
}
