/*
 * main.c
 *
 *  Created on: 2 лист. 2015
 *      Author: tko
 */
#include "pcf2123.h"
#include "SPI.h"
#include <avr/interrupt.h>

/**************READ**WRITE**MODES**DEFINITION****************************/
 #define R_W   	7//Must be set to 1 when register is read

/*************************************PCF2123 REGISTER BITS DEFINITION*********/
/**************REG CONTROL_1***********************************************/
typedef enum
{
	N0,
	CIE,
	_12_24,
	N1,
	SR,
	STOP,
	N2,
	EXT_TEST,
}CTRL_1_REG;
/***************REG CONTROL_2************************************************************/
typedef enum
{
	MI,
	SI,
	MSF,
	TI_TP,
	TF,
	AF,
	AIE,
	TIE,
}CTRL_2_REG;
/******************************************************************************/
/****************REGISTER**DEFINITION************************************/
typedef enum
{
	control_1 = 0x10,
	control_2 = 0x11,
	seconds = 0x12,
	minutes = 0x13,
	hours = 0x14,
	days = 0x15,
	weekdays = 0x16,
	months = 0x17,
	years = 0x18,
	minute_alarm = 0x19,
	hour_alarm = 0x1A,
	day_alarm = 0x1B,
	weekday_alarm = 0x1C,
	offset_register = 0x1D,
	timer_clkout = 0x1E,
	countdown_timer = 0x1F,
} REG_TYPE;
/********************COMMAND**DEFINITION**************************/

#define RESET_RTC 				 0x1058
#define ENABLE_INTERRUPT_SECONDS ((unsigned int)(control_2 << 8) | 1 << SI)
#define ENABLE_INTERRUPT_MINUTES ((unsigned int)(control_2 << 8) | 1 << MI)
#define DISABLE_INTERRUPTS_ALL ((unsigned int)(control_2 << 8))
#define CLEAR_INT_FLAG
#define SET_OFFSET
/************************************************************/


#define ENABLE 		1
#define DISABLE 	0
#define SECOND_INT
#define MIN_INT
#define TIMER_INT
#define PCF_INT PB1

typedef struct
{
   unsigned char seconds;
   unsigned char minutes;
   unsigned char hours;
   unsigned char days;
   unsigned char weekdays;
   unsigned char months;
   unsigned char years;
}pcf2123_data_reg;

typedef struct
{
   unsigned char minute;
   unsigned char hour;
   unsigned char day;
   unsigned char weekday;
}pcf2123_alarm_reg;

typedef struct
{
   unsigned char control;
   unsigned char countdown;
}pcf2123_ctrl;

struct
{
	pcf2123_data_reg 	time_data;
	pcf2123_alarm_reg 	alarm_data;
	pcf2123_ctrl 		ctrl_data;
}rtc, *rtc_ptr;

void rtc_transmit_command(unsigned int command)
{
	SPI_transmit_word(command);
}

void rtc_transmit_data(unsigned char register_name, unsigned char data)
{
   SPI_put_into_buffer(register_name);
   SPI_put_into_buffer(data);
}

unsigned char rtc_receive_data(unsigned int register_name)
{
   register_name |= (1 << R_W);
   SPI_put_into_buffer((unsigned char)register_name << 8);
   SPI_put_into_buffer(0);
   return SPDR;
}

unsigned char rtc_control_os_flag(void)
{
   if(rtc_receive_data(seconds) & 0x80)
   {
      return 1;
   }
   return 0;
}

void rtc_handle_alarm(void)
{
	rtc_transmit_data(control_2, ~(1 << AF));
	/*DEBUG FUNC read control 2 and put it to uart buffer*/
	//set new alarm time
	//handle alarm
}

void rtc_update_date(void)
{
	rtc_transmit_data(control_2, ~(1 << MSF));
	rtc_ptr->time_data.minutes 		 = rtc_receive_data(minutes); // split to separate function
	rtc_ptr->time_data.hours   		 = rtc_receive_data(hours);
	rtc_ptr->time_data.weekdays  	 = rtc_receive_data(weekdays);
	rtc_ptr->time_data.days   		 = rtc_receive_data(days);
	rtc_ptr->time_data.months  		 = rtc_receive_data(months);
	rtc_ptr->time_data.years  		+= rtc_receive_data(years);
}

void rtc_handle_countdown_timer(void)
{
	rtc_transmit_data(control_2, ~(1 << TF));
	//to do action should be done after timer out for instance set a DO.
}

void rtc_handle_interrupts(void)
{
	if(GICR & (1 << INT0))
	{
		if(rtc_receive_data(control_2) & (1 << MSF))
		{
			rtc_update_date();
		}
		if(rtc_receive_data(control_2) & (1 << AF))
		{
			rtc_handle_alarm();
		}
		if(rtc_receive_data(control_2) & (1 << TF))
		{
			rtc_handle_countdown_timer();
		}
	}
}

void rtc_adjust_clk(unsigned char offset_value)
{
	rtc_transmit_data(offset_register, offset_value);
}

void rtc_set_alarm_time(unsigned char minute, unsigned char hour)
{
	;
}

// add external interrupt that will initiate data receiving

int main (void)
{
	SPI_init();

	rtc_transmit_data(years, 0);// rtc_ptr->time_data->years should be set via LCD display to current year
	rtc_transmit_data(minutes,  rtc_ptr->time_data.minutes);
	rtc_transmit_data(seconds,  rtc_ptr->time_data.seconds);
	rtc_transmit_data(hours, 	rtc_ptr->time_data.hours);
	rtc_transmit_data(weekdays, rtc_ptr->time_data.weekdays);
	rtc_transmit_data(days, 	rtc_ptr->time_data.days);

	rtc_transmit_data(day_alarm, 	 rtc_ptr->alarm_data.day);
	rtc_transmit_data(weekday_alarm, rtc_ptr->alarm_data.weekday);
	rtc_transmit_data(hour_alarm, 	 rtc_ptr->alarm_data.hour);





	rtc_ptr->time_data.minutes 		 = rtc_receive_data(minutes);
	rtc_ptr->time_data.hours   		 = rtc_receive_data(hours);
	rtc_ptr->time_data.weekdays  	 = rtc_receive_data(weekdays);
	rtc_ptr->time_data.days   		 = rtc_receive_data(days);
	rtc_ptr->time_data.months  		 = rtc_receive_data(months);
	rtc_ptr->time_data.years  		+= rtc_receive_data(years);

	PORTB |= 1;
	return 0;
}

void send_message_to_UDR(char * message, int integer)
{
	char* buff = (char*) malloc((sizeof(int)*8+1));
	itoa(integer, buff,10);
	do
	{
		printf("%c",*message);//Function to put char to UDR should be here
	}while(*++message);
	//while(!*(++buff))
	do
	{
		printf("%c", *buff);//Function to put char to UDR should be here
	}while(*++buff);
	//Add commands to run cursor to new line or create new function for this
}
