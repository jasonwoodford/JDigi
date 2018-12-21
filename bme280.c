/*
 * bme280.c
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
 * $Id: bme280.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "bme280.h"

BME280 enviroBME280;

/** Initialize the BME280 sensor.
 */
uint8_t bme280Init (uint8_t sla) {
    uint8_t stat;
    uint8_t buf[26];
    // Is this a BME280 device?
    stat = i2cReadBytes(sla, BME280_REG_ID, buf, 1);
    if (stat > 0) goto bme280InitError;
    if (buf[0] != BME280_ID) { i2cStop(); return BME280_ERROR_BADDEV; } 
    // Do a burst read of temperature and pressure compensation values.
    stat = i2cReadBytes(sla, BME280_REG_CALIB0, buf, 26);
    if (stat > 0) goto bme280InitError;
    enviroBME280.dig_T1 = (uint16_t) ((buf[1] << 8) | buf[0]);
    enviroBME280.dig_T2 = (int16_t) ((buf[3] << 8) | buf[2]);
    enviroBME280.dig_T3 = (int16_t) ((buf[5] << 8) | buf[4]);
    enviroBME280.dig_P1 = (uint16_t) ((buf[7] << 8) | buf[6]);
    enviroBME280.dig_P2 = (int16_t) ((buf[9] << 8) | buf[8]);
    enviroBME280.dig_P3 = (int16_t) ((buf[11] << 8) | buf[10]);
    enviroBME280.dig_P4 = (int16_t) ((buf[13] << 8) | buf[12]);
    enviroBME280.dig_P5 = (int16_t) ((buf[15] << 8) | buf[14]);
    enviroBME280.dig_P6 = (int16_t) ((buf[17] << 8) | buf[16]);
    enviroBME280.dig_P7 = (int16_t) ((buf[19] << 8) | buf[18]);
    enviroBME280.dig_P8 = (int16_t) ((buf[21] << 8) | buf[20]);
    enviroBME280.dig_P9 = (int16_t) ((buf[23] << 8) | buf[22]);
    // Read H1.
    enviroBME280.dig_H1 = (uint8_t) buf[25];
    // Read H2 to H6.
    stat = i2cReadBytes(sla, BME280_REG_CALIB2, buf, 7);
    if (stat > 0) goto bme280InitError;
    enviroBME280.dig_H2 = (int16_t) ((buf[1] << 8) | buf[0]);
    enviroBME280.dig_H3 = (uint8_t) buf[2];
    enviroBME280.dig_H4 = (int16_t) ((buf[3] << 4) | (buf[4] & 0x0F));
    enviroBME280.dig_H5 = (int16_t) (((buf[4] >> 4) & 0x0F) | (buf[5] << 4));
    enviroBME280.dig_H6 = (int8_t) buf[6];
    return 0;
bme280InitError:
    i2cStop();
    return stat;
}

/** Read all environmental values and store raw values.
 */
uint8_t bme280ReadEnviro (uint8_t sla) {
    uint8_t stat;
    uint8_t buf[8];
    // Tell the BME280 to do a forced read.
    stat = i2cWriteByte(sla, BME280_REG_CTRL_H, BME280_CTRL_H_OS16);
    if (stat > 0) goto bme280ReadEnviroError;
    stat = i2cWriteByte(sla, BME280_REG_CTRL_M, BME280_CTRL_M_POS16 | BME280_CTRL_M_TOS16 | BME280_CTRL_M_FORCE);
    if (stat > 0) goto bme280ReadEnviroError;
    // Wait until BME280 has finished measuring.
    buf[0] = (BME280_STATUS_IMUPD | BME280_STATUS_MEAS);
    while (buf[0] & (BME280_STATUS_IMUPD | BME280_STATUS_MEAS)) {
        stat = i2cReadBytes(sla, BME280_REG_STATUS, buf, 1);
        if (stat > 0) goto bme280ReadEnviroError;
    }
    // Do a burst read of all environmental values.
    stat = i2cReadBytes(sla, BME280_REG_P_MSB, buf, 8);
    if (stat > 0) goto bme280ReadEnviroError;
    enviroBME280.rawPres = (uint32_t) ((buf[0] << 12) | (buf[1] << 4) | ((buf[2] >> 4) & 0x0F));
    enviroBME280.rawTemp = (int32_t) ((buf[3] << 12) | (buf[4] << 4) | ((buf[5] >> 4) & 0x0F));
    enviroBME280.rawHumd = (uint32_t) ((buf[6] << 8) | buf[7]);
    return 0;
bme280ReadEnviroError:
    i2cStop();
    return stat;
}

/** Returns temperature in 0.01 DegC. Output value of “5123” equals 51.23 DegC.
 */
double bme280CompTemp (void) {
    double var1, var2;
    var1 = (((double)enviroBME280.rawTemp)/16384.0 - ((double)enviroBME280.dig_T1)/1024.0) * ((double)enviroBME280.dig_T2);
    var2 = ((((double)enviroBME280.rawTemp)/131072.0 - ((double)enviroBME280.dig_T1)/8192.0) * (((double)enviroBME280.rawTemp)/131072.0 - ((double) enviroBME280.dig_T1)/8192.0)) * ((double)enviroBME280.dig_T3);
    enviroBME280.tempFine = (int32_t)(var1 + var2);
    return (double)(var1 + var2) / 5120.0;
}

/** Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
 */
double bme280CompPres (void) {
    double var1, var2, p;
    var1 = ((double)enviroBME280.tempFine/2.0) - 64000.0;
    var2 = var1 * var1 * ((double)enviroBME280.dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)enviroBME280.dig_P5) * 2.0;
    var2 = (var2/4.0)+(((double)enviroBME280.dig_P4) * 65536.0);
    var1 = (((double)enviroBME280.dig_P3) * var1 * var1 / 524288.0 + ((double)enviroBME280.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)enviroBME280.dig_P1);
    if (var1 == 0.0) return 0; // avoid exception caused by division by zero.
    p = 1048576.0 - (double)enviroBME280.rawPres;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)enviroBME280.dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)enviroBME280.dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)enviroBME280.dig_P7)) / 16.0;
    return p;
}

/* Returns humidity in %rH as as double. Output value of “46.332” represents 46.332 %rH
 */
double bme280CompHumd (void) {
    double h;
    h = (((double)enviroBME280.tempFine) - 76800.0);
    h = (enviroBME280.rawHumd - (((double)enviroBME280.dig_H4) * 64.0 + ((double)enviroBME280.dig_H5) / 16384.0 * h)) * (((double)enviroBME280.dig_H2) / 65536.0 * (1.0 + ((double)enviroBME280.dig_H6) / 67108864.0 * h * (1.0 + ((double)enviroBME280.dig_H3) / 67108864.0 * h)));
    h = h * (1.0 - ((double)enviroBME280.dig_H1) * h / 524288.0);
    if (h > 100.0) h = 100.0;
    else if (h < 0.0) h = 0.0;
    return h;
}
