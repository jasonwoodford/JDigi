/*
 * i2c.c
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
 * $Id: i2c.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "i2c.h"

/** Send a Slave Address with r/W.
 * Returns 0 on success, or an I2C error number.
 */
uint8_t i2cSendSla (uint8_t sla, uint8_t rw) {
    return i2cWrite((uint8_t) (((sla & 0x7F) << 1) | (rw & 1)));
}

/** Write one byte to I2C device at address 'sla', register 'reg'.
 * Returns 0 on success, or an I2C error number.
 */
uint8_t i2cWriteByte (uint8_t sla, uint8_t reg, uint8_t data) {
    uint8_t stat;
    //
    stat = i2cStart();
    if ((stat != TW_START) && (stat != TW_REP_START)) goto i2cWriteByteError;
    stat = i2cSendSla(sla, TW_WRITE);
    if (stat != TW_MT_SLA_ACK) goto i2cWriteByteError;
    stat = i2cWrite(reg);
    if (stat != TW_MT_DATA_ACK) goto i2cWriteByteError;
    stat = i2cWrite(data);
    if (stat != TW_MT_DATA_ACK) goto i2cWriteByteError;
    return 0;
i2cWriteByteError:
    i2cStop();
    return stat;
}

/** Read a stream of 'n' bytes from I2C device at address 'sla', register 'reg'.
 * Returns 0 on success, or an I2C error number.
 */
uint8_t i2cReadBytes (uint8_t sla, uint8_t reg, uint8_t *data, uint8_t n) {
    uint8_t stat;
    //
    stat = i2cStart();
    if ((stat != TW_START) && (stat != TW_REP_START)) goto i2cReadBytesError;
    stat = i2cSendSla(sla, TW_WRITE);
    if (stat != TW_MT_SLA_ACK) goto i2cReadBytesError;
    stat = i2cWrite(reg);
    if (stat != TW_MT_DATA_ACK) goto i2cReadBytesError;
    stat = i2cStart();
    if ((stat != TW_START) && (stat != TW_REP_START)) goto i2cReadBytesError;
    stat = i2cSendSla(sla, TW_READ);
    if (stat != TW_MR_SLA_ACK) goto i2cReadBytesError;
    while (n > 1) {
        stat = i2cRead(data, 1);
        if (stat != TW_MR_DATA_ACK) goto i2cReadBytesError;
        ++data;
        --n;
    }
    stat = i2cRead(data, 0);
    if (stat != TW_MR_DATA_NACK) goto i2cReadBytesError;
    return 0;
i2cReadBytesError:
    i2cStop();
    return stat;
}
