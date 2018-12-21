/*
 * menu.c
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
 * $Id: menu.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "menu.h"

Menu menu;

extern FILE fstrLcd;
extern FILE fstrUart0;
extern LCD lcd;
extern APRSConfig config;
extern APRSConfig const configJD PROGMEM;

/** Initialize the menu structure.
 */
void menuInit (void) {
    menu.butVal = 0;
    menu.butTimeout = 0;
    menu.context = 0;
    menu.curoffset = 0;
    menu.curpos = 0;
    menu.editoffset = 0;
}

/** Process buttons in menu context.
 * 
 * A button press is a SET/RESET process where the trigger
 * is either a RESET or a button timeout.
 */
void menuProcess (uint8_t button, time_t timer) {
    uint8_t length;
    int16_t val;
    //// Only record a button press if butVal is zero.
    if (menu.butVal == 0) {
    //// If button was pressed record it and set timeout.
        if (button != 0) {
            menu.butVal = button;
            menu.butTimeout = timer + MENU_BUTTON_TIMEOUT;
        }
        return;
    }
    //// Otherwise, butVal is set.
    if ((button == 0) || (menu.butTimeout < timer)) {
    //// The button has been released OR has timed out.
        lcd.timeout = timer + LCD_MENU_TIMEOUT;
        if (menu.context & MENU_CONTEXT_EDIT) {
    //// Inside Edit Context.
            // Edit the value.
            switch (menu.infotype) {
                case MENU_INFOTYPE_BOOL:
                case MENU_INFOTYPE_ENUM:
                case MENU_INFOTYPE_UINT8:
                    menu.info[0] = menuEditInt16(menu.info[0]);
                    break;
                case MENU_INFOTYPE_INT8:
                    menu.info[0] = (int8_t) menuEditInt16((int8_t) menu.info[0]);
                    break;
                case MENU_INFOTYPE_INT16:
                    menuSetInfoInt16(menuEditInt16(menuGetInfoInt16()));
                    break;
                case MENU_INFOTYPE_FLOAT:
                    menu.infofloat = menuEditFloat(menu.infofloat);
                    break;
                case MENU_INFOTYPE_STRING:
                    length = strlen((char*) menu.info);
                    switch (menu.butVal) {
                        case LCDSHIELD_BTN_LEFT:
                            if (menu.curpos > 0) {
                                --menu.curpos;
                            } else {
                                if (length > 0) menu.info[length-1] = 0;
                            }
                            if (menu.curpos < menu.editoffset) {
                                menu.editoffset -= (LCD_COLUMNS / 2);
                            }
                            break;
                        case LCDSHIELD_BTN_RIGHT:
                            if (length == 0) {
                                menu.info[0] = MENU_INFO_CHAR_DEFAULT;
                                menu.info[1] = 0;
                                break;
                            }
                            if (menu.curpos == (menu.infomax - 1)) break;
                            ++menu.curpos;
                            if (menu.curpos >= length) {
                                menu.info[length] = MENU_INFO_CHAR_DEFAULT;
                                menu.info[length+1] = 0;
                            }
                            if (menu.curpos >= (menu.editoffset + LCD_COLUMNS)) {
                                menu.editoffset += (LCD_COLUMNS / 2);
                            }
                            break;
                    }
                    // Follow through as a char.
                case MENU_INFOTYPE_CHAR:
                    switch (menu.butVal) {
                        case LCDSHIELD_BTN_UP:
                            ++menu.info[menu.curpos];
                            if (menu.info[menu.curpos] > MENU_INFO_CHAR_MAX) {
                                menu.info[menu.curpos] = MENU_INFO_CHAR_MIN;
                            }
                            break;
                        case LCDSHIELD_BTN_DOWN:
                            --menu.info[menu.curpos];
                            if (menu.info[menu.curpos] < MENU_INFO_CHAR_MIN) {
                                menu.info[menu.curpos] = MENU_INFO_CHAR_MAX;
                            }
                            break;
                    }
                    break;
            }
            // If the select button was pressed commit the edited value and exit edit context.
            if (menu.butVal == LCDSHIELD_BTN_SEL) {
                switch (menu.context & MENU_CONTEXT_TYPEMASK) {
                    case MENU_CONTEXT_CALL:
                        strncpy(config.srcCall, (char*) menu.info, menu.infomax);
                        break;
                    case MENU_CONTEXT_CALLID:
                        config.srcSSID = menu.info[0];
                        break;
                    case MENU_CONTEXT_DEST:
                        strncpy(config.destCall, (char*) menu.info, menu.infomax);
                        break;
                    case MENU_CONTEXT_DESTID:
                        config.destSSID = menu.info[0];
                        break;
                    case MENU_CONTEXT_VIA1:
                        strncpy(config.viaCall1, (char*) menu.info, menu.infomax);
                        break;
                    case MENU_CONTEXT_VIA1ID:
                        config.viaSSID1 = menu.info[0];
                        break;
                    case MENU_CONTEXT_VIA2:
                        strncpy(config.viaCall2, (char*) menu.info, menu.infomax);
                        break;
                    case MENU_CONTEXT_VIA2ID:
                        config.viaSSID2 = menu.info[0];
                        break;
                    case MENU_CONTEXT_INTPOS:
                        config.txIntPos = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_INTWX:
                        config.txIntWX = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_INTBEACON:
                        config.txIntBeacon = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_OFFPOS:
                        config.txOffPos = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_OFFWX:
                        config.txOffWX = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_OFFBEACON:
                        config.txOffBeacon = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_POSLATD:
                        config.posLatD = (int8_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_POSLATM:
                        config.posLatM = menu.infofloat;
                        break;
                    case MENU_CONTEXT_POSLONGD:
                        config.posLongD = (int8_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_POSLONGM:
                        config.posLongM = menu.infofloat;
                        break;
                    case MENU_CONTEXT_APRSICON1:
                        config.icon1 = menu.info[0];
                        break;
                    case MENU_CONTEXT_APRSICON2:
                        config.icon2 = menu.info[0];
                        break;
                    case MENU_CONTEXT_INFOCOM:
                        strncpy(config.infoComment, (char*) menu.info, menu.infomax);
                        break;
                    case MENU_CONTEXT_UART0BAUD:
                        config.uart0Baud = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_UART1BAUD:
                        config.uart1Baud = (uint16_t) menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_LCDMODE:
                        config.lcdMode = (config.lcdMode & ~LCD_FLAGS_MODE_MASK) | menu.info[0];
                        lcd.flags = (lcd.flags & ~LCD_FLAGS_MODE_MASK) | menu.info[0];
                        if (menu.info[0] == LCD_FLAGS_MODE_DBG) {
                            stderr = &fstrLcd;
                        } else {
                            stderr = &fstrUart0;
                        }
                        break;
                    case MENU_CONTEXT_LCDLIGHT:
                        if (menu.info[0] == TRUE) {
                            config.lcdMode |= LCD_FLAGS_BACKLIGHT;
                        } else {
                            config.lcdMode &= ~LCD_FLAGS_BACKLIGHT;
                        }
                        lcdshieldBacklightSet(menu.info[0]);
                        break;
                    case MENU_CONTEXT_FRAMEINFO:
                        config.debugFlags &= ~CHAIN_PRINTFRAME_MASK;
                        config.debugFlags |= menu.info[0];
                        break;
                    case MENU_CONTEXT_FRAMEEN:
                        if (menu.info[0] == TRUE) {
                            config.debugFlags |= CHAIN_PRINTFRAME_E;
                        } else {
                            config.debugFlags &= ~CHAIN_PRINTFRAME_E;
                        }
                        break;
                    case MENU_CONTEXT_REP_LEGACY:
                        if (menu.info[0] == TRUE) {
                            config.repeatFlags |= CHAIN_REPEATFLAG_LEGACY;
                        } else {
                            config.repeatFlags &= ~CHAIN_REPEATFLAG_LEGACY;
                        }
                        break;
                    case MENU_CONTEXT_REP_WIDE1:
                        if (menu.info[0] == TRUE) {
                            config.repeatFlags |= CHAIN_REPEATFLAG_WIDE1;
                        } else {
                            config.repeatFlags &= ~CHAIN_REPEATFLAG_WIDE1;
                        }
                        break;
                    case MENU_CONTEXT_REP_WIDE2:
                        if (menu.info[0] == TRUE) {
                            config.repeatFlags |= CHAIN_REPEATFLAG_WIDE2;
                        } else {
                            config.repeatFlags &= ~CHAIN_REPEATFLAG_WIDE2;
                        }
                        break;
                    case MENU_CONTEXT_UTC_OFFSET:
                        config.utcOffset = 60 * menuGetInfoInt16();
                        break;
                    case MENU_CONTEXT_WREEPROM:
                        if (menu.info[0] == TRUE) {
                            lcdSetLine(2);
                            fputs_P(PSTR("Writing..."), &fstrLcd);
                            lcdUpdate();
                            eepromWriteConfig();
                        }
                        break;
                    case MENU_CONTEXT_LOADDEF:
                        if (menu.info[0] == TRUE) {
                            lcdSetLine(2);
                            fputs_P(PSTR("Loading..."), &fstrLcd);
                            lcdUpdate();
                            memcpy_P(&config, &configJD, APRS_CONFIG_TOTAL_SIZE);
                        }
                        break;
                }
                menu.context &= ~MENU_CONTEXT_EDIT;
                menu.curoffset = 0;
                menu.curpos = 0;
                menu.editoffset = 0;
            }
        } else {
    //// Outside edit context.
            menu.curoffset = 0;
            menu.curpos = 0;
            menu.editoffset = 0;
            // Process buttons.
            if (menu.butVal == LCDSHIELD_BTN_UP) {
                // Increment context.
                ++menu.context;
                if (menu.context == MENU_CONTEXT_SIZE) menu.context = 0;
            }
            if (menu.butVal == LCDSHIELD_BTN_DOWN) {
                // Decrement context.
                if (menu.context == 0) menu.context = MENU_CONTEXT_SIZE;
                --menu.context;
            }
            if ((menu.butVal == LCDSHIELD_BTN_LEFT) && (menu.infotype != MENU_INFOTYPE_NULL)) {
                // Enter the edit context.
                menu.context |= MENU_CONTEXT_EDIT;
                menu.curoffset = LCD_COLUMNS;
                switch (menu.infotype) {
                    case MENU_INFOTYPE_CHAR:
                    case MENU_INFOTYPE_STRING:
                        menu.curpos = 0;
                        break;
                    case MENU_INFOTYPE_UINT8:
                        menu.curpos = 5;
                        break;
                    case MENU_INFOTYPE_INT8:
                        menu.curpos = 6;
                        break;
                    case MENU_INFOTYPE_INT16:
                        menu.curpos = 7;
                        break;
                }
            }
            if (menu.butVal == LCDSHIELD_BTN_RIGHT) {
                // Exit the menu.
                lcd.flags &= (uint8_t) ~LCD_FLAGS_MENU;
            }
            // Update the menu title.
            lcdSetLine(1);
            menu.infomin = 0;   // default
            switch (menu.context & MENU_CONTEXT_TYPEMASK) {
                case MENU_CONTEXT_CALL:
                    fputs_P(PSTR("Callsign:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_STRING;
                    menu.infomax = 6;
                    strncpy((char*) menu.info, config.srcCall, menu.infomax);
                    menu.info[menu.infomax] = 0;
                    break;
                case MENU_CONTEXT_CALLID:
                    fputs_P(PSTR("Callsign SSID:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_UINT8;
                    menu.infomax = 15;
                    menu.info[0] = config.srcSSID;
                    break;
                case MENU_CONTEXT_DEST:
                    fputs_P(PSTR("APRS Dest:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_STRING;
                    menu.infomax = 6;
                    strncpy((char*) menu.info, config.destCall, menu.infomax);
                    menu.info[menu.infomax] = 0;
                    break;
                case MENU_CONTEXT_DESTID:
                    fputs_P(PSTR("APRS Dest SSID:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_UINT8;
                    menu.infomax = 15;
                    menu.info[0] = config.destSSID;
                    break;
                case MENU_CONTEXT_VIA1:
                    fputs_P(PSTR("Via1 Call:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_STRING;
                    menu.infomax = 6;
                    strncpy((char*) menu.info, config.viaCall1, menu.infomax);
                    menu.info[menu.infomax] = 0;
                    break;
                case MENU_CONTEXT_VIA1ID:
                    fputs_P(PSTR("Via1 SSID:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_UINT8;
                    menu.infomax = 15;
                    menu.info[0] = config.viaSSID1;
                    break;
                case MENU_CONTEXT_VIA2:
                    fputs_P(PSTR("Via2 Call:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_STRING;
                    menu.infomax = 6;
                    strncpy((char*) menu.info, config.viaCall2, menu.infomax);
                    menu.info[menu.infomax] = 0;
                    break;
                case MENU_CONTEXT_VIA2ID:
                    fputs_P(PSTR("Via2 SSID:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_UINT8;
                    menu.infomax = 15;
                    menu.info[0] = config.viaSSID2;
                    break;
                case MENU_CONTEXT_INTPOS:
                    fputs_P(PSTR("Posit interval:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 60;
                    menu.infomax = 8 * ONE_HOUR;
                    menuSetInfoInt16(config.txIntPos);
                    break;
                case MENU_CONTEXT_INTWX:
                    fputs_P(PSTR("WX interval:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 60;
                    menu.infomax = 8 * ONE_HOUR;
                    menuSetInfoInt16(config.txIntWX);
                    break;
                case MENU_CONTEXT_INTBEACON:
                    fputs_P(PSTR("Beacon interval:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 60;
                    menu.infomax = 8 * ONE_HOUR;
                    menuSetInfoInt16(config.txIntBeacon);
                    break;
                case MENU_CONTEXT_OFFPOS:
                    fputs_P(PSTR("Posit offset:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 0;
                    menu.infomax = ONE_HOUR;
                    menuSetInfoInt16(config.txOffPos);
                    break;
                case MENU_CONTEXT_OFFWX:
                    fputs_P(PSTR("WX offset:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 0;
                    menu.infomax = ONE_HOUR;
                    menuSetInfoInt16(config.txOffWX);
                    break;
                case MENU_CONTEXT_OFFBEACON:
                    fputs_P(PSTR("Beacon offset:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 0;
                    menu.infomax = ONE_HOUR;
                    menuSetInfoInt16(config.txOffBeacon);
                    break;
                case MENU_CONTEXT_POSLATD:
                    fputs_P(PSTR("Position LatD:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT8;
                    menu.infomin = -90;
                    menu.infomax = 90;
                    menuSetInfoInt16(config.posLatD);
                    break;
                case MENU_CONTEXT_POSLATM:
                    fputs_P(PSTR("Position LatM:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_FLOAT;
                    menu.infomax = 60;
                    menu.infofloat = config.posLatM;
                    break;
                case MENU_CONTEXT_POSLONGD:
                    fputs_P(PSTR("Position LongD:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = -180;
                    menu.infomax = 180;
                    menuSetInfoInt16(config.posLongD);
                    break;
                case MENU_CONTEXT_POSLONGM:
                    fputs_P(PSTR("Position LongM:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_FLOAT;
                    menu.infomax = 60;
                    menu.infofloat = config.posLongM;
                    break;
                case MENU_CONTEXT_APRSICON1:
                    fputs_P(PSTR("APRS Icon #1:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_CHAR;
                    menu.info[0] = config.icon1;
                    break;
                case MENU_CONTEXT_APRSICON2:
                    fputs_P(PSTR("APRS Icon #2:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_CHAR;
                    menu.info[0] = config.icon2;
                    break;
                case MENU_CONTEXT_INFOCOM:
                    fputs_P(PSTR("APRS Comment:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_STRING;
                    menu.infomax = MENU_INFO_SIZE - 1;
                    strncpy((char*) menu.info, config.infoComment, menu.infomax);
                    menu.info[menu.infomax] = 0;
                    break;
                case MENU_CONTEXT_UART0BAUD:
                    fputs_P(PSTR("USB Baudrate:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 2400;
                    menu.infomax = 19200;
                    menuSetInfoInt16(config.uart0Baud);
                    break;
                case MENU_CONTEXT_UART1BAUD:
                    fputs_P(PSTR("KISS Baudrate:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = 2400;
                    menu.infomax = 19200;
                    menuSetInfoInt16(config.uart1Baud);
                    break;
                case MENU_CONTEXT_LCDMODE:
                    fputs_P(PSTR("LCD Mode:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_ENUM;
                    menu.infomax = LCD_FLAGS_MODE_SIZE - 1;
                    menuSetInfoPStr(LCD_FLAGS_MODE_CLK, PSTR("Clock"));
                    menuSetInfoPStr(LCD_FLAGS_MODE_STAT, PSTR("Stats"));
                    menuSetInfoPStr(LCD_FLAGS_MODE_ENV, PSTR("Environmental"));
                    menuSetInfoPStr(LCD_FLAGS_MODE_DBG, PSTR("Debug"));
                    menu.info[0] = config.lcdMode & LCD_FLAGS_MODE_MASK;
                    break;
                case MENU_CONTEXT_LCDLIGHT:
                    fputs_P(PSTR("LCD Backlight:"), &fstrLcd);
                    if (config.lcdMode & LCD_FLAGS_BACKLIGHT) { menu.info[0] = TRUE; } else { menu.info[0] = FALSE; }
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_FRAMEINFO:
                    fputs_P(PSTR("Frame Verbosity:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_ENUM;
                    menu.infomax = CHAIN_PRINTFRAME_MASK;
                    menuSetInfoPStr(0, PSTR("Simple"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_VIA, PSTR("Via"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_INFO, PSTR("Info"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_VIA | CHAIN_PRINTFRAME_INFO, PSTR("Via+Info"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_CRC, PSTR("CRC"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_CRC | CHAIN_PRINTFRAME_VIA, PSTR("Via+CRC"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_CRC | CHAIN_PRINTFRAME_INFO, PSTR("Info+CRC"));
                    menuSetInfoPStr(CHAIN_PRINTFRAME_CRC | CHAIN_PRINTFRAME_VIA | CHAIN_PRINTFRAME_INFO, PSTR("Via+Info+CRC"));
                    menu.info[0] = config.debugFlags & CHAIN_PRINTFRAME_MASK;
                    break;
                case MENU_CONTEXT_FRAMEEN:
                    fputs_P(PSTR("Frame Enable:"), &fstrLcd);
                    if (config.chainFlags & CHAIN_PRINTFRAME_E) { menu.info[0] = TRUE; } else { menu.info[0] = FALSE; }
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_REP_LEGACY:
                    fputs_P(PSTR("Repeat legacy:"), &fstrLcd);
                    if (config.repeatFlags & CHAIN_REPEATFLAG_LEGACY) { menu.info[0] = TRUE; } else { menu.info[0] = FALSE; }
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_REP_WIDE1:
                    fputs_P(PSTR("Repeat WIDE1:"), &fstrLcd);
                    if (config.repeatFlags & CHAIN_REPEATFLAG_WIDE1) { menu.info[0] = TRUE; } else { menu.info[0] = FALSE; }
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_REP_WIDE2:
                    fputs_P(PSTR("Repeat WIDE2:"), &fstrLcd);
                    if (config.repeatFlags & CHAIN_REPEATFLAG_WIDE2) { menu.info[0] = TRUE; } else { menu.info[0] = FALSE; }
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_UTC_OFFSET:
                    fputs_P(PSTR("UTC offset min:"), &fstrLcd);
                    menu.infotype = MENU_INFOTYPE_INT16;
                    menu.infomin = -12 * 60;
                    menu.infomax = +12 * 60;
                    menuSetInfoInt16(config.utcOffset / 60);
                    break;
                case MENU_CONTEXT_WREEPROM:
                    fputs_P(PSTR("Write to EEPROM:"), &fstrLcd);
                    menu.info[0] = FALSE;
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_LOADDEF:
                    fputs_P(PSTR("Reload defaults:"), &fstrLcd);
                    menu.info[0] = FALSE;
                    menuSetInfoBool();
                    break;
                case MENU_CONTEXT_CRC:
                    fprintf_P(&fstrLcd, PSTR("Current CRC:\n0:%04X  1:%04X"), config.crc0, config.crc1);
                    menu.infotype = MENU_INFOTYPE_NULL;
                    break;
            }
        }
        // Refresh edit line contents.
        if (menu.infotype != MENU_INFOTYPE_NULL) lcdSetLine(2);
        switch (menu.infotype) {
            case MENU_INFOTYPE_UINT8:
                val = menu.info[0];
                fprintf_P(&fstrLcd, PSTR("[% 3d] "), val);
                menuPrintMinMax(val);
                break;
            case MENU_INFOTYPE_INT8:
                val = (int8_t) menu.info[0];
                fprintf_P(&fstrLcd, PSTR("[%+ 4d] "), val);
                menuPrintMinMax(val);
                break;
            case MENU_INFOTYPE_INT16:
                val = menuGetInfoInt16();
                fprintf_P(&fstrLcd, PSTR("[%+ 6d] "), val);
                menuPrintMinMax(val);
                break;
            case MENU_INFOTYPE_CHAR:
                fputc((int) menu.info[0], &fstrLcd);
                break;
            case MENU_INFOTYPE_STRING:
                fputs((char*) (menu.info + menu.editoffset), &fstrLcd);
                break;
            case MENU_INFOTYPE_BOOL:
            case MENU_INFOTYPE_ENUM:
                fputs_P(menuGetInfoPStr(menu.info[0]), &fstrLcd);
                break;
            case MENU_INFOTYPE_FLOAT:
                fprintf_P(&fstrLcd, PSTR("[%+ 5.2f] "), (double) menu.infofloat);
                menuPrintMinMax((uint16_t) menu.infofloat);
                break;
        }
    }
    //// If the button was released, reset butVal.
    if (button == 0) menu.butVal = 0;
}

/** Append a MIN and/or MAX tag to the menu prompt.
 */
void menuPrintMinMax (int16_t val) {
    if (val == menu.infomin) fputs_P(PSTR("MIN"), &fstrLcd);
    if (val == menu.infomax) fputs_P(PSTR("MAX"), &fstrLcd);
}

/** Set the first two bytes of info from an int16.
 */
void menuSetInfoInt16 (int16_t val) {
    menu.info[1] = (uint8_t) (val >> 8) & 0xFF;
    menu.info[0] = (uint8_t) (val & 0xFF);
}

/** Return the first two bytes of info as an int16.
 */
int16_t menuGetInfoInt16 (void) {
    return (((menu.info[1] << 8) & 0xFF00) | menu.info[0]);
}

/** 
 */
void menuSetInfoPStr (uint8_t index, PGM_P str) {
    index = (index + 1) << 1;
    menu.info[index + 1] = (uint8_t) ((uint16_t) str >> 8) & 0xFF;
    menu.info[index] = (uint8_t) ((uint16_t) str & 0xFF);
}

/** 
 */
char* menuGetInfoPStr (uint8_t index) {
    index = (index + 1) << 1;
    return ((char*) (((menu.info[index + 1] << 8) & 0xFF00) | menu.info[index]));
}

/** 
 */
void menuSetInfoBool (void) {
    menu.infotype = MENU_INFOTYPE_BOOL;
    menu.infomin = FALSE;
    menu.infomax = TRUE;
    menuSetInfoPStr(TRUE, PSTR("YES"));
    menuSetInfoPStr(FALSE, PSTR("NO"));
}

/** Adjust an integer value up/down based on butVal.
 */
int16_t menuEditInt16 (int16_t val) {
    switch (menu.butVal) {
        case LCDSHIELD_BTN_UP:
            if (val < menu.infomax) ++val;
            break;
        case LCDSHIELD_BTN_DOWN:
            if (val > menu.infomin) --val;
            break;
        case LCDSHIELD_BTN_RIGHT:
            if ((val + MENU_BUTTON_BIGINC) < menu.infomax) val += MENU_BUTTON_BIGINC;
            break;
        case LCDSHIELD_BTN_LEFT:
            if ((val - MENU_BUTTON_BIGINC) > menu.infomin) val -= MENU_BUTTON_BIGINC;
            break;
    }
    return val;
}

/** Adjust an integer value up/down based on butVal.
 */
float menuEditFloat (float val) {
    switch (menu.butVal) {
        case LCDSHIELD_BTN_UP:
            if ((val + 1.0f) <= menu.infomax) val += 1.0f;
            break;
        case LCDSHIELD_BTN_DOWN:
            if ((val - 1.0f) >= menu.infomin) val -= 1.0f;
            break;
        case LCDSHIELD_BTN_RIGHT:
            if ((val + MENU_BUTTON_FRACINC) <= menu.infomax) val += MENU_BUTTON_FRACINC;
            break;
        case LCDSHIELD_BTN_LEFT:
            if ((val - MENU_BUTTON_FRACINC) >= menu.infomin) val -= MENU_BUTTON_FRACINC;
            break;
    }
    return val;
}
