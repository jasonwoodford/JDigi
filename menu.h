/*
 * menu.h
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
 * $Id: menu.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup menu <menu.h>: Menu
 * \code #include <menu.h> \endcode
 * <h3>Introduction to the Menu Module</h3>
 *  
 * This defines Menu functions for applications.
 */

#ifndef __MENU_H__
#define __MENU_H__

#include "atom.h"
#include "aprs.h"
#include "eeprom.h"
#include "lcdshield.h"

/** \ingroup menu */
/* @{ */
#define MENU_BUTTON_TIMEOUT     1       ///< in seconds.
#define MENU_BUTTON_BIGINC      20      ///< large integer adjust
#define MENU_BUTTON_FRACINC     0.01f   ///< fractional float adjust

#define MENU_CONTEXT_CALL       0
#define MENU_CONTEXT_CALLID     1
#define MENU_CONTEXT_DEST       2
#define MENU_CONTEXT_DESTID     3
#define MENU_CONTEXT_VIA1       4
#define MENU_CONTEXT_VIA1ID     5
#define MENU_CONTEXT_VIA2       6
#define MENU_CONTEXT_VIA2ID     7
#define MENU_CONTEXT_INTPOS     8
#define MENU_CONTEXT_INTWX      9
#define MENU_CONTEXT_INTBEACON  10
#define MENU_CONTEXT_OFFPOS     11
#define MENU_CONTEXT_OFFWX      12
#define MENU_CONTEXT_OFFBEACON  13
#define MENU_CONTEXT_POSLATD    14
#define MENU_CONTEXT_POSLATM    15
#define MENU_CONTEXT_POSLONGD   16
#define MENU_CONTEXT_POSLONGM   17
#define MENU_CONTEXT_APRSICON1  18
#define MENU_CONTEXT_APRSICON2  19
#define MENU_CONTEXT_INFOCOM    20
#define MENU_CONTEXT_UART0BAUD  21
#define MENU_CONTEXT_UART1BAUD  22
#define MENU_CONTEXT_LCDMODE    23
#define MENU_CONTEXT_LCDLIGHT   24
#define MENU_CONTEXT_FRAMEINFO  25
#define MENU_CONTEXT_FRAMEEN    26
#define MENU_CONTEXT_REP_LEGACY 27
#define MENU_CONTEXT_REP_WIDE1  28
#define MENU_CONTEXT_REP_WIDE2  29
#define MENU_CONTEXT_UTC_OFFSET 30
#define MENU_CONTEXT_WREEPROM   31
#define MENU_CONTEXT_LOADDEF    32
#define MENU_CONTEXT_CRC        33
#define MENU_CONTEXT_SIZE       34              ///< "Last"
#define MENU_CONTEXT_TYPEMASK   (uint8_t) 0x3F  ///< 0 to 63.
#define MENU_CONTEXT_EDIT       (uint8_t) (1 << 7)

#define MENU_INFO_SIZE          32
#define MENU_INFO_CHAR_MIN      ' '
#define MENU_INFO_CHAR_MAX      '~'
#define MENU_INFO_CHAR_DEFAULT  'A'

#define MENU_INFOTYPE_NULL      0
#define MENU_INFOTYPE_BOOL      1
#define MENU_INFOTYPE_ENUM      2
#define MENU_INFOTYPE_UINT8     3
#define MENU_INFOTYPE_INT8      4
#define MENU_INFOTYPE_INT16     5
#define MENU_INFOTYPE_CHAR      6
#define MENU_INFOTYPE_STRING    7
#define MENU_INFOTYPE_FLOAT     8
#define MENU_INFOTYPE_DOUBLE    9

typedef struct Menu {
    time_t  timeout;
    time_t  butTimeout;
    uint8_t butVal;
    uint8_t context;
    uint8_t curoffset;
    uint8_t curpos;
    uint8_t editoffset;
    uint8_t infotype;
    int16_t infomin;
    int16_t infomax;
    uint8_t info[MENU_INFO_SIZE];
    float   infofloat;
} Menu;

void    menuInit (void);
void    menuProcess (uint8_t button, time_t timer);
void    menuPrintMinMax (int16_t val);
void    menuSetInfoInt16 (int16_t val);
int16_t menuGetInfoInt16 (void);
void    menuSetInfoPStr (uint8_t index, PGM_P str);
char*   menuGetInfoPStr (uint8_t index);
void    menuSetInfoBool (void);
int16_t menuEditInt16 (int16_t val);
float   menuEditFloat (float val);

/* @} */
#endif /* __MENU_H__ */
