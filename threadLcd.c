/*
 * threadLcd.c
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
 * $Id: threadLcd.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"

static char const pstrThreadLcdNA[] PROGMEM = "N/A";

extern FILE     fstrLcd;
extern LCD      lcd;
extern Menu     menu;
extern Stats    statTable[STATS_SLOTS];
extern APRSPosit statPosit[STATS_SLOTS];

/** LCD refresh thread.
 * 
 * This reads the button value and does screen updates.
 */
void threadLcd (uint32_t bufsz) {
    uint8_t button;
    uint8_t msglength;
    uint8_t msgoffset;
    uint8_t count;
    uint8_t ss;
    time_t  timer;
    time_t  delay;
    char    devid[11];
    char    pattern[21];
    uint8_t i, j;
    char    *p;
    struct tm clk;
    //
    delay = 0;
    msglength = 0;
    msgoffset = 0;
    count = 0;
    ss = 0;
    // Upgrade here.
    strcpy_P(devid, PSTR("18B20_00"));
    //
    while (TRUE) {
        button = lcdshieldButtonRead();
        timer = time(NULL);
        //
        if (!(lcd.flags & LCD_FLAGS_MENU) && (button == LCDSHIELD_BTN_SEL)) {
            lcd.flags |= LCD_FLAGS_MENU;
            lcd.timeout = timer + LCD_MENU_TIMEOUT;
        }
        //
        if (atomQueueGet(&lcd.msgqueue, -1, (uint8_t*) lcd.message) == ATOM_OK) {
            lcd.flags |= LCD_FLAGS_MESSAGE;
            lcd.timeout = timer + LCD_MESSAGE_TIMEOUT;
            delay = timer + LCD_MESSAGE_DELAY;
            msglength = strlen(lcd.message);
            msgoffset = 0;
        }
        //
        if (lcd.flags & LCD_FLAGS_MENU) {
            // we are in menu mode.
            if (lcd.timeout < timer) {
                lcd.flags &= (uint8_t) ~LCD_FLAGS_MENU;
                menu.context &= MENU_CONTEXT_TYPEMASK;
                menu.curpos = 0;
                menu.curoffset = 0;
                menu.editoffset = 0;
            } else {
                menuProcess(button, timer);
            }
        } else if (lcd.flags & LCD_FLAGS_MESSAGE) {
            // we are in message mode.
            if (lcd.timeout < timer) {
                lcd.flags &= ~LCD_FLAGS_MESSAGE;
                delay = 0;
                msglength = 0;
                msgoffset = 0;
            }
            // msg processing
            if (delay <= timer) {
                delay = timer + LCD_MESSAGE_DELAY;
                msgoffset += LCD_COLUMNS;
                if (msgoffset >= msglength) msgoffset = 0;
            }
            lcdClear();
            for (i = 0; i < LCD_VIEWPORT_SIZE; ++i) {
                j = msgoffset + i;
                if (j >= msglength) break;
                lcd.viewport[i] = lcd.message[j];
            }
        } else {
            // Standby mode: show display by type.
            switch (lcd.flags & LCD_FLAGS_MODE_MASK) {
                case LCD_FLAGS_MODE_CLK:
                    // update the clock display.
                    lcdClear();
                    ctime_r(&timer, lcd.message);
                    lcd.message[10] = '\n';
                    fputs(lcd.message, &fstrLcd);
                    break;
                case LCD_FLAGS_MODE_STAT:
                    // update the stats display.
                    lcdClear();
                    if (statTable[ss].flags & STATS_FLAG_E) {
                        j = 0;
                        for (i = 0; i < STATS_HOURS; ++i) j += statTable[ss].hourlyHeard[i];
                        p = lcd.message;
                        p += ax25FmtAddr(p, statTable[ss].callsign);
                        switch ((count / 32) % 12) {
                            case 0:
                                p += sprintf_P(p, PSTR(" last\n"));
                                strcpy_P(pattern, PSTR("%m/%d %H:%M:%S"));
                                localtime_r(&statTable[ss].lastHeard, &clk);
                                p += strftime(p, LCD_COLUMNS, pattern, &clk);
                                break;
                            case 1:
                                p += sprintf_P(p, PSTR(" lat\n"));
                                p += aprsPrintLat(p, statPosit[ss].latitudeD, statPosit[ss].latitudeM, APRS_PRINTFMT_DEGMIN);
                                break;
                            case 2:
                                p += sprintf_P(p, PSTR(" long\n"));
                                p += aprsPrintLong(p, statPosit[ss].longitudeD, statPosit[ss].longitudeM, APRS_PRINTFMT_DEGMIN);
                                break;
                            case 3:
                                p += sprintf_P(p, PSTR(" heard\n1h:%d %dh:%d"), statTable[ss].hourlyHeard[0], i, j);
                                break;
                            case 4:
                                p += sprintf_P(p, PSTR(" power\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_PHG) {
                                    p += sprintf_P(p, PSTR("%0.0f W"), powf((float) (statPosit[ss].height - 48), 2.0f));
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 5:
                                p += sprintf_P(p, PSTR(" height\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_PHG) {
                                    p += sprintf_P(p, PSTR("%0.0f Feet"), 10.0f * powf(2.0f, (float) (statPosit[ss].height - 48)));
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 6:
                                p += sprintf_P(p, PSTR(" gain\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_PHG) {
                                    p += sprintf_P(p, PSTR("%d dB"), (statPosit[ss].gain - 48));
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 7:
                                p += sprintf_P(p, PSTR(" dir\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_PHG) {
                                    i = statPosit[ss].direction - 48;
                                    if (i == 0) {
                                        p += sprintf_P(p, PSTR("Omni"));
                                    } else {
                                        p += sprintf_P(p, PSTR("%d%c"), 45 * i, LCD_SYM_DEGREE);
                                    }
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 8:
                                p += sprintf_P(p, PSTR(" course\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_CS) {
                                    p += sprintf_P(p, PSTR("%03d%c"), statPosit[ss].course, LCD_SYM_DEGREE);
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 9:
                                p += sprintf_P(p, PSTR(" speed\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_CS) {
                                    p += sprintf_P(p, PSTR("%03dkt %03.1fkm/h"), statPosit[ss].speed, (1.852f * (float) statPosit[ss].speed));
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 10:
                                p += sprintf_P(p, PSTR(" range\n"));
                                if (statPosit[ss].flags & APRS_POSITFLAG_RNG) {
                                    p += sprintf_P(p, PSTR("%c"), statPosit[ss].range);
                                } else {
                                    p += sprintf_P(p, pstrThreadLcdNA);
                                }
                                break;
                            case 11:
                                count = 0;
                                ss = statsGetNextSlot(ss);
                                p += sprintf_P(p, PSTR("\nNEXT STATION"));
                                break;
                            }
                        fputs(lcd.message, &fstrLcd);
                    } else {
                        ss = statsGetNextSlot(ss);
                        fputs_P(PSTR("NO DATA"), &fstrLcd);
                    }
                    break;
                case LCD_FLAGS_MODE_ENV:
                    // update the environmental display.
                    if (!(count & 0x0F)) {
                        lcdClear();
                        fprintf_P(&fstrLcd, PSTR("18B20: %+1.1f%cC"), ds18b20ConvTemp(devid, DS18B20_UNIT_C), LCD_SYM_DEGREE);
                        lcdSetCurpos(LCD_COLUMNS);
                        fprintf_P(&fstrLcd, PSTR("Therm: %+1.1f%cC"), thermistorGetTempL(THERMISTOR_PIN), LCD_SYM_DEGREE);
                        // Display a piston cursor.
                        lcdSetCurpos(LCD_VIEWPORT_SIZE - 1);
                        j = ((count / 0x10) % 4) + 5;
                        if (j == 8) j = 6;
                        fputc(j, &fstrLcd);
                    }
                    break;
                case LCD_FLAGS_MODE_DBG: 
                    // debug mode redirects stderr to the LCD; do nothing.
                    break;
            }
        }
        // Reposition the cursor during Menu context.
        if (lcd.flags & LCD_FLAGS_MENU) lcdSetCurpos(menu.curoffset + menu.curpos - menu.editoffset);
        // All screen updates occur here.
        if (!(count++ & 0x03)) lcdUpdate();
        // Let the thread sleep.
        atomTimerDelay(LCD_REFRESH_TICKS);
    }
}
