/*
 * ax25.c
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
 * $Id: ax25.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "ax25.h"

AX25Frame frame[AX25_FRAME_SLOTS];
AX25FrameInfo frameinfo;

/** Initialize the frame table.
 *
 */
int8_t ax25InitQueue (void) {
    int i;
    //
    frameinfo.ticks = AX25_FRAME_TICKS;
    for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
        frame[i].queueflag = 0;
        frame[i].infosize = 0;
    }
    frameinfo.free = AX25_FRAME_INFOBUFSIZE;
    if (atomMutexCreate(&frameinfo.mutex) != ATOM_OK) return AX25_ERROR_MUTEX;
    return i;
}

/** Allocate an empty frame slot.
 * 
 * Returns the slot index, or an error.
 */
int8_t ax25Alloc (void) {
    int8_t i;
    //
    while (atomMutexGet(&frameinfo.mutex, 10) != ATOM_OK) atomTimerDelay(1);
    for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
        if ((frame[i].queueflag & AX25_QUEUEFLAG_USED) == 0) {
            frame[i].queueflag |= AX25_QUEUEFLAG_USED;
            frame[i].infosize = 0;
            frame[i].lifetime = AX25_FRAME_LIFETIME;
            break;
        }
    }
    atomMutexPut(&frameinfo.mutex);
    if (i == AX25_FRAME_SLOTS) return AX25_ERROR_NOSLOTS;
    return i;
}

/** Decodes a raw AX25 frame in 'buf', and populates frame[slot].
 * 
 * Returns the number of info bytes, if any.
 */
int16_t ax25Decode (uint8_t slot, uint8_t *buf, uint16_t len) {
    int16_t i;
    uint8_t *p;
    uint8_t lastaddr;
    //
    i = 0;
    p = buf;
    lastaddr = 0;
    // Note that a KISS frame does NOT include AX.25 frame flags or FCS.
    // destination address
    if (ax25GetAddr(p, frame[slot].addr[0]) != 6) return AX25_ERROR_DSTADDR;
    // destination SSID
    p += 6;
    if (*p & 1) return AX25_ERROR_DSTADDR;
    if (*p & 0x80) i = 1; // assume command for now
    frame[slot].addr[0][6] = (*p & 0x1E) >> 1;
    ++p;
    // source address
    if (ax25GetAddr(p, frame[slot].addr[1]) != 6) return AX25_ERROR_SRCADDR;
    // source SSID
    p += 6;
    if (*p & 1) lastaddr = 1;
    if (i == 1) {
        if (*p & 0x80) i = 0; // pre v.2.0
    } else {
        if (*p & 0x80) i = 2; // response
    }
    frame[slot].cmdresp = (uint8_t) i;
    frame[slot].addr[1][6] = (*p & 0x1E) >> 1;
    ++p;
    // now get our via path(s), if exists
    i = 2;
    while (lastaddr == 0) {
        if (i == 10) return AX25_ERROR_OVADDR;
        if (ax25GetAddr(p, frame[slot].addr[i]) != 6) return AX25_ERROR_VIAADDR;
        // SSID
        p += 6;
        if (*p & 1) lastaddr = 1;
        // preserve the MSB for has repeated flag in SSID byte!
        frame[slot].addr[i][6] = (*p & 0x80) | ((*p & 0x1E) >> 1);
        ++p;
        ++i;
    }
    frame[slot].hops = (uint8_t) i - 2;
    // Control byte
    frame[slot].ctrl = *p;
    i = 0;
    if (((*p & 0b00000001) == 0) || ((*p & 0b11101110) == 0b00000010)) {
        // I-frames _or_ UI-frames
        ++p;
        frame[slot].pid = *p;
        i = len - (17 + (7 * (frame[slot].hops)));
    } else if ((frame[slot].ctrl & 0b11101110) == 0b10000110) {
        // FRMR U-frame
        i = len - (16 + (7 * (frame[slot].hops))); // _should_ be 3.
    }
    ++p;
    i = ax25SetInfo(slot, (char *) p, (uint16_t) i);
    if (i < 0) return i;
    frame[slot].queueflag |= (AX25_QUEUEFLAG_E | AX25_QUEUEFLAG_I);
    return i;
}

