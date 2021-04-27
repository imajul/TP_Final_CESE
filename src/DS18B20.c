/*=============================================================================
 * Program: DS18B20
 * Date: 2021/04/18
 *===========================================================================*/

#include "board.h"
#include "chip.h"
#include "sapi.h"
#include "DS18B20.h"

/*==================[macros and definitions]=================================*/

#define owOUT(port,pin)		Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, port, pin)
#define owIN(port,pin)		Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, pin)
#define owREAD(port,pin)	Chip_GPIO_GetPinState(LPC_GPIO_PORT, port, pin)
#define owLOW(port,pin)		Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, port, pin)
#define owHIGH(port,pin)	Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT, port, pin)

/*==================[internal data declaration]==============================*/

static int8_t high_temp_alarm = HIGH_TEMP_ALARM;
static int8_t low_temp_alarm = LOW_TEMP_ALARM;
static uint8_t bit_resolution = RES_12_BIT;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

extern pinInitGpioLpc4337_t gpioPinsInit[];

/*==================[internal functions definition]==========================*/

static void pauses(uint32_t t)
{
    DWT_CTRL |= 1;
	DWT_CYCCNT = 0;
	t *= (SystemCoreClock/1000000);

	while(DWT_CYCCNT < t);

}

static uint8_t owCrc(uint8_t code[], uint8_t n)
{
	volatile uint8_t i, j, a, b, cy=0, crc=0;

	for(i=0; i<n; i++)
	{
		a = code[i];
		b = code[i];
		for(j=0; j<8; j++)
		{
			a ^= crc;
			cy = a&1;
			a = crc;
			if(cy)
				a ^= 0x18;

			if(cy)
			{
				cy = a&1;
				a >>= 1;
				a |= 0x80;
			}
			else
			{
				cy = a&1;
				a >>= 1;
			}
			crc = a;

			if(b&1)
			{
				b >>= 1;
				b |= 0x80;
			}
			else
			{
				b >>= 1;
			}
			a = b;
		}
	}
	return crc;
}

static int owPresence(int port, int pin)
{
	owOUT(port,pin);
	owLOW(port,pin);
	pauses(480);
	owIN(port,pin);
	pauses(40);

	if(owREAD(port,pin)==true)
	{
		return -1;
	}
	else
	{
		while(owREAD(port,pin)==false);
		return 0;
	}
}

static void owCmd(uint8_t cmd, void * buffer, uint8_t n, int port, int pin)
{
	volatile uint8_t i = 1, j;
	volatile uint8_t * p = (uint8_t *)buffer;

	owOUT(port,pin);
	do
	{
		if(cmd & i)
		{
			owLOW(port,pin);
			pauses(3);
			owHIGH(port,pin);
			pauses(60);
		}
		else
		{
			owLOW(port,pin);
			pauses(60);
			owHIGH(port,pin);
			pauses(10);
		}
		if(i==0x80)
		{
			break;
		}
		else
		{
			i <<= 1;
		}
	}while(i != 0);

	for(i=0; i<n; i++)
	{
		p[i] = 0;
		for(j=0; j<8; j++)
		{
			owOUT(port,pin);
			owLOW(port,pin);
			pauses(3);
			owIN(port,pin);
			pauses(12);
			p[i] >>= 1;
			if(owREAD(port,pin)) p[i] |= 0x80;
			pauses(55);
		}
	}
}

/*==================[external functions definition]==========================*/

void owInit(gpioMap_t gpioPin)
{
	uint8_t port,pin;
	port=gpioPinsInit[gpioPin].gpio.port;
	pin=gpioPinsInit[gpioPin].gpio.pin;

	/* Init cycle counter */
	DWT_CTRL |= 1;

    Chip_SCU_PinMux(port, pin, SCU_MODE_INACT | SCU_MODE_ZIF_DIS, SCU_MODE_FUNC0 );
    owIN(port,pin);
}

int owReadROM(void * buffer8, int port, int pin)
{
	int rv = -1;
	uint8_t crc = 0;
	uint8_t * p = (uint8_t *)buffer8;

	if(owPresence(port,pin)==0)
	{
		pauses(400);

		__set_PRIMASK(1);

		owCmd(0x33, p, 8, port, pin); /* READ ROM CMD */

		__set_PRIMASK(0);

		crc = owCrc(p, 7);

		if(crc == p[7])
		{
			rv = 0;
		}
	}
	return rv;
}

