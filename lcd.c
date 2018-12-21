/*
 * lcd.c
 * 
 * Upper-level layer of LCD driver.
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
 * $Id: lcd.c,v 1.0 2018/12/18 21:12:00 NSTjason_woodford Exp $
 */
#include "lcd.h"

LCD lcd;

extern APRSConfig config;
extern FILE fstrLcd;

/** Initialize the LCD buffer.
 */
void lcdInit (void) {
    lcd.flags = config.lcdMode & LCD_FLAGS_MODE_MASK;
    if (config.lcdMode == LCD_FLAGS_MODE_DBG) stderr = &fstrLcd;
    lcd.timeout = time(NULL);
    lcdClear();
    atomQueueCreate(&lcd.msgqueue, (uint8_t *) lcd.msgqueuebuf, LCD_MESSAGE_SIZE, 1);
    // Create the byte-atomic mutex.
    atomMutexCreate(&lcd.mutex);
}

/** Clear LCD viewport buffer and reset cursor position to zero.
 */
void lcdClear (void) {
    uint8_t i;
    
    for (i = 0; i < LCD_VIEWPORT_SIZE; ++i) lcd.viewport[i] = ' ';
    lcd.curpos = 0;
}

/** Clear line 'n' of viewport buffer and position cursor at start of line.
 * 
 * If 'n' equals 0, clear the current line.
 */
void lcdSetLine (uint8_t n) {
    uint8_t i;
    
    if ((n < 1) || (n > LCD_ROWS)) n = (lcd.curpos / LCD_COLUMNS) + 1;
    lcd.curpos = (n - 1) * LCD_COLUMNS;
    for (i = lcd.curpos; i < lcd.curpos + LCD_COLUMNS; ++i) lcd.viewport[i] = ' ';
}

/** Put character 'c' into the LCD viewport buffer.
 * 
 * '\n' puts the cursor to the next line. If the cursor position is larger
 * than the screen size, scroll the viewport and set the cursor to the
 * beginning of the last line.
 */
int lcdPutchar (uint8_t c, FILE *unused) {
    uint8_t flag;
    uint8_t i;
    
    flag = TRUE;
    if (lcd.curpos >= LCD_VIEWPORT_SIZE) {
        // shift all lines up one, destroy first line.
        for (i = 0; i < (LCD_ROWS - 1) * LCD_COLUMNS; ++i) {
            lcd.viewport[i] = lcd.viewport[i + LCD_COLUMNS];
        }
        // clear the last line and set cursor.
        lcdSetLine(LCD_ROWS);
    }
    if (c == '\n') {
        lcd.curpos = ((lcd.curpos / LCD_COLUMNS) + 1) * LCD_COLUMNS;
        flag = FALSE;
    }
    if (flag) {
        lcd.viewport[lcd.curpos] = c;
        ++lcd.curpos;
    }
    return 0;
}

/** Update cursor position in DDRAM based on 'pos'.
 * 
 * This incorporates the proper cursor repositioning for an HD44780
 * display. If 'pos' is beyond the display's size, set to zero.
 */
void lcdSetCurpos (uint8_t pos) {
    uint8_t i, j;
    
    if (pos >= LCD_VIEWPORT_SIZE) pos = 0;
    lcd.curpos = pos;
    i = pos / LCD_COLUMNS;
    j = pos - (i * LCD_COLUMNS);
    switch (i) {
        case 0:
            j += HD44780_ROW1ADDR;
            break;
        case 1:
            j += HD44780_ROW2ADDR;
            break;
        case 2:
            j += HD44780_ROW3ADDR;
            break;
        case 3:
            j += HD44780_ROW4ADDR;
    }
    lcdshieldOutcmd(HD44780_DDADDR(j));
}

/** Update the LCD with the content of the viewport buffer.
 * 
 */
void lcdUpdate (void) {
    uint8_t i;
    
    for (i = 0; i < LCD_VIEWPORT_SIZE; ++i) {
        lcdSetCurpos(i);
        lcdshieldOutdata(lcd.viewport[i]);
    }
}
