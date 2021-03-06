/*
 * main.c
 *
 *  Created on: 2 лист. 2015
 *      Author: tko
 */
#include <avr/io.h>
#include "pcf2123.h"
#include "SPI.h"
#include "UART.h"

/**************READ**WRITE**MODES**DEFINITION****************************/
#define R_W   	7//Must be set to 1 when register is read
//#define DEBUG  //uncomment the line for debugging

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

typedef enum
{
	CTD0,
	CTD1,
	TE = 3,
	COF0,
	COF1,
	COF2
}TIMER_CLKOUT_REG;

typedef enum
{
	AE_M = 7,
}MINUTE_ALARM_REG;

typedef enum
{
	AE_H = 7,
	AMPM = 5,
}HOUR_ALARM_REG;

typedef enum
{
	AE_D = 7,
}DAY_ALARM_REG;

typedef enum
{
	AE_W = 7,
}WEEKDAY_ALARM_REG;
/********************COMMANDS**DEFINITION**************************/

#define RESET_RTC 				 0x1058
#define ENABLE_INTERRUPT_SECONDS ((unsigned int)(control_2 << 8) | 1 << SI)
#define ENABLE_INTERRUPT_MINUTES ((unsigned int)(control_2 << 8) | 1 << MI)
#define DISABLE_TIME_INTERRUPTS ((unsigned int)(control_2 << 8))
#define ENABLE_MINUTE_ALARM ((unsigned int)(timer_clkout << 8) | 1 << AE_M)
#define ENABLE_HOUR_ALARM ((unsigned int)(timer_clkout << 8) | 1 << AE_H | 1 << AE_M)
#define ENABLE_DAY_ALARM ((unsigned int)(timer_clkout << 8) | 1 << AE_D | 1 << AE_H | 1 << AE_M)
#define ENABLE_WEEKDAY_ALARM ((unsigned int)(timer_clkout << 8) | 1 << AE_W | 1 << AE_D | 1 << AE_H | 1 << AE_M)
#define DISABLE_ALL_ALARM ((unsigned int)(timer_clkout << 8))
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
   unsigned char clk_out_frq;
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
#ifdef DEBUG
	send_message_to_UDR("Control 2 ", rtc_receive_data(control_2), 16);
#endif
	//set new alarm time in if is needed
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
	rtc_transmit_data(offset_register, offset_value);// should be supplemented
}

void rtc_configure_alarms(unsigned char alarm_intr_flags)//first four bits should be set according to needed type of alarm
{
	rtc_transmit_data(minute_alarm, (((alarm_intr_flags << 7) & (1 << AE_M)) | rtc_ptr->alarm_data.minute));
	rtc_transmit_data(hour_alarm, (((alarm_intr_flags << 6) & (1 << AE_H)) | rtc_ptr->alarm_data.hour));
	rtc_transmit_data(day_alarm, (((alarm_intr_flags << 5) & (1 << AE_D)) | rtc_ptr->alarm_data.day));
	rtc_transmit_data(weekday_alarm, (((alarm_intr_flags << 4) & (1 << AE_W)) | rtc_ptr->alarm_data.weekday));
}

void rtc_set_time(void)
{
	rtc_transmit_data(seconds,  rtc_ptr->time_data.seconds);
	SPI_put_into_buffer(rtc_ptr->time_data.minutes);
	SPI_put_into_buffer(rtc_ptr->time_data.hours);
	SPI_put_into_buffer(rtc_ptr->time_data.days);
	SPI_put_into_buffer(rtc_ptr->time_data.weekdays);
	SPI_put_into_buffer(rtc_ptr->time_data.months);
	SPI_put_into_buffer(rtc_ptr->time_data.years);

}

void rtc_set_alarm(unsigned char alarm_intr_flags)//first four bits should be set
{
	rtc_transmit_data(minute_alarm, (((alarm_intr_flags << 7) & (1 << AE_M)) | rtc_ptr->alarm_data.minute));
	SPI_put_into_buffer((((alarm_intr_flags << 6) & (1 << AE_H)) | rtc_ptr->alarm_data.hour));
	SPI_put_into_buffer((((alarm_intr_flags << 5) & (1 << AE_D)) | rtc_ptr->alarm_data.day));
	SPI_put_into_buffer((((alarm_intr_flags << 4) & (1 << AE_W)) | rtc_ptr->alarm_data.weekday));
}

unsigned char rtc_ctrl_clkout(unsigned char clk_out_frq)
{
	/*freq_source_clk  = 0b111 - high-Z
	 * freq_source_clk = 0b110 - 1
	 * freq_source_clk = 0b101 - 1024
	 * freq_source_clk = 0b100 - 2048
	 * freq_source_clk = 0b011 - 4096
	 * freq_source_clk = 0b010 - 8192
	 * freq_source_clk = 0b001 - 16384
	 * freq_source_clk = 0b000 - 32768*/
	return (clk_out_frq & (1 << COF2 | 1 << COF1 | 1 << COF0));
}

void rtc_set_countdown_timer(unsigned char alarm_intr_flags, unsigned char clk_ctrl)
{
	rtc_transmit_data(timer_clkout, (rtc_ctrl_clkout( rtc->ctrl_data.clk_out_frq) | ((alarm_intr_flags >> 1) & (1 << TE)) | (clk_ctrl & (1 << CTD1)) | (clk_ctrl & (1 << CTD0))));
	rtc_transmit_data(countdown_timer, (rtc_ptr->ctrl_data.countdown));
}

// add external interrupt that will initiate data receiving

int main (void)
{
	SPI_init();
	rtc_ptr = &rtc;
	init_uart( );
	init_integer_buff( );


	rtc_transmit_data(years, 0);// rtc_ptr->time_data->years should be set via LCD display to current year
	rtc_transmit_data(minutes,  rtc_ptr->time_data.minutes);
	rtc_transmit_data(seconds,  rtc_ptr->time_data.seconds);
	rtc_transmit_data(hours, 	rtc_ptr->time_data.hours);
	rtc_transmit_data(weekdays, rtc_ptr->time_data.weekdays);
	rtc_transmit_data(days, 	rtc_ptr->time_data.days);

	rtc_transmit_data(day_alarm, 	 rtc_ptr->alarm_data.day);
	rtc_transmit_data(weekday_alarm, rtc_ptr->alarm_data.weekday);
	rtc_transmit_data(hour_alarm, 	 rtc_ptr->alarm_data.hour);
	rtc_transmit_data(hour_alarm, 	 rtc_ptr->alarm_data.minute);





	rtc_ptr->time_data.minutes 		 = rtc_receive_data(minutes);
	rtc_ptr->time_data.hours   		 = rtc_receive_data(hours);
	rtc_ptr->time_data.weekdays  	 = rtc_receive_data(weekdays);
	rtc_ptr->time_data.days   		 = rtc_receive_data(days);
	rtc_ptr->time_data.months  		 = rtc_receive_data(months);
	rtc_ptr->time_data.years  		+= rtc_receive_data(years);

	rtc_ptr->time_data.minutes = 89;
	rtc_ptr->time_data.hours = 18;

	send_message_to_UDR("Minutes ", rtc_ptr->time_data.minutes, 16);
	send_message_to_UDR("Hours ", rtc_ptr->time_data.hours, 16);

	while(1)
	{
		;
	}

	return 0;
}
