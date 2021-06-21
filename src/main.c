/*=============================================================================
 * Program: TPfinal
 * Date: 2021/04/11
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi.h"
#include "eeprom_24C32.h"
#include "DS18B20.h"
#include "rtc_DS3231.h"

/*=====[Definition macros of private constants]==============================*/

DEBUG_PRINT_ENABLE

#define RTC_SET			OFF
#define PRINT_DEBUG 	ON
#define EEPROM_RESET	OFF

#define	SENSOR_TEMP_SUCIO	GPIO7
#define	SENSOR_TEMP_LIMPIO	GPIO8

#define	DS3231_I2C_ADDRESS	0x68

// SETEO DE LA HORA
#define ANIO 			21
#define MES 			5
#define DIA_MES 		8
#define DIA_SEMANA 		7
#define HORA 			16
#define MINUTOS			16
#define SEGUNDOS		20

// SETEO DE LA ALARMA
#define ALARMA_HORA 	20
#define ALARMA_MINUTOS 	30
#define ALARMA_SEGUNDOS 5
#define ALARMA_DIA		0

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

uint8_t flag = 0;
rtcDS3231_t time;
Eeprom24C32_t eeprom24C32;

/*=====[Definitions of private global variables]=============================*/

typedef union
{
	struct
	{
		int32_t  T_clean;	/* Temperature measurement from clean panel */
		int32_t  T_dirty;	/* Temperature measurement from dirty panel */
		uint32_t soil;		/* Soling percentage calculated each day */
		uint16_t I_clean;   /* Current measurement from clean panel */
		uint16_t I_dirty;	/* Current measurement from dirty panel */
		uint8_t  mday;		/* 1 to 31  */
		uint8_t  month;  	/* 1 to 12  */
		uint8_t  year;	 	/* 0 to 99  */
	};		 				// size: 20 bytes

	uint8_t buffer[20];

} soiling_t;  // size: 20 bytes

/*=====[Main function, program entry point after power on or reset]==========*/

void My_IRQ_Init (void);

