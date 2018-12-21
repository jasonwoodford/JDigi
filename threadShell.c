/*
 * threadShell.c
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
 * $Id: threadShell.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"
#include "command-extern.h" // Program memory string pointers.

static uint8_t  bufShell[SHELL_STATIC_SIZE];    ///< USB Shell static buffer.

/** USB Shell thread.
 */
void threadShell (uint32_t bufsz) {
    uint8_t argc, flag, size;
    char    **argv, *p, *q;
    int     c;
    //
    while (TRUE) {
        p = (char*) bufShell;
        flag = FALSE;
        size = 0;
        fputs_P(pstrCmdPROMPT, stdout);
        // Capture command line input.
        while (!flag) {
            c = fgetc(stdin);
            switch (c) {
                case _FDEV_EOF:
                    // Reset size quietly.
                    size = 0;
                    break;
                case _FDEV_ERR:
                    // Reset size with error.
                    size = 0;
                    fputs_P(PSTR("\nShell: Receive Error."), stdout);
                    break;
                case '\n':
                    // Ignore newlines.
                    break;
                case '\b':
                    // Process backspace.
                    if (size > 0) {
                        --size;
                        fputs_P(PSTR("\b \b"), stdout);
                    }
                    break;
                case '\r':
                    // End of command.
                    flag = TRUE;
                    fputc('\n', stdout);
                    break;
                default:
                    *(p + size++) = (char) c;
                    fputc(c, stdout);
            }
            if (size == COMMAND_LINESIZE) {
                size = 0;
                fputs_P(PSTR("\nShell: Buffer Overflow."), stdout);
            }
        }
        // Allocate argv and parse command line.
        argv = (char**) p + size;
        argc = cmdParseLine(p, size, argv);
        // If there are no args skip the command interpreter.
        if (argc == 0) continue;
        // Allocate return string.
        p += size + (argc * sizeof(argv));
        // Command interpreter.
        strupr(argv[0]);
        if (strcmp_P(argv[0], pstrCmdBeaconList) == 0) {
            cmdBeaconList(p);
        } else if (strcmp_P(argv[0], pstrCmdBeaconClear) == 0) {
            cmdBeaconClear(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdBeaconSet) == 0) {
            cmdBeaconSet(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetClock) == 0) {
            cmdClockSet(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetCall) == 0) {
            cmdConfigSetcall(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetDest) == 0) {
            cmdConfigSetdest(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetVia1) == 0) {
            cmdConfigSetvia1(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetVia2) == 0) {
            cmdConfigSetvia2(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetPos) == 0) {
            cmdConfigSetpos(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetIcon) == 0) {
            cmdConfigSeticon(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetComment) == 0) {
            cmdConfigSetcomment(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetPosint) == 0) {
            cmdConfigSetposint(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetWXint) == 0) {
            cmdConfigSetwxint(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetBeaconint) == 0) {
            cmdConfigSetbeaconint(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdSetChainflags) == 0) {
            cmdConfigSetchainflags(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdWriteEeprom) == 0) {
            cmdConfigWriteEeprom(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdLcdInit) == 0) {
            cmdLcdInit(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdEnviroList) == 0) {
            cmdEnviroList(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdOnewireList) == 0) {
            cmdOnewireList(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdOnewireEnum) == 0) {
            cmdOnewireEnumerate(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdDS18B20Init) == 0) {
            cmdDS18B20Init(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdHelp) == 0) {
            cmdPrintHelp(p, argc, argv);
        } else if (strcmp_P(argv[0], pstrCmdAPRSStats) == 0) {
            q = p;
            cmdAPRSHeardDirect(q, argc, argv);
            q += strlen(q);
            *q++ = '\n';
            cmdAPRSHeardAll(q, argc, argv);
        } else {
            strcpy_P(p, PSTR("?Unknown command"));
        }
        // Send output.
        fputs(p, stdout);
        fprintf_P(stdout, PSTR("\nStatic usage: %d bytes"), size + (argc * sizeof(argv)) + strlen(p));
    }
}
