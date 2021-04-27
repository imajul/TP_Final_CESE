/*=============================================================================
 * Program: TPfinal
 * Date: 2021/04/11
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "TP_PCSE.h"

#include "rtc_DS3231.h"
//#include "antiRebote.h"
#include "sapi.h"

/*=====[Definition macros of private constants]==============================*/

DEBUG_PRINT_ENABLE

UART_485

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

rtcDS3231_t time;
Eeprom24C32_t eeprom24C32;
estados_t estadoTecla;

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
	// ----- Setup -----------------------------------
	boardInit();
	debugPrintConfigUart( UART_USB, 115200 );

	uint32_t i=0;
	uint16_t eeprom_address = EEPROM24C32_FIRST_MEMORY_ADDRESS;
	uint8_t readedByte = 0;

	i2cInit( I2C0, 100000 );
	debugPrintlnString( "I2C initialization complete." );

	RTC_Init(&time);  // inicializo la estructura time con los registros horarios
	debugPrintlnString( "RTC initialization complete." );

	eeprom24C32Init( &eeprom24C32, I2C0, 1, 1, 1, EEPROM24C32_PAGE_SIZE, EEPROM_32_K_BIT );  // inicializo la EEPROM
	debugPrintlnString( "EEPROM initialization complete." );

	RTC_write_time(&time, I2C0);  // cargo la hora en el RTC DS3231

	eeprom24C32WriteDate(&eeprom24C32, &eeprom_address, time); // escribo la fecha en la EEPROM

	while( true )
	{
		time = RTC_read_time( &time, I2C0);  // leo los registros horarios del RTC y los guardo en la estructura time

		debugPrintHex( time.hour,8 );   // imprimo la hora por UART
		debugPrintString(":");
		debugPrintHex( time.min,8 );
		debugPrintString(":");
		debugPrintHex( time.sec,8 );
		debugPrintString("  ");
		debugPrintHex( time.mday,8 );
		debugPrintString("/");
		debugPrintHex( time.month,8 );
		debugPrintString("/");
		debugPrintHex( time.year,8 );

		debugPrintString(" Temp: ");	// imprimo la temperatura por UART
		debugPrintInt(time.MSB_temp);
		debugPrintString(",");
		debugPrintInt(time.LSB_temp);
		debugPrintString(" Pin: ");
		debugPrintInt(gpioRead(GPIO1));

		if(!gpioRead(GPIO1))	// verifico si el RTC genera una alarma o  si se pulsa la tecla
		{
			debugPrintString(" ALARMA !! \r\n");

			RTC_reset_alarm(&time, I2C0);	// el registro queda alarmado luego del primer disparo -> debo resetearlo

			eeprom24C32WriteByte( &eeprom24C32, eeprom_address, time.MSB_temp); // escribo la temperatura leida por I2C en la EEPROM

			eeprom24C32ReadRandom( &eeprom24C32, eeprom_address, &readedByte );  // leo la hora escrita e imprimo
			debugPrintString( "Temp leida de EEPROM: " );
			debugPrintInt( readedByte );

			if(eeprom_address == EEPROM24C32_LAST_MEMORY_ADDRESS) {    // si llego al ultimo registro de la EEPROM genero overflow de direccon
				eeprom_address = EEPROM24C32_FIRST_MEMORY_ADDRESS;
			} else {
				eeprom_address++;
			}
		}

		debugPrintString("\r\n ");
		delay(1000);

	}

	// YOU NEVER REACH HERE, because this program runs directly or on a
	// microcontroller and is not called by any Operating System, as in the
	// case of a PC program.
	return 0;
}
