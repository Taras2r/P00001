/*
 * UART.h
 *
 *  Created on: 6 лист. 2015
 *      Author: tko
 */

#ifndef UART_H_
#define UART_H_

#ifndef _STDIO_H_
	#include<avr/io.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif


#define BAUD 115200
#include <util/setbaud.h>
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

void send_message_to_UDR(char * message, int integer)
{
   char* buff = (char*) malloc((sizeof(int)*8+1));
   itoa(integer, buff,10);
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
   //Add commands to run cursor to new line or create new function for this
}

#endif /* UART_H_ */
