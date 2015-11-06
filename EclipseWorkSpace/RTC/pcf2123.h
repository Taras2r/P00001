/*
 * pcf2123.h
 *
 *  Created on: 2 лист. 2015
 *      Author: tko
 */

#ifndef PCF2123_H_
#define PCF2123_H_

#ifndef _STDIO_H_
	#include <avr/io.h>
#endif

static inline void SPI_put_into_buffer(unsigned char data)__attribute__((always_inline));
void rtc_transmit_command(unsigned int command);
void rtc_transmit_data(unsigned char register_name, unsigned char data);
unsigned char rtc_receive_data(unsigned int register_name);
static inline unsigned char rtc_control_os_flag(void)__attribute__((always_inline));

#endif /* PCF2123_H_ */
