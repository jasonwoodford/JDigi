/*
 * uart.h
 *
 * UART implementation that utilizes FIFO buffers.
 * 
 * Copyright (C)2016 Jason Woodford. All rights reserved.
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
 * $Id: uart.h,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

/** \file */

/** \defgroup uart <uart.h>: UART
 * \code #include <uart.h> \endcode
 * <h3>Introduction to the UART Object</h3>
 *  
 * Each architecture needs to provide it's own dependent functions, but
 * this is the core part of JDigi's UART mechanism. It has built-in FIFO buffers
 * for both receive and transmit streams.
 */

#ifndef __UART_H_
#define __UART_H_

#include "atom.h"
#include "atommutex.h"
#include <stdio.h>

/** \ingroup uart */
/* @{ */
#define UART_PORTS  4   ///< Number of UARTs.

#define UART_RX_BUFSIZE     176 ///< UART FIFO receive buffer size.
#define UART_TX_BUFSIZE     16  ///< UART FIFO transmit buffer size.

#define	UART_FLAG_PARSE	    (uint8_t) (1 << 7)  ///< Receive PARSE flag, for debugging.
#define	UART_FLAG_FRAMEERR	(uint8_t) (1 << 6)  ///< Receive data Frame Error.
#define	UART_FLAG_PARITYERR	(uint8_t) (1 << 5)  ///< Receive data Parity Error.
#define	UART_FLAG_RXOVERRUN (uint8_t) (1 << 4)  ///< Receive buffer overflow.
#define	UART_FLAG_RXFULL    (uint8_t) (1 << 3)  ///< Receive buffer is full.
#define	UART_FLAG_TXFULL    (uint8_t) (1 << 2)  ///< Transmit buffer is full. 
#define	UART_FLAG_RXDATA    (uint8_t) (1 << 1)  ///< There is new data in the receive buffer.
#define UART_FLAG_TXDATA    (uint8_t) (1)       ///< There is new data in the transmit buffer.

#define UART_ERROR_PORT     (-1)    ///< Port number out of range.
#define UART_ERROR_MUTEX    (-2)    ///< Cannot create mutex.
#define UART_ERROR_BAUD     (-3)    ///< Bad baud rate.

/**
 * Architecture independent UART structure.
 */
typedef struct UART {
    volatile uint8_t flags;     ///< Status flags.
    volatile uint8_t rxChar;    ///< Last byte received.
    volatile uint8_t rxHead;    ///< RX buffer head.
    volatile uint8_t rxTail;    ///< RX buffer tail.
    volatile uint8_t txHead;    ///< TX buffer head.
    volatile uint8_t txTail;    ///< TX buffer tail.
    ATOM_MUTEX mutex;    ///< Byte atomic mutex.
    uint8_t rxBuffer[UART_RX_BUFSIZE];    ///< RX FIFO buffer.
    uint8_t txBuffer[UART_TX_BUFSIZE];    ///< TX FIFO buffer.
} UART;

/**
 * Initialize the relevant UART structure and configure it.
 */
int8_t uartInit (uint8_t port, uint32_t baudrate);

/**
 * Place a byte to be transmitted into the TX buffer.
 */
int uartPutchar (uint8_t c, FILE *stream);

/**
 * Read the next available byte from the RX buffer.
 */
int uartGetchar (FILE *stream);

/*** Architecture specific prototypes. ***/

/**
 * An architecture specific function that configures a UART port
 * with the specified bitrate and enables interrupts.
 */
void uartConfigurePort (uint8_t port, uint32_t baudrate);

/**
 * An architecture specific function that enables transmit interrupts.
 */
void uartEnableTxInterrupt (uint8_t port);

/**
 * An architecture specific function that translates a file stream
 * to a UART port number.
 */
int8_t uartGetPort (FILE *stream);

/* @} */
#endif /* __UART_H_ */