int owReadScratch(void * buffer9, int port, int pin)
{
	int rv = -1;
	uint8_t crc = 0;
	uint8_t * p = (uint8_t *)buffer9;

	if(owPresence(port,pin)==0)
	{
		pauses(400);

		__set_PRIMASK(1);

		owCmd(0x33, p, 8, port, pin); /* READ ROM CMD */
		owCmd(0xBE, p, 9, port, pin); /* READ SCRATCHPAD */

		__set_PRIMASK(0);

		crc = owCrc(p, 8);

		if(crc == p[8])
		{
			rv = 0;
		}
	}
	return rv;
}

int owSetBitResolution(gpioMap_t gpioPin, uint8_t res)
{
	uint8_t port,pin;
	port=gpioPinsInit[gpioPin].gpio.port;
	pin=gpioPinsInit[gpioPin].gpio.pin;

	switch(res)
	{
		case 9:
			bit_resolution = RES_9_BIT;
			break;
		case 10:
			bit_resolution = RES_10_BIT;
			break;
		case 11:
			bit_resolution = RES_11_BIT;
			break;
		case 12:
			bit_resolution = RES_12_BIT;
			break;
		default:
			bit_resolution = RES_9_BIT;
			break;
	}

	return owWriteScratch(port, pin);
}

int owSetHighTempAlarm(gpioMap_t gpioPin, int8_t alarm_high)
{
	uint8_t port,pin;
	port=gpioPinsInit[gpioPin].gpio.port;
	pin=gpioPinsInit[gpioPin].gpio.pin;

	high_temp_alarm = alarm_high;
	return owWriteScratch(port, pin);
}

int owSetLowTempAlarm(gpioMap_t gpioPin, int8_t alarm_low)
{
	uint8_t port,pin;
	port=gpioPinsInit[gpioPin].gpio.port;
	pin=gpioPinsInit[gpioPin].gpio.pin;

	low_temp_alarm = alarm_low;
	return owWriteScratch(port, pin);
}

int owWriteScratch(int port, int pin)
{
	int rv = -1;
	uint8_t crc = 0;
	uint8_t p[9];

	if(owPresence(port,pin)==0)
	{
		pauses(400);

		__set_PRIMASK(1);

		owCmd(0x33, p, 8, port, pin); 		/* READ ROM CMD */
		owCmd(0x4E, p, 0, port, pin); 		/* WRITE SCRATCHPAD */

		owCmd(high_temp_alarm, p, 0, port, pin); 	/* WRITE TH BYTE */
		owCmd(low_temp_alarm, p, 0, port, pin); 	/* WRITE TL BYTE */
		owCmd(bit_resolution, p, 0, port, pin); 	/* WRITE CONFIG BYTE */

		owCmd(0x33, p, 8, port, pin);		/* READ ROM CMD */
		owCmd(0x48, p, 0, port, pin); 		/* COPY SCRATCHPAD */

		__set_PRIMASK(0);

		owIN(port,pin);
		while(owREAD(port,pin) == false); /* wait for end of conv */
	}
	return rv;
}

int owReadTemperature(gpioMap_t gpioPin)
{
	uint8_t port,pin;
	port=gpioPinsInit[gpioPin].gpio.port;
	pin=gpioPinsInit[gpioPin].gpio.pin;

	volatile int rv = -1;
	volatile uint8_t crc = 0;
	volatile uint8_t p[9];

	if(owPresence(port,pin)==0)
	{
		pauses(400);

		__set_PRIMASK(1);

		owCmd(0x33, p, 8, port, pin); /* READ ROM CMD */
		owCmd(0x44, p, 0, port, pin); /* START CONVERSION */

		__set_PRIMASK(0);

		owIN(port,pin);
		while(owREAD(port,pin) == false); /* wait for end of conv */

		owPresence(port,pin);

		pauses(400);

		__set_PRIMASK(1);

		owCmd(0x33, p, 8, port, pin); /* READ ROM CMD */
		owCmd(0xBE, p, 9, port, pin); /* READ SCRATCH */

		__set_PRIMASK(0);

		crc = owCrc(p, 8);

		if(crc == p[8])
		{
			rv = p[1];
			rv <<= 8;
			rv |= p[0];
		}
	}
	return rv;
}