/** Encodes a raw (no flags or FCS) AX25 frame into 'buf' from the contents of frame[slot].
 *
 * Returns the length of the resulting frame, or -1 if an error.
 */
int16_t ax25Encode (uint8_t slot, uint8_t *buf) {
    int16_t len;
    uint8_t *p;
    char    *q;
    uint16_t i, j, k;
    //
    p = buf;
    // dest address
    for (i = 0; i < 6; ++i) *p++ = frame[slot].addr[0][i] << 1;
    *p = (frame[slot].addr[0][i] << 1) | 0b01100000;
    if (frame[slot].cmdresp == 1) *p |= 0b10000000;
    ++p;
    // source address
    for (i = 0; i < 6; ++i) *p++ = frame[slot].addr[1][i] << 1;
    *p = (frame[slot].addr[1][i] << 1) | 0b01100000;
    if (frame[slot].cmdresp == 2) *p |= 0b10000000;
    if (frame[slot].hops == 0) *p |= 0b00000001;
    ++p;
    // via addresses
    j = 2;
    k = frame[slot].hops + 2;
    while (j < k) {
        for (i = 0; i < 6; ++i) *p++ = frame[slot].addr[j][i] << 1;
        // SSID with has-repeated and last address
        *p = ((frame[slot].addr[j][i] & 0b00001111) << 1) | (frame[slot].addr[j][i] & 0b10000000) | 0b01100000;
        ++j;
        if (j == k) *p |= 0b00000001;
        ++p;
    }
    // control field
    *p = frame[slot].ctrl;
    len = (7 * k) + 1;
    if (((*p & 0b00000001) == 0) || ((*p & 0b11101110) == 0b00000010)) {
        // insert PID for I-frames _or_ UI-frames
        ++p;
        *p = frame[slot].pid;
        ++len;
    }
    ++p;
    // information field. Do not use ax25GetInfo(); it uses \0 termination.
    while (atomMutexGet(&frameinfo.mutex, 10) != ATOM_OK) atomTimerDelay(1);
    j = frame[slot].infosize;
    q = frame[slot].info;
    for (i = 0; i < j; ++i) *p++ = *q++;
    atomMutexPut(&frameinfo.mutex);
    return (len + j);
}

/** Decodes a frame address field from 'buf' into 'addr'.
 * 
 * Returns less than 6 if an error.
 */
uint8_t ax25GetAddr (uint8_t *buf, uint8_t *addr) {
    uint8_t i;
    
    for (i = 0; i < 6; ++i) if (!(*addr++ = ax25CheckAddrc(*buf++))) break;
    return i;
}

/** Verifies if the byte in 'c' (from a raw AX25 frame) is a valid address byte.
 *
 * Returns the byte, or 0 if an error.
 */
