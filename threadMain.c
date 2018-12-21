/*
 * threadMain.c
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
 * $Id: threadMain.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */
#include "threads.h"

/** All thread contexts and stacks are defined here.
 */
uint8_t  stackIdle[IDLE_STACK_SIZE];    ///< Idle thread stack.
ATOM_TCB tcbMain;                       ///< Main thread TCB.
uint8_t  stackMain[MAIN_STACK_SIZE];    ///< Main thread stack.
ATOM_TCB tcbRx;                         ///< Receive thread TCB.
uint8_t  stackRx[RX_STACK_SIZE];        ///< Receive thread stack.
ATOM_TCB tcbShell;                      ///< USB Shell thread TCB.
uint8_t  stackShell[SHELL_STACK_SIZE];  ///< USB Shell thread stack.
ATOM_TCB tcbLcd;                        ///< LCD thread TCB.
uint8_t  stackLcd[LCD_STACK_SIZE];      ///< LCD thread stack.
ATOM_TCB tcbEnviro;                     ///< Environmental thread TCB.
uint8_t  stackEnviro[ENVIRO_STACK_SIZE];///< Environmental thread stack.
ATOM_TCB tcbGPS;                        ///< GPS thread TCB.
uint8_t  stackGPS[GPS_STACK_SIZE];      ///< GPS thread stack.

static uint8_t bufMain[MAIN_STATIC_SIZE];   ///< Main thread static buffer.

extern APRSConfig   config;
extern AX25Frame    frame[];
extern AX25FrameInfo frameinfo;

/** Main Thread.
 * 1) Initializes the system;
 * 2) Starts all other threads;
 * 3) Processes and transmits queued AX.25 frames (an infinite loop).
 */
void threadMain (uint32_t bufsz) {
    int8_t slot;
    // Initialize the system.
    sysInit((char*) bufMain);
    // Say hello.
    fprintf_P(stderr, PSTR("\n***\nThis is %s-%d"), config.srcCall, config.srcSSID);
    // Start the receive thread.
    if (atomThreadCreate(&tcbRx, RX_THREAD_PRIO, threadRx,
                         RX_STATIC_SIZE, stackRx, RX_STACK_SIZE, FALSE) != ATOM_OK)
        fputs_P(PSTR("\nCannot start Receive thread!"), stderr);
    // Start the LCD refresh thread.
    if (atomThreadCreate(&tcbLcd, LCD_THREAD_PRIO, threadLcd,
                         0, stackLcd, LCD_STACK_SIZE, FALSE) != ATOM_OK)
        fputs_P(PSTR("\nCannot start LCD thread!"), stderr);
    // Start the Environmental update thread.
    if (atomThreadCreate(&tcbEnviro, ENVIRO_THREAD_PRIO, threadEnviro,
                         ENVIRO_STATIC_SIZE, stackEnviro, ENVIRO_STACK_SIZE, FALSE) != ATOM_OK)
        fputs_P(PSTR("\nCannot start Enviro thread!"), stderr);
    // Start the USB Shell thread.
    if (atomThreadCreate(&tcbShell, SHELL_THREAD_PRIO, threadShell,
                         SHELL_STATIC_SIZE, stackShell, SHELL_STACK_SIZE, FALSE) != ATOM_OK)
        fputs_P(PSTR("\nCannot start USB Shell thread!"), stderr);
    // Start the GPS thread.
    if (atomThreadCreate(&tcbGPS, GPS_THREAD_PRIO, threadGPS,
                         GPS_STATIC_SIZE, stackGPS, GPS_STACK_SIZE, FALSE) != ATOM_OK)
        fputs_P(PSTR("\nCannot start GPS thread!"), stderr);
    // Start processing.
    while (TRUE) {
        // Refresh stats.
        statsRefresh();
        // Check for beacons ready to transmit.
        beaconProcess(bufMain);
        // Process each slot individually.
        if (frameinfo.ticks == 0) {
            frameinfo.ticks = AX25_FRAME_TICKS;
            for (slot = 0; slot < AX25_FRAME_SLOTS; ++slot) {
                if (frame[slot].lifetime > 0) --frame[slot].lifetime;
                chainInput(slot);
                statsUpdate(slot, (char*) bufMain);
                aprsParseInfo(slot, (char*) bufMain);
                chainForward(slot);
                chainOutput(slot, bufMain);
                chainDequeue(slot);
            }
        }
        // Let the thread nap.
        atomTimerDelay(AX25_FRAME_TICKS);
    }
}
