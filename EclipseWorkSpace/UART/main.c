/*
 * main.c
 *
 *  Created on: Nov 5, 2015
 *      Author: Taras
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

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
char* buff;
void send_message_to_UDR(char * message, int integer)
{
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
}

void init_integer_buff(char * buff)
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

int main(void) {
	init_uart();
	init_integer_buff(&buff);
	DDRB |= (1<<PB0);
	while(buff != NULL)
	{
		_delay_ms(1000);
		PORTB |= (1 << PB0);
		send_message_to_UDR("Data ", -45);
		PORTB &= ~(1 << PB0);
		_delay_ms(1000);
		send_message_to_UDR("New Data ", 250);
		;
		;
	}
	return 0;
}
