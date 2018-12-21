/*
 * uart.c
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
 * $Id: uart.c,v 1.0 2018/12/18 21:12:00 NST jason_woodford Exp $
 */

#include "uart.h"

UART uart[UART_PORTS];

/** Initialize the relevant UART structure and configure the port.
 * 
 */
int8_t uartInit (uint8_t port, uint32_t baudrate) {
    // Is the port value valid?
    if (port >= UART_PORTS) return UART_ERROR_PORT;
    // Create a mutex for single-threaded putchar() access.
    if (atomMutexCreate(&(uart[port].mutex)) != ATOM_OK) return UART_ERROR_MUTEX;
    // ATmega datasheets list 2400 baud as the lowest example rate.
    if (baudrate < 2400) return UART_ERROR_BAUD;
    // Setup buffer indexes before enabling the UART.
    uart[port].rxHead = 0;
    uart[port].rxTail = 0;
    uart[port].txHead = 0;
    uart[port].txTail = 0;
    uart[port].flags = 0;
    uartConfigurePort(port, baudrate);
    return 0;
}

/** C-style getchar() used by stdio, which utilizes a FIFO buffer.
 * 
 * Input errors while talking to the UART will cause an immediate
 * return of an error.  Notably, this will be caused by a
 * framing error (serial line "break" condition), by an input
 * overrun, or by a parity error (if parity was enabled and automatic
 * parity recognition is supported by hardware).
 *
 * __This function will block if there is no data.__
 */
int uartGetchar (FILE *stream) {
    int c;
    int8_t port;
    //
    port = uartGetPort(stream);
    if (port < 0) return _FDEV_ERR;
    // Block until there is new data.
    while (!(uart[port].flags & UART_FLAG_RXDATA)) atomTimerDelay(1);
    // Get private access to the UART.
    while (atomMutexGet(&(uart[port].mutex), 10) != ATOM_OK) atomTimerDelay(1);
    // get the char now and update index and flags.
    c = uart[port].rxBuffer[uart[port].rxTail++];
    if (uart[port].rxTail == UART_RX_BUFSIZE) uart[port].rxTail = 0;
    uart[port].flags &= ~UART_FLAG_RXFULL;
    if (uart[port].rxTail == uart[port].rxHead) uart[port].flags &= ~UART_FLAG_RXDATA;
    // check other flags. All must be reset.
    if (uart[port].flags & UART_FLAG_FRAMEERR) {
        uart[port].flags &= ~UART_FLAG_FRAMEERR;
        c = _FDEV_ERR;
    }
    if (uart[port].flags & UART_FLAG_PARITYERR) {
        uart[port].flags &= ~UART_FLAG_PARITYERR;
        c = _FDEV_ERR;
    }
    if (uart[port].flags & UART_FLAG_RXOVERRUN) {
        uart[port].flags &= ~UART_FLAG_RXOVERRUN;
        c = _FDEV_ERR;
    }
    // Return mutex access.
    atomMutexPut(&(uart[port].mutex));
    return c;
}

/** C-style putchar() used by stdio, which utilizes a FIFO buffer.
 *
 * __This function will block if the buffer is full.__
 */
int uartPutchar (uint8_t c, FILE *stream) {
    int8_t port;
    //
    port = uartGetPort(stream);
    if (port < 0) return _FDEV_ERR;
    // Block on private access to the UART
    while (atomMutexGet(&(uart[port].mutex), 10) != ATOM_OK) atomTimerDelay(1);
    // can we put data in the buffer?
    while (uart[port].flags & UART_FLAG_TXFULL) atomTimerDelay(1);
    // put a new char in the buffer.
    uart[port].txBuffer[uart[port].txHead] = c;
    ++uart[port].txHead;
    if (uart[port].txHead == UART_TX_BUFSIZE) uart[port].txHead = 0;
    // if the head equals the tail at this point, then the buffer is full.
    if (uart[port].txHead == uart[port].txTail) uart[port].flags |= UART_FLAG_TXFULL;
    // finally, we have data to transmit.
    uart[port].flags |= UART_FLAG_TXDATA;
    // Return mutex access
    atomMutexPut(&(uart[port].mutex));
    // enable interrupt.
    uartEnableTxInterrupt(port);
    return 0;
}
