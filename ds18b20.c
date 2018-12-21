/*
 * ds18b20.c
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
 * $Id: ds18b20.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "ds18b20.h"

DS18B20 ds18b20[DS18B20_MAX_DEV];

extern ONEWIRE onewire;
extern ONEWIREDev onewireDevice[];

/** Find all DS18B20 devices and rename their ID's.
 * Returns the number of DS18B20 devices found.
 */
uint8_t ds18b20Init (void) {
    uint8_t val, i;
    //
    val = 0;
    for (i = 0; i < DS18B20_MAX_DEV; ++i) ds18b20[i].flags = 0;
    for (i = 0; (i < onewire.devcount) && (val < DS18B20_MAX_DEV); ++i) {
        if ((onewireDevice[i].flags & ONEWIRE_FLAG_VALID) && (onewireDevice[i].rom[0] == DS18B20_FAMILY)) {
            ds18b20[val].flags |= DS18B20_FLAG_VALID;
            ds18b20[val].onewireIndex = i;
            sprintf_P(onewireDevice[i].id, PSTR("18B20_%02d"), val);
            ++val;
        }
    }
    return val;
}

/** Get a DS18B20 data index.
 * Uses a OneWire data index.
 */
int8_t ds18b20GetIndex (uint8_t owindex) {
    int8_t val;
    
    for (val = 0; val < DS18B20_MAX_DEV; ++val) {
        if ((ds18b20[val].onewireIndex == owindex) && (ds18b20[val].flags & DS18B20_FLAG_VALID)) break;
    }
    if (val == DS18B20_MAX_DEV) val = DS18B20_ERROR_DEV;
    return val;
}

/** Load a DS18B20 data index with current temperature.
 * Uses a OneWire device ID string.
 */
int8_t ds18b20GetTemp (char *devid) {
    int8_t i, j;
    //
    i = onewireGetIndex(devid);
    if (i < 0) return i;
    if (onewireDevice[i].rom[0] != DS18B20_FAMILY) return DS18B20_ERROR_FAM;
    j = ds18b20GetIndex(i);
    if (j < 0) return j;
    //
    onewireMatchRom(i);
    onewireWriteByte(DS18B20_ROMCMD_CONV);
    while (!onewireRead());
    onewireReset();
    onewireMatchRom(i);
    onewireWriteByte(DS18B20_ROMCMD_RSP);
    for (i = 0; i < 9; ++i) ds18b20[j].sp[i] = onewireReadByte();
    onewireReset();
    return j;
}

/** Converts the existing temperature data to a floating point output.
 * Uses a OneWire device ID string and a unit option.
 */
double ds18b20ConvTemp (char *devid, uint8_t unit) {
    int8_t i, j;
    float  temp;
    //
    i = onewireGetIndex(devid);
    if (i < 0) return 0.0f;
    j = ds18b20GetIndex(i);
    if (j < 0) return 0.0f;
    //
    if (ds18b20[j].sp[1] & (1 << 7)) {
        temp = -1.0f * ((float) (((~ds18b20[j].sp[1] & 0x07) << 4) | ((~ds18b20[j].sp[0] & 0xF0) >> 4)) + ((float) (~ds18b20[j].sp[0] & 0x0F) / 16.0f));
    } else {
        temp = (float) (((ds18b20[j].sp[1] & 0x07) << 4) | ((ds18b20[0].sp[j] & 0xF0) >> 4)) + ((float) (ds18b20[j].sp[0] & 0x0F) / 16.0f);
    }
    switch (unit) {
        case DS18B20_UNIT_F:
            temp = (temp * 9.0f / 5.0f) + 32.0f;
            break;
        case DS18B20_UNIT_K:
            temp += 273.15f;
            break;
    }
    return temp;
}

/**
 */
char* ds18b20PrintError (char *buf, int8_t errno) {
    buf += sprintf_P(buf, PSTR("DS18B20: "));
    switch (errno) {
        case DS18B20_ERROR_DEV:
            buf += sprintf_P(buf, PSTR("Device not found."));
            break;
        case DS18B20_ERROR_FAM:
            buf += sprintf_P(buf, PSTR("Bad device family."));
            break;
        default:
            buf = onewirePrintError(buf, errno);
    }
    return buf;
}
