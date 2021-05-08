/*=============================================================================
 * Author: Ignacio Majul <imajul89@gmail.com>
 * Date: 2019/12/10
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _DS3132_H_
#define _DS3132_H_

/*==================[inclusions]=============================================*/

#include "sapi.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

// ----------------------------------------------------------------------------
#define EVERY_SECOND			0x0F
#define SECONDS_MATCH			0x0E
#define SEC_MIN_MATCH			0x0C
#define HOUR_SEC_MIN_MATCH		0x08
#define DAY_HOUR_SEC_MIN_MATCH	0x00

/*==================[typedef]================================================*/

typedef struct{

	uint8_t  year;	 /* 0 to 99 */
    uint8_t  month;  /* 1 to 12   */
	uint8_t  mday;	 /* 1 to 31   */
	uint8_t  wday;	 /* 1 to 7    */
	uint8_t  hour;	 /* 0 to 23   */
	uint8_t  min;	 /* 0 to 59   */
	uint8_t  sec;	 /* 0 to 59   */
	uint8_t  alarm1_seconds;	 /* 0 to 59   */
	uint8_t  alarm1_minutes;	 /* 0 to 59   */
	uint8_t  alarm1_hours;	 /* 1–12 + AM/PM / 00–23 */
	uint8_t  alarm1_DY_DT;	 /* 01 to 7  or 1 to 31 */
	uint8_t  alarm2_minutes;	 /* 0 to 59   */
	uint8_t  alarm2_hours;	 /* 1–12 + AM/PM / 00–23 */
	uint8_t  alarm2_DY_DT;	 /* 01 to 7  or 1 to 31 */
	uint8_t control;
	uint8_t control_status;
	uint8_t aging_offset;
	uint8_t MSB_temp;     	/* temperatura byte superior */
	uint8_t LSB_temp;		/* temperatura byte inferior */

} rtcDS3231_t;

/*==================[external functions declaration]=========================*/

//-----------------------------------------------------------------------------
//  RTC MANAGEMENT
//-----------------------------------------------------------------------------

uint8_t RTC_setAddress( void );

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------

void RTC_Init( rtcDS3231_t* now, uint8_t year, uint8_t month, uint8_t month_day, uint8_t week_day, uint8_t hour, uint8_t minute, uint8_t second );

//-----------------------------------------------------------------------------
// WRITE OPERATIONS
//-----------------------------------------------------------------------------

// Write time registers
void RTC_write_time( rtcDS3231_t* time, int32_t i2c, uint8_t address);

// Reset alarm flags
void RTC_reset_alarm( rtcDS3231_t* time, int32_t i2c, uint8_t address);

// 	byte alarmBits sets the behavior of the alarms:
//	alarmaBits	Rate
//	0xF			Once per second
//	0xE			Alarm when seconds match
//	0xC			Alarm when min, sec match
//	0x8			Alarm when hour, min, sec match
//	0x0			Alarm when date, h, m, s match
void RTC_set_alarm_time(rtcDS3231_t* time, int32_t i2c, uint8_t address, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint8_t alarmBits);

void RTC_turn_alarm_on(rtcDS3231_t* time, int32_t i2c, uint8_t address);

void RTC_turn_alarm_off(rtcDS3231_t* time, int32_t i2c, uint8_t address);

//-----------------------------------------------------------------------------
// READ OPERATIONS
//-----------------------------------------------------------------------------

// Read time registers
void RTC_read_time( rtcDS3231_t* now, int32_t i2c, uint8_t address);

void RTC_read_temp( rtcDS3231_t* now, int32_t i2c, uint8_t address);

void RTC_read_control_registers( rtcDS3231_t* now, int32_t i2c, uint8_t address);

// Auxiliary functions

uint8_t decToBcd(uint8_t val);

uint8_t bcdToDec(uint8_t val);


/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _DS3132_H_ */