uint8_t ax25CheckAddrc (uint8_t c) {
    // bit 0 must be clear
    if (c & 1) {
        c = 0;
    } else {
        c = c >> 1;
        // must be [0-9] or [A-Z] or space
        if (!(((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || (c = ' '))) c = 0;
    }
    return c;
}

/** Generates a printable AX25 address field from 'addr' into 'buf'.
 * 
 * 'addr' typically points to an AX25Frame address record.
 * Returns the number of chars in 'buf'.
 */
uint8_t ax25FmtAddr (char *buf, uint8_t *addr) {
    uint8_t i;
    uint8_t len;
    
    len = 0;
    for (i = 0; i < 6; ++i) {
        if (*addr != ' ') {
            *buf++ = *addr;
            ++len;
        }
        ++addr;
    }
    i = (uint8_t) *addr & 0x0F;
    if (i > 0) {
        *buf++ = '-';
        if (i > 9) {
            *buf++ = '1';
            i -= 10;
            ++len;
        }
        *buf++ = i + '0';
        len += 2;
    }
    if (*addr & 0x80) {
        *buf++ = '*';
        ++len;   
    }
    *buf = 0;
    return len;
}

/** Compare the address in 'buf' to 'addr'; SSID not included.
 * 
 * 'buf' must be null-terminated. If 'buf' ends first, good.
 * If 'addr' ends first, error.
 * Returns 1 if a match, 0 if not.
 */
uint8_t ax25CmpAddr (char *buf, uint8_t *addr) {
    uint8_t i;
    //
    for (i = 0; i < 6; ++i) {
        if (*buf == 0) break;
        if (*buf != *addr) return 0;
        if (*buf == ' ') break;
        ++buf;
        ++addr;
    }
    return 1;
}

/** Set frame address.
 * 
 * Puts a null terminated string (up to 6 bytes) from 'buf'
 * into an AX.25 frame slot padded with spaces as needed
 * and appends the ssid byte.
 */
uint8_t ax25SetAddr (char *buf, uint8_t ssid, uint8_t *addr) {
    char    c;
    uint8_t i;
    //
    c = 0;
    for (i = 0; i < 6; ++i) {
        if ((*buf == 0) || (c == ' ')) {
            c = ' ';
        } else {
            c = *buf++;
        }
        *addr++ = c;
    }
    *addr = ssid;
    return 0;
}

/** Sets the frame info data for a frame.
 * 
 * 'len' is the size of data to set.
 * Returns the bytes set, or an error.
 */
int16_t ax25SetInfo (uint8_t slot, char *buf, uint16_t len) {
    int16_t i;
    char    *p;
    //
    ax25FreeInfo(slot);
    while (atomMutexGet(&frameinfo.mutex, 10) != ATOM_OK) atomTimerDelay(1);
    if (len > frameinfo.free) return AX25_ERROR_INFOBUF;
    frameinfo.free -= len;
    p = &frameinfo.info[frameinfo.free];
    frame[slot].info = p;
    frame[slot].infosize = len;
    for (i = 0; i < len; ++i) *p++ = *buf++;
    frame[slot].infocrc = crcCalc_16((uint8_t *) frame[slot].info, len);
    atomMutexPut(&frameinfo.mutex);
    return i;
}

/** Copy frame info.
 * 
 * Puts a copy of a frame's info data into 'buf'.
 * 'len' is the size of 'buf', and must be one byte
 * larger than the info data. Returns the size of data
 * copied (including null termination) or -1 if len is
 * too small.
 */
int16_t ax25GetInfo (uint8_t slot, char *buf, uint16_t len) {
    int16_t i;
    char    *p;
    //
    if (len <= frame[slot].infosize) return -1;
    p = frame[slot].info;    
    while (atomMutexGet(&frameinfo.mutex, 10) != ATOM_OK) atomTimerDelay(1);
    for (i = 0; i < frame[slot].infosize; ++i) *buf++ = *p++;
    *buf = 0;
    atomMutexPut(&frameinfo.mutex);
    return i;
}

/** Deallocate frame info.
 * 
 * De-allocates the frame's info and reorganizes the frameinfo buffer.
 */
void ax25FreeInfo (uint8_t slot) {
    uint8_t  i;
    uint16_t j;
    char     *p, *q;
    //
    j = frame[slot].infosize;
    if (j == 0) return;
    while (atomMutexGet(&frameinfo.mutex, 10) != ATOM_OK) atomTimerDelay(1);
    p = frame[slot].info;
    for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
        if (i == slot) continue;
        if (frame[i].queueflag & AX25_QUEUEFLAG_USED) {
            q = frame[i].info;
            if (p > q) frame[i].queueflag |= AX25_QUEUEFLAG_MOVE;
        }
    }
    p = frame[slot].info + j - 1;
    q = frame[slot].info - 1;
    for (i = 0; i < j; ++i) *p-- = *q--;
    for (i = 0; i < AX25_FRAME_SLOTS; ++i) {
        if (frame[i].queueflag & AX25_QUEUEFLAG_MOVE) {
            frame[i].info += j;
            frame[i].queueflag &= ~AX25_QUEUEFLAG_MOVE;
        }
    }
    frameinfo.free += j;
    frame[slot].infosize = 0;
    atomMutexPut(&frameinfo.mutex);
}

/** Deallocate AX25 frame.
 * 
 * De-allocates a AX25 frame completely.
 */
void ax25Free (uint8_t slot) {
    ax25FreeInfo(slot);
    frame[slot].queueflag = 0;
}

/** Frame timer. 
 * 
 * Reduces the lifetime of the global frame timer by one tick until zero.
 * This is called from ISR (TIMER1_COMPA_vect), the system tick interrupt.
 * See atomport.c
 */
void ax25Tick (void) {
    if (frameinfo.ticks > 0) --frameinfo.ticks;
}
