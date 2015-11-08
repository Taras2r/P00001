/*
 * UART.h
 *
 *  Created on: Nov 7, 2015
 *      Author: Taras
 */

#ifndef UART_H_
#define UART_H_

#ifndef _AVR_IO_H_
	#include <avr/io.h>
#endif

#ifndef _STDLIB_H_
	#include <stdlib.h>
#endif

#define BAUD 115200
#include <util/setbaud.h>



static inline void set_uart_baud(void)__attribute__((always_inline));
static inline void init_uart(void)__attribute__((always_inline));
static inline void put_char_to_udr(char data_byte)__attribute__((always_inline));
static inline init_integer_buff(void)__attribute__((always_inline));
void send_message_to_UDR(char * message, int integer);
;
;

#endif /* UART_H_ */
