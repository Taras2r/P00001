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


char* buff;
static inline void set_uart_baud(void)__attribute__((always_inline));
static inline void init_uart(void)__attribute__((always_inline));
static inline void put_char_to_udr(char data_byte)__attribute__((always_inline));
static inline void init_integer_buff(void)__attribute__((always_inline));
void send_message_to_UDR(char * message, int integer, char integer_format);

static void set_uart_baud(void)
{
   UBRRH = UBRRH_VALUE;
   UBRRL = UBRRL_VALUE;
   #if USE_2X
   UCSRA |= (1 << U2X);
   #else
   UCSRA &= ~(1 << U2X);
   #endif
}

static void init_uart(void)
{
   /* Enable receiver and transmitter */
   UCSRB = (1<<RXEN)|(1<<TXEN);
   /* Set frame format: 8data, 1stop bit */
   UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
   set_uart_baud();
}

static void put_char_to_udr(char data_byte)
{
   while(!(UCSRA & (1 << UDRE)));
   {
      //??UCSRA |= (1 << UDRE);
      UDR = data_byte;
   }

}

void init_integer_buff(void)
{
		buff = (char*) malloc(sizeof(int)*8+1);
		if(buff == NULL)
		{
			char* error_message = "Malloc err.\n\r";
			do
			{
				put_char_to_udr(*error_message);
			}while(*++error_message);
		}
}

void send_message_to_UDR(char * message, int integer, char integer_format)
{
   itoa(integer, buff, integer_format);
   do
   {
     put_char_to_udr(*message);
   }while(*++message);
   do
   {
     put_char_to_udr(*buff);
   }while(*++buff);
   put_char_to_udr('\n');
   put_char_to_udr('\r');
}

#endif /* UART_H_ */
