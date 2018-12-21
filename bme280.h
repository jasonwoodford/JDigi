/*
 * bme280.h
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
 * $Id: bme280.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup bme280 <bme280.h>: BME280
 * \code #include <bme280.h> \endcode
 * <h3>Introduction to the BME280 Module</h3>
 *  
 * This module uses the I2C subsystem to communicate with a BME280 sensor.
 */

#ifndef __BME280_H__
#define __BME280_H__

#include "atom.h"
#include "i2c.h"

/** \ingroup bme280 */
/* @{ */
#define BME280_I2C_ADDRESS  0x76

#define BME280_REG_CALIB0   0x88
#define BME280_REG_CALIB1   0xA1
#define BME280_REG_ID       0xD0
#define BME280_REG_RESET    0xE0
#define BME280_REG_CALIB2   0xE1
#define BME280_REG_CTRL_H   0xF2
#define BME280_REG_STATUS   0xF3
#define BME280_REG_CTRL_M   0xF4
#define BME280_REG_CONFIG   0xF5
#define BME280_REG_P_MSB    0xF7
#define BME280_REG_P_LSB    0xF8
#define BME280_REG_P_XLSB   0xF9
#define BME280_REG_T_MSB    0xFA
#define BME280_REG_T_LSB    0xFB
#define BME280_REG_T_XLSB   0xFC
#define BME280_REG_H_MSB    0xFD
#define BME280_REG_H_LSB    0xFE

#define BME280_ID           0x60
#define BME280_RESET_KEY    0xB6
#define BME280_CTRL_H_OS0   0b00000000
#define BME280_CTRL_H_OS1   0b00000001
#define BME280_CTRL_H_OS2   0b00000010
#define BME280_CTRL_H_OS4   0b00000011
#define BME280_CTRL_H_OS8   0b00000100
#define BME280_CTRL_H_OS16  0b00000101
#define BME280_STATUS_MEAS  0b00001000
#define BME280_STATUS_IMUPD 0b00000001
#define BME280_CTRL_M_TOS0  0b00000000
#define BME280_CTRL_M_TOS1  0b00100000
#define BME280_CTRL_M_TOS2  0b01000000
#define BME280_CTRL_M_TOS4  0b01100000
#define BME280_CTRL_M_TOS8  0b10000000
#define BME280_CTRL_M_TOS16 0b10100000
#define BME280_CTRL_M_POS0  0b00000000
#define BME280_CTRL_M_POS1  0b00000100
#define BME280_CTRL_M_POS2  0b00001000
#define BME280_CTRL_M_POS4  0b00001100
#define BME280_CTRL_M_POS8  0b00010000
#define BME280_CTRL_M_POS16 0b00010100
#define BME280_CTRL_M_SLEEP 0b00000000
#define BME280_CTRL_M_FORCE 0b00000010
#define BME280_CTRL_M_NORM  0b00000011
#define BME280_CONFIG_SB0   0b00000000  // 0.5 ms
#define BME280_CONFIG_SB1   0b00100000  // 62.5 ms
#define BME280_CONFIG_SB2   0b01000000  // 125 ms
#define BME280_CONFIG_SB3   0b01100000  // 250 ms
#define BME280_CONFIG_SB4   0b10000000  // 500 ms
#define BME280_CONFIG_SB5   0b10100000  // 1000 ms
#define BME280_CONFIG_SB6   0b11000000  // 10 ms
#define BME280_CONFIG_SB7   0b11100000  // 20 ms
#define BME280_CONFIG_FIL0  0b00000000
#define BME280_CONFIG_FIL2  0b00000100
#define BME280_CONFIG_FIL4  0b00001000
#define BME280_CONFIG_FIL8  0b00001100
#define BME280_CONFIG_FIL16 0b00010000
#define BME280_CONFIG_SPI3W 0b00000001

// I2C status values in the range 0xF9 to 0xFF are never used, so we can use
// this range to return errors.
#define BME280_ERROR_BADDEV 0xF9

typedef struct BME280 {
    int32_t  rawTemp;
    uint32_t rawPres;
    uint32_t rawHumd;
    int32_t  tempFine;
	uint16_t dig_T1;
	int16_t  dig_T2;
	int16_t  dig_T3;
	uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280;

uint8_t  bme280Init (uint8_t sla);
uint8_t  bme280ReadEnviro (uint8_t sla);
double   bme280CompTemp (void);
double   bme280CompPres (void);
double   bme280CompHumd (void);

/* @} */
#endif /* __BME280_H__ */
