/*
 * lcd.h
 * 
 * Copyright (c) 2016, Jason Woodford, VO1JWW. All rights reserved.
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
 * $Id: lcd.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup lcd <lcd.h>: LCD
 * \code #include <lcd.h> \endcode
 * <h3>Introduction to the LCD Module</h3>
 *  
 * This defines LCD functions for applications.
 */

#ifndef __LCD_H_
#define __LCD_H_

#include "atom.h"
#include "atommutex.h"
#include "atomqueue.h"
#include "aprs.h"
#include "time.h"
#include <stdio.h>

/** \ingroup lcd */
/* @{ */
#define LCD_ROWS            2     ///< Display row size.
#define LCD_COLUMNS         16    ///< Display column size.
#define LCD_MENU_TIMEOUT    15    ///< Menu timeout in seconds.
#define LCD_MESSAGE_TIMEOUT 30    ///< Message timeout in seconds.
#define LCD_MESSAGE_DELAY   5     ///< Message line advance delay.
#define LCD_MESSAGE_SIZE    64
#define LCD_STATS_DELAY     30
#define LCD_VIEWPORT_SIZE   (LCD_ROWS * LCD_COLUMNS)

#define LCD_FLAGS_BACKLIGHT (1 << 7)
#define LCD_FLAGS_MENU      (1 << 5)
#define LCD_FLAGS_MESSAGE   (1 << 4)
#define LCD_FLAGS_MODE_CLK  0
#define LCD_FLAGS_MODE_STAT 1
#define LCD_FLAGS_MODE_ENV  2
#define LCD_FLAGS_MODE_DBG  3
#define LCD_FLAGS_MODE_SIZE 4     ///< "Last"
#define LCD_FLAGS_MODE_MASK 0x07  ///< 0 to 7.

#define LCD_REFRESH_TICKS   (SYSTEM_TICKS_PER_SEC / 10) ///< 0.1 seconds.

#define LCD_SYM_DEGREE      0xDF

typedef struct LCD {
    uint8_t flags;
    uint8_t curpos;
    time_t  timeout;
    uint8_t viewport[LCD_VIEWPORT_SIZE];    ///< "memory-mapped" viewable buffer
    uint8_t msgqueuebuf[LCD_MESSAGE_SIZE];  ///< Message queue buffer
    char    message[LCD_MESSAGE_SIZE];      ///< Thread buffer
    ATOM_MUTEX mutex;
    ATOM_QUEUE msgqueue;
} LCD;

/* LCD function declarations. */

void lcdInit (void);
void lcdClear (void);
void lcdSetLine (uint8_t n);
int  lcdPutchar (uint8_t c, FILE *stream);
void lcdSetCurpos (uint8_t pos);
void lcdUpdate (void);

/* @} */
#endif /* __LCD_H_ */
