/*=============================================================================
 * Author: Ignacio Majul <imajul89@gmail.com>
 * Date: 2019/12/10
 *===========================================================================*/

/*==================[inclusions]=============================================*/
#include "rtc_DS3231.h"   // <= own header
#include "sapi.h"

/*==================[internal data definition]===============================*/
DEBUG_PRINT_ENABLE

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------

void RTC_Init( rtcDS3231_t* now, uint8_t year, uint8_t month, uint8_t month_day, uint8_t week_day, uint8_t hour, uint8_t minute, uint8_t second )
{
	now->year = (((year / 10) << 4)|(year % 10));
	now->month = (((month / 10) << 4)|(month % 10));
	now->mday = (((month_day / 10) << 4)|(month_day % 10));
	now->wday = (((week_day / 10) << 4)|(week_day % 10));
	now->hour = (((hour / 10) << 4)|(hour % 10));
	now->min = (((minute / 10) << 4)|(minute % 10));
	now->sec= (((second / 10) << 4)|(second % 10));
	now->alarm1_seconds = 0;	 /* 0 to 59   */
	now->alarm1_minutes = 0;	 /* 0 to 59   */
	now->alarm1_hours = 0;	 /* 1–12 + AM/PM / 00–23 */
	now->alarm1_DY_DT = 0;	 /* bit 7 = Alarm when hours, minutes, and seconds match. LSB=01 to 7  or 1 to 31 */
	now->alarm2_minutes = 0;	 /* 0 to 59   */
	now->alarm2_hours = 0;	 /* 1–12 + AM/PM / 00–23 */
	now->alarm2_DY_DT = 0;	 /* 01 to 7  or 1 to 31 */
	now->control = 0;  /* Alarm 1 enable & Interrupt Control enable */
	now->control_status = 0;
	now->aging_offset;
	now->MSB_temp;     	/* temperatura byte superior */
	now->LSB_temp;		/* temperatura byte inferior */
}

//-----------------------------------------------------------------------------
// WRITE OPERATIONS
//-----------------------------------------------------------------------------

// Write time registers
void RTC_write_time( rtcDS3231_t* time, int32_t i2c, uint8_t address)
{
	uint8_t dato[17];

	dato[0]=0x00;
	dato[1]=time->sec;
	dato[2]=time->min;
	dato[3]=time->hour;
	dato[4]=time->wday;
	dato[5]=time->mday;
	dato[6]=time->month;
	dato[7]=time->year;
	dato[8]=time->alarm1_seconds;
	dato[9]=time->alarm1_minutes;
	dato[10]=time->alarm1_hours;
	dato[11]=time->alarm1_DY_DT;
	dato[12]=time->alarm2_minutes;
	dato[13]=time->alarm2_hours;
	dato[14]=time->alarm2_DY_DT;
	dato[15]=time->control;
	dato[16]=time->control_status;

	i2cWrite(i2c, address,dato,17,TRUE );
}

// Reset alarm flags
void RTC_reset_alarm( rtcDS3231_t* time, int32_t i2c, uint8_t address)
{
	uint8_t dato[2];

	RTC_read_control_registers( time, i2c, address);

	dato[0]=0x0F;
	dato[1]=time->control_status & ~(1<<0);

	i2cWrite(i2c, address,dato,2,TRUE );
}

// Set alarm 1
void RTC_set_alarm_time(rtcDS3231_t* time, int32_t i2c, uint8_t address, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t alarmBits)
{
	uint8_t dato[5];

	dato[0]=0x07;
	dato[1]= decToBcd(second) | ((alarmBits & 0b00000001) << 7);
	dato[2]= decToBcd(minute) | ((alarmBits & 0b00000010) << 6);
	dato[3]= decToBcd(hour)   | ((alarmBits & 0b00000100) << 5);
	dato[4]= decToBcd(day)    | ((alarmBits & 0b00001000) << 4);

	i2cWrite(i2c,address,dato,5,TRUE );

}

void RTC_turn_alarm_on(rtcDS3231_t* time, int32_t i2c, uint8_t address)
{
	uint8_t dato[2];

	dato[0]=0x0E;
	dato[1]=0b00000101;

	i2cWrite(i2c, address,dato,2,TRUE );
}

void RTC_turn_alarm_off(rtcDS3231_t* time, int32_t i2c, uint8_t address)
{
	uint8_t dato[2];

	dato[0]=0x0E;
	dato[1]=0;

	i2cWrite(i2c, address,dato,2,TRUE );
}

//-----------------------------------------------------------------------------
// READ OPERATIONS
//-----------------------------------------------------------------------------

// Read time registers
void RTC_read_time( rtcDS3231_t* now, int32_t i2c, uint8_t address)
{
	uint8_t lectura[7];
	uint8_t dato = 0x00;

//	i2cWrite(i2c, address,0x00,1,FALSE );
//	i2cRead( i2c,  address,(uint8_t*)0, 0, FALSE, lectura, 19, TRUE);

	i2cRead( i2c, address, &dato, 1, FALSE, lectura, 7, TRUE);

//	now->year = lectura[10];
//	now->month = lectura[9];
//	now->mday = lectura[8];
//	now->wday = lectura[7];
//	now->hour = lectura[6];
//	now->min = lectura[5];
//	now->sec = lectura[4];
//
//	now->MSB_temp = (int8_t)lectura[2];     	  /* temperatura byte superior */
//	now->LSB_temp = ((uint8_t)lectura[3]>>6)*25;		/* temperatura byte inferior */

	now->year = lectura[6];
	now->month = lectura[5];
	now->mday = lectura[4];
	now->wday = lectura[3];
	now->hour = lectura[2];
	now->min = lectura[1];
	now->sec = lectura[0];

//	now->MSB_temp = (int8_t)lectura[17];     	  /* temperatura byte superior */
//	now->LSB_temp = ((uint8_t)lectura[18]>>6)*25;		/* temperatura byte inferior */

}

void RTC_read_temp( rtcDS3231_t* now, int32_t i2c, uint8_t address)
{
	uint8_t lectura[2];
	uint8_t dato = 0x11;

	i2cRead( i2c, address, &dato, 1, FALSE, lectura, 2, TRUE);

	now->MSB_temp = (int8_t)lectura[0];     	  /* temperatura byte superior */
	now->LSB_temp = ((uint8_t)lectura[1]>>6)*25;		/* temperatura byte inferior */

}

void RTC_read_control_registers( rtcDS3231_t* now, int32_t i2c, uint8_t address)
{
	uint8_t lectura[2];
	uint8_t dato = 0x0E;

	i2cRead( i2c, address, &dato, 1, FALSE, lectura, 2, TRUE);

	now->control = lectura[0];     	 		/* registro control */
	now->control_status = lectura[1];		/* registro control/status */

}

uint8_t decToBcd(uint8_t val)
{
// Convert normal decimal numbers to binary coded decimal
	return ( (val/10*16) + (val%10) );
}

uint8_t bcdToDec(uint8_t val)
{
// Convert binary coded decimal to normal decimal numbers
	return ( (val/16*10) + (val%16) );
}


/*==================[ISR external functions definition]======================*/

/*==================[end of file]============================================*/
