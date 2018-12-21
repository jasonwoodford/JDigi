/*
 * command.h
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
 * $Id: command.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup command <command.h>: Command
 * \code #include <command.h> \endcode
 * <h3>Introduction to the Command Module</h3>
 *  
 * Application code can use these functions as built-in commands.
 */

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "threads.h"

/** \ingroup command */
/* @{ */
#define COMMAND_LINESIZE        128
#define COMMAND_CMDLIST_SIZE    22

uint8_t cmdParseLine (char *buf, uint8_t size, char **argv);

char* cmdAPRSHeardDirect (char *buf, uint8_t argc, char **argv);
char* cmdAPRSHeardAll (char *buf, uint8_t argc, char **argv);
char* cmdAPRSHeardStatsDirect (char *buf, uint8_t argc, char **argv);
char* cmdAPRSHeardStatsAll (char *buf, uint8_t argc, char **argv);
char* cmdAPRSRouteTrace (char *buf, uint8_t slot);
char* cmdBeaconList (char *buf);
char* cmdBeaconClear (char *buf, uint8_t argc, char **argv);
char* cmdBeaconSet (char *buf, uint8_t argc, char **argv);
char* cmdClockSet (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetcall (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetdest (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetvia1 (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetvia2 (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetpos (char *buf, uint8_t argc, char **argv);
char* cmdConfigSeticon (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetcomment (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetposint (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetbeaconint (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetwxint (char *buf, uint8_t argc, char **argv);
char* cmdConfigSetchainflags (char *buf, uint8_t argc, char **argv);
char* cmdConfigWriteEeprom (char *buf, uint8_t argc, char **argv);
char* cmdDS18B20Init (char *buf, uint8_t argc, char **argv);
char* cmdEnviroList (char *buf, uint8_t argc, char **argv);
char* cmdLcdInit (char *buf, uint8_t argc, char **argv);
char* cmdOnewireList (char *buf, uint8_t argc, char **argv);
char* cmdOnewireEnumerate (char *buf, uint8_t argc, char **argv);
char* cmdPrintHelp (char *buf, uint8_t argc, char **argv);

/* @} */
#endif /* __COMMAND_H__ */