int main( void )
{
	// ----- Setup -----------------------------------
	boardInit();
	gpioConfig( GPIO2, GPIO_INPUT_PULLUP);
	gpioConfig( SENSOR_TEMP_SUCIO, GPIO_INPUT );

	debugPrintConfigUart( UART_USB, 115200 );

	uint32_t i=0;
	char str[5];
	uint16_t eeprom_current_address = EEPROM24C32_FIRST_MEMORY_ADDRESS;

	soiling_t soil_sample;
	soiling_t *soil_read;
	uint8_t readedByte[sizeof(soil_sample)];

	i2cInit( I2C0, 100000 );
	debugPrintlnString( "I2C initialization complete." );

	eeprom24C32Init( &eeprom24C32, I2C0, 1, 1, 1, EEPROM24C32_PAGE_SIZE, EEPROM24C32_MEMORY_SIZE );  // inicializo la EEPROM
	debugPrintlnString( "EEPROM initialization complete." );

#if EEPROM_RESET

	eeprom24C32Reset(&eeprom24C32);
	debugPrintlnString( "EEPROM reset" );

#endif

#if RTC_SET

	RTC_Init(&time,ANIO,MES,DIA_MES,DIA_SEMANA,HORA,MINUTOS,SEGUNDOS);  // inicializo la estructura time con los registros horarios
	debugPrintlnString( "RTC initialization complete." );
	RTC_write_time(&time, I2C0, DS3231_I2C_ADDRESS);  // cargo la hora en el RTC DS3231

#endif

	RTC_set_alarm_time(&time,I2C0, DS3231_I2C_ADDRESS,0,ALARMA_HORA,ALARMA_MINUTOS,ALARMA_SEGUNDOS,EVERY_SECOND);
	RTC_turn_alarm_on(&time, I2C0, DS3231_I2C_ADDRESS);
	RTC_reset_alarm(&time, I2C0, DS3231_I2C_ADDRESS);

	owInit(SENSOR_TEMP_SUCIO);
	owSetBitResolution(SENSOR_TEMP_SUCIO,12);
	debugPrintlnString( "Temperature sensor initialization complete." );	// inicializo el sensor de temperatura

//	eeprom_current_address = eeprom24C32GetCurrentAddress(&eeprom24C32);

	My_IRQ_Init();

	while( TRUE )
	{
		if(flag)
		{
			flag = 0;
			NVIC_DisableIRQ(PIN_INT0_IRQn);

			RTC_reset_alarm(&time, I2C0, DS3231_I2C_ADDRESS);
			RTC_read_time( &time, I2C0, DS3231_I2C_ADDRESS);  // leo los registros horarios del RTC y los guardo en la estructura time

			debugPrintString(" ALARMA !! ");
			debugPrintHex( time.hour,8 );   // imprimo la hora por UART
			debugPrintString(":");
			debugPrintHex( time.min,8 );
			debugPrintString(":");
			debugPrintHex( time.sec,8 );

			soil_sample.T_clean = owReadTemperature(SENSOR_TEMP_SUCIO);
			soil_sample.T_dirty = owReadTemperature(SENSOR_TEMP_SUCIO);
			soil_sample.soil = 10;
			soil_sample.I_clean = 20;
			soil_sample.I_dirty = 30;
			soil_sample.mday = time.mday;
			soil_sample.month = time.month;
			soil_sample.year = time.year;

			eeprom_current_address = eeprom24C32GetCurrentAddress(&eeprom24C32);

			for(i=0 ; i<sizeof(soil_sample) ; i++)
			{
				eeprom24C32WriteByte( &eeprom24C32, eeprom_current_address, soil_sample.buffer[i]);
				eeprom_current_address++;
			}

			eeprom24C32UpdateDataAddress(&eeprom24C32, eeprom_current_address);

#if PRINT_DEBUG		/* print last stored data */

			eeprom_current_address -= sizeof(soil_sample);

			debugPrintString(" current address: ");
			debugPrintInt(eeprom_current_address);

			for(i=0 ; i<sizeof(soil_sample) ; i++)
			{
				eeprom24C32ReadRandom( &eeprom24C32, eeprom_current_address, readedByte+i);
				eeprom_current_address++;
			}

			soil_read = (soiling_t *)readedByte;
			debugPrintString(" Tc: ");
			sprintf(str, "%d.%04d", (soil_read->T_clean)>>4, (soil_read->T_clean & 0xF) * 625);
			debugPrintString(str);
			debugPrintString(" Td: ");
			sprintf(str, "%d.%04d", (soil_read->T_dirty)>>4, (soil_read->T_dirty & 0xF) * 625);
			debugPrintString(str);
			debugPrintString(" Soil: ");
			debugPrintUInt( soil_read->soil );
			debugPrintString(" Ic: ");
			debugPrintUInt( soil_read->I_clean );
			debugPrintString(" Id: ");
			debugPrintUInt( soil_read->I_dirty );
			debugPrintString("  ");
			debugPrintHex( soil_read->mday,8 );
			debugPrintString("/");
			debugPrintHex( soil_read->month,8 );
			debugPrintString("/");
			debugPrintHex( soil_read->year,8 );

#endif
			debugPrintEnter();
			NVIC_EnableIRQ(PIN_INT0_IRQn);
		}

			sleepUntilNextInterrupt();
		//		__WFI();

	}

	return 0;
}

void GPIO0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0); // Borramos el flag de interrupción
	flag = 1;
}

void My_IRQ_Init (void)
{
	Chip_PININT_Init(LPC_GPIO_PIN_INT);
	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
	Chip_SCU_GPIOIntPinSel(0, 3, 4);  // Canal 0, Puerto 3, Pin 4 correspondiente al pin GPIO2 de la EDU-CIAA

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);

	NVIC_SetPriority(PIN_INT0_IRQn, 5);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
}
