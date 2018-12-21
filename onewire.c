/*
 * onewire.c
 * 
 * Copyright (c) 2017, Jason Woodford, VO1JWW. All rights reserved.
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
 * $Id: onewire.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "onewire.h"

ONEWIRE     onewire;
ONEWIREDev  onewireDevice[ONEWIRE_MAX_DEV];

/** Writes one byte to the OneWire bus, LSB first.
 */
void onewireWriteByte (uint8_t val) {
    uint8_t i;
    
    for (i = 0; i < 8; ++i) {
        onewireWrite(val & 1);
        val >>= 1;
    }
}

/** Reads one byte from the OneWire bus, LSB first.
 */
uint8_t onewireReadByte (void) {
    uint8_t i;
    uint8_t val;
    
    val = 0;
    for (i = 0; i < 8; ++i) {
        val >>= 1;
        if (onewireRead()) val |= (1 << 7);
    }
    return val;
}

/** Perform the 1-Wire Search Algorithm on the 1-Wire bus. 
 */
int8_t onewireEnumerate (void) {
    uint8_t rom[8];
    uint8_t devIndex;
    int8_t result;
    uint8_t lastDisc;
    uint8_t lastDevice;
    uint8_t lastSearch;
    uint8_t lastZero;
    uint8_t romByteNum;
    uint8_t romByteMask;
    uint8_t idBitNum;
    uint8_t idBitVal;
    uint8_t idBitCmp;
    uint8_t searchDir;
    //uint8_t crc8;
    // Reset the search state.
    devIndex = 0;
    result = 0;
    lastDisc = 0;
    lastDevice = FALSE;
    lastSearch = TRUE;
    //
    while (lastSearch & !lastDevice & (devIndex < ONEWIRE_MAX_DEV)) {
        // initialize for search
        idBitNum = 1;
        lastZero = 0;
        romByteNum = 0;
        romByteMask = 1;
        lastSearch = FALSE;
        //crc8 = 0;
        // reset the bus.
        result = onewireReset();
        if (result < 0) break;
        // issue the search command 
        onewireWriteByte(ONEWIRE_ROMCMD_SEARCH);
        // loop to do the search
        while(romByteNum < 8) {
            // read a bit and its complement
            idBitVal = onewireRead();
            idBitCmp = onewireRead();
            // check for no devices on 1-wire
            if ((idBitVal == 1) && (idBitCmp == 1)) {
                result = ONEWIRE_ERROR_NODEV;
                break;
            } else {
                // all devices coupled have 0 or 1
                if (idBitVal != idBitCmp) {
                    searchDir = idBitVal;  // bit write value for search
                } else {
                    // if this discrepancy is before the Last Discrepancy
                    // on a previous search then pick the same as last time
                    if (idBitNum < lastDisc) {
                        searchDir = ((rom[romByteNum] & romByteMask) > 0);
                    } else {
                        // if equal to last pick 1, if not then pick 0
                        searchDir = (idBitNum == lastDisc);
                    }
                    // if 0 was picked then record its position in LastZero
                    if (searchDir == 0) lastZero = idBitNum;
                }
                // set or clear the bit in the ROM byte romByteNum with mask romByteMask
                if (searchDir == 1) {
                    rom[romByteNum] |= romByteMask;
                } else {
                    rom[romByteNum] &= ~romByteMask;
                }
                // serial number search direction write bit
                onewireWrite(searchDir);
                // increment the byte counter idBitNum and shift the mask romByteMask
                ++idBitNum;
                romByteMask <<= 1;
                // if the mask is 0 then go to new SerialNum byte romByteNum and reset mask
                if (romByteMask == 0) {
                    ++romByteNum;
                    romByteMask = 1;
                }
            }
        }
        //crc8 = crcCalc_8(rom, 8);
        // Was the search successful?
        if (!((idBitNum < 65) /*|| (crc8 != 0)*/)) {
            lastSearch = TRUE;
            lastDisc = lastZero;
            // check for last device
            if (lastDisc == 0) lastDevice = TRUE;
            // Record this in the address table.
            memcpy(onewireDevice[devIndex].rom, rom, 8);
            sprintf_P(onewireDevice[devIndex].id, PSTR("DEV_%02d"), devIndex);
            onewireDevice[devIndex].flags |= ONEWIRE_FLAG_VALID;
            result = ++devIndex;
        }
    }
    return result;
}

/**
 */
int8_t onewireGetIndex (char *str) {
    int8_t i;
    //
    for (i = 0; i < ONEWIRE_MAX_DEV; ++i) {
        if (!(onewireDevice[i].flags & ONEWIRE_FLAG_VALID)) continue;
        if (strncmp(onewireDevice[i].id, str, 10) == 0) return i;
    }
    return ONEWIRE_ERROR_NOID;
}

/**
 */
int8_t onewireMatchRom (uint8_t index) {
    uint8_t i;
    //
    if (!(onewireDevice[index].flags & ONEWIRE_FLAG_VALID)) return ONEWIRE_ERROR_NODEV;
    onewireWriteByte(ONEWIRE_ROMCMD_MATCH);
    for (i = 0; i < 8; ++i) onewireWriteByte(onewireDevice[index].rom[i]);
    return 0;
}

/**
 */
char* onewirePrintError (char *buf, int8_t errno) {
    buf += sprintf_P(buf, PSTR("Onewire: "));
    switch (errno) {
        case ONEWIRE_ERROR_BUS:
            buf += sprintf_P(buf, PSTR("Bus not ready."));
            break;
        case ONEWIRE_ERROR_NODEV:
            buf += sprintf_P(buf, PSTR("No device found."));
            break;
        case ONEWIRE_ERROR_NOID:
            buf += sprintf_P(buf, PSTR("ID not found."));
            break;
        case ONEWIRE_ERROR_NORESP:
            buf += sprintf_P(buf, PSTR("No response."));
            break;
        case ONEWIRE_ERROR_RESP:
            buf += sprintf_P(buf, PSTR("Bad response."));
            break;
    }
    return buf;
}
