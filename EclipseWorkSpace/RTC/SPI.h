/*
 * SPI.h
 *
 *  Created on: 2 лист. 2015
 *      Author: tko
 */

#ifndef SPI_H_
#define SPI_H_

#ifndef _STDIO_H_
	#include<avr/io.h>
#endif

#define DDR_SPI	DDRB
#define DD_MOSI	PB2
#define DD_MISO	PB3
#define DD_SCK	PB1
#define SET_SCK(f)	(SPCR |= f) // f = 0 -> fclk/4; f = 1 -> fclk/16; f = 2 -> fclk/64; f = 3 -> fclk/128
#define RESET_SCK (SPCR &= ~(3))
#define SET_DOUBLE_SPEED	(SPSR |= (1 << SPI2X))
#define RESET_DOUBLE_SPEED	(SPSR &= ~(1 << SPI2X))
#define INTERRUPT_ENABLE_SPI (SPCR |= (1 << SPIE))
#define INTERRUPT_DISABLE_SPI (SPCR &= ~(1 << SPIE))
#define ENABLE_SPI (SPCR |= (1 << SPE))
#define DISABLE_SPI (SPCR &= ~(1 << SPE))
#define MSB_IS_LAST (SPCR |= (1 << DORD))
#define MSB_IS_FIRST (SPCR &= ~(1 << DORD))
#define SET_SS(module_port, select_module) (module_port |= (1 << select_module))
#define RESET_SS(select_module) (module_port &= ~(1 << select_module))

#define SS_PCF2123 PB1

static inline void SPI_init(void)__attribute__((always_inline));
static inline void SPI_put_into_buffer(unsigned char data)__attribute__((always_inline));
static inline void SPI_transmit_word(unsigned int word)__attribute__((always_inline));

void SPI_init(void)
{
	//Set data direction
	DDR_SPI |= (1 << DD_MOSI) | (1 << DD_SCK) | (1 << SS_PCF2123);
	//Setting serial peripheral control register
	SPCR |= (1 << MSTR);
	ENABLE_SPI;
	//transfer frequency
	SET_SCK(1);//1MHz
	//initialize io for selecting chip
	PORTB |= (1 << SS_PCF2123);
}

void SPI_put_into_buffer(unsigned char data)
{
	if(SPSR & (1 << SPIF))
		SPDR = data;
}

//WORNING: works only MSB_first mode
void SPI_transmit_word(unsigned int word)
{
	if(SPSR & (1 << SPIF))
	{
		SPI_put_into_buffer((unsigned char) (word >> 8));
		while(!(SPSR & (1 << SPIF)));
		SPI_put_into_buffer((unsigned char) (word));
	}
}

/*
#undef DD_SS
#undef DD_SCK
#undef DD_MISO
#undef DD_MOSI
#undef DDR_SPI
*/

#endif /* SPI_H_ */
