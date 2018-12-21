/*
 * i2c.h
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
 * $Id: i2c.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup i2c <i2c.h>: I2C
 * \code #include <i2c.h> \endcode
 * <h3>Introduction to the I2C Module</h3>
 *  
 * This is JDigi's I2C module.
 */

#ifndef __I2C_H__
#define __I2C_H__

#include "atom.h"
#include <util/twi.h>

/** \ingroup i2c */
/* @{ */
#define I2C_BITRATE     10000UL
#define I2C_WAIT_TICKS  20

uint8_t i2cSendSla (uint8_t sla, uint8_t rw);
uint8_t i2cWriteByte (uint8_t sla, uint8_t reg, uint8_t data);
uint8_t i2cReadBytes (uint8_t sla, uint8_t reg, uint8_t *data, uint8_t n);

/* Architecture dependent prototypes. */

/** Architecture dependent I2C initialization.
 * 
 */
void i2cInit (void);

/** Architecture dependent I2C wait function.
 * Wait for an I2C response.
 * Returns an I2C status.
 */
uint8_t i2cWait (void);

/** Architecture dependent I2C Start sequence.
 * Send an I2C START.
 * Returns an I2C status.
 */
uint8_t i2cStart (void);

/** Architecture dependent I2C Stop sequence.
 * Send an I2C STOP.
 */
void i2cStop (void);

/** Architecture dependent I2C byte write.
 * Write 'data' to the I2C bus.
 * Returns an I2C status.
 */
uint8_t i2cWrite (uint8_t data);

/** Architecture dependent I2C byte read.
 * Read one byte into *data. Send an ACK if ack is TRUE.
 * Returns an I2C status.
 */
uint8_t i2cRead (uint8_t *data, uint8_t ack);

/* @} */
#endif /* __I2C_H__ */
