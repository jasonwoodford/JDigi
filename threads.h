/*
 * threads.h
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
 * $Id: threads.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup threads <threads.h>: Threads
 * \code #include <threads.h> \endcode
 * <h3>Introduction to the Threads Module</h3>
 *  
 * All application code is used within a running thread.
 * 
 * __Stack Size Definitions__
 *
 * The Idle stack needs to be large enough to handle any interrupt handlers
 * and callbacks called by interrupt handlers (e.g. user-created
 * timer callbacks) as well as the saving of all context when
 * switching away from this thread.
 * 
 * The Main thread stack generally needs to be larger than the idle
 * thread stack, as not only does it need to store interrupt handler
 * stack saves and context switch saves, but the application main thread
 * will generally be carrying out more nested function calls and require
 * stack for application code local variables, etc.
 * 
 * Thread buffer sizes are also defined here.
 */

#ifndef __THREADS_H__
#define __THREADS_H__

#include "atom.h"
#include "atommutex.h"
#include "atomqueue.h"
#include "aprs.h"
#include "ax25.h"
#include "bme280.h"
#include "beacon.h"
#include "chains.h"
#include "clock.h"
#include "command.h"
#include "dht11.h"
#include "ds18b20.h"
#include "eeprom.h"
#include "init.h"
#include "kiss.h"
#include "lcd.h"
#include "lcdshield.h"
#include "menu.h"
#include "nmea.h"
#include "threads.h"
#include "stats.h"
#include "thermistor.h"
#include "time.h"
#include "uart.h"

/** \ingroup threads */
/* @{ */
#define IDLE_STACK_SIZE     256
#define MAIN_STACK_SIZE     256
#define MAIN_STATIC_SIZE    320
#define MAIN_THREAD_PRIO    16
#define RX_STACK_SIZE       256
#define RX_STATIC_SIZE      320
#define RX_THREAD_PRIO      16
#define LCD_STACK_SIZE      256
#define LCD_THREAD_PRIO     16
#define ENVIRO_STACK_SIZE   256
#define ENVIRO_STATIC_SIZE  320
#define ENVIRO_THREAD_PRIO  16
#define SHELL_STACK_SIZE    256
#define SHELL_STATIC_SIZE   320
#define SHELL_THREAD_PRIO   16
#define GPS_STACK_SIZE      256
#define GPS_STATIC_SIZE     320
#define GPS_THREAD_PRIO     16

void threadMain (uint32_t bufsz);    ///< Main thread.
void threadRx (uint32_t bufsz);      ///< Receive thread.
void threadLcd (uint32_t bufsz);     ///< LCD thread.
void threadEnviro (uint32_t bufsz);  ///< Environmental thread.
void threadShell (uint32_t bufsz);   ///< USB Shell thread.
void threadGPS (uint32_t bufsz);     ///< GPS thread.

/* @} */
#endif /* __THREADS_H__ */
