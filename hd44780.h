/*
 * hd44780.h
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
 * $Id: hd44780.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup hd44780 <hd44780.h>: HD44780
 * \code #include <hd44780.h> \endcode
 * <h3>Introduction to the HD44780 Module</h3>
 *  
 * These are the HD44780 LCD controller command byte definitions.
 */

#ifndef __HD44780_H__
#define __HD44780_H__

/** \ingroup hd44780 */
/* @{ */
#define HD44780_CLR             (uint8_t) 0x01  ///< Clear the display.
#define HD44780_HOME            (uint8_t) 0x02  ///< Reset display and cursor to origin.
#define HD44780_ENTRYMODE       (uint8_t) 0x04  ///< Determines the display behavior when a byte is written.
#define HD44780_ENTRYMODE_INC   (uint8_t) 0x02  ///< Increment the cursor to next position.
#define HD44780_ENTRYMODE_SHIFT (uint8_t) 0x01  ///< Shift entire display.
#define HD44780_DISPCTRL        (uint8_t) 0x08  ///< Display control command.
#define HD44780_DISPCTRL_ON     (uint8_t) 0x04  ///< Turn display on, otherwise off.
#define HD44780_DISPCTRL_CURSOR (uint8_t) 0x02  ///< Turn cursor underline on, otherwise off.
#define HD44780_DISPCTRL_BLINK  (uint8_t) 0x01  ///< Turn cursor blink on, otherwise off.
#define HD44780_SHIFT           (uint8_t) 0x10  ///< Shift display command.
#define HD44780_SHIFT_DISPLAY   (uint8_t) 0x08  ///< Shift the whole display, otherwise just the cursor.
#define HD44780_SHIFT_RIGHT     (uint8_t) 0x04  ///< Shift right, otherwise left.
#define HD44780_FUNCSET         (uint8_t) 0x20  ///< Display configuration command.
#define HD44780_FUNCSET_8BIT    (uint8_t) 0x10  ///< Use 8-bit data, otherwise 4-bit.
#define HD44780_FUNCSET_2LINE   (uint8_t) 0x08  ///< Configure display for 2-lines.
#define HD44780_FUNCSET_5X10    (uint8_t) 0x04  ///< Use a 5x10 font, otherwise 5x8.
#define HD44780_CGADDR(addr)    (uint8_t) (0x40 | ((addr) & 0x3F))
#define HD44780_DDADDR(addr)    (uint8_t) (0x80 | ((addr) & 0x7F))
#define HD44780_ROW1ADDR        (uint8_t) 0     ///< 1st line starts at 0.
#define HD44780_ROW2ADDR        (uint8_t) 64    ///< 2nd line starts at 64 (0x40).
#define HD44780_ROW3ADDR        LCD_COLUMNS
#define HD44780_ROW4ADDR        (HD44780_ROW2ADDR + LCD_COLUMNS)

/* @} */
#endif /* __HD44780_H__ */
