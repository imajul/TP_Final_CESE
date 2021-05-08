/*=============================================================================
 * Author: Ignacio Majul <imajul89@gmail.com>
 * Date: 2019/12/10
 *===========================================================================*/

/*==================[inclusions]=============================================*/
#include "eeprom_24C32.h"   // <= own header
#include "rtc_DS3231.h"
#include "sapi.h"

#define eeprom24C32DelayMs   delayInaccurateMs

/*==================[internal data definition]===============================*/
DEBUG_PRINT_ENABLE

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

//-----------------------------------------------------------------------------
// MANAGEMENT
//-----------------------------------------------------------------------------

uint8_t eeprom24C32I2cAddress( Eeprom24C32_t* eeprom )
{
	bool_t a0 = eeprom->A0;
	bool_t a1 = eeprom->A1;
	bool_t a2 = eeprom->A2;

	//                0b1010               A2        A1       A0
	return (EEPROM24C32_CTRL_CODE<<3) | (a2<<2) | (a1<<1) | (a0<<0);
}

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------

bool_t eeprom24C32Init( Eeprom24C32_t* eeprom,
		int32_t i2c, bool_t A0, bool_t A1, bool_t A2,
		int32_t pageSize, int32_t memorySize )
{

	bool_t retVal = FALSE;
	uint8_t testByte = 0;

	// I2C port connected to EEPROM, example I2C0
	eeprom->i2c = i2c;
	// Use this if fixed address
	eeprom->A0 = A0;
	eeprom->A1 = A1;
	eeprom->A2 = A2;
	// EEPROM capacity
	eeprom->pageSize = pageSize;   // EEPROM page size [bytes]
	eeprom->memorySize = memorySize; // EEPROM total memory size [bytes]

	return TRUE; //retVal;
}

//-----------------------------------------------------------------------------
// WRITE OPERATIONS
//-----------------------------------------------------------------------------

// Byte Write
bool_t eeprom24C32WriteByte( Eeprom24C32_t* eeprom, int32_t memoryAddress, uint8_t byteToWrite )
{
	bool_t retVal = TRUE; // True if OK

	// Check memory address
	if( memoryAddress > eeprom->memorySize ) {
		return FALSE;
	}

	uint8_t dataToWrite[3];

	// Memory address High
	dataToWrite[0] = EEPROM_ADDRESS_HIGH( memoryAddress );
	// Memory address Low
	dataToWrite[1] = EEPROM_ADDRESS_LOW( memoryAddress );

	// Byte to write
	dataToWrite[2] = (uint8_t)byteToWrite;

	/* uint8_t i2cNumber, uint8_t  i2cSlaveAddress,
      uint8_t* transmitDataBuffer, uint16_t transmitDataBufferSize,
      bool_t sendWriteStop */
	retVal = i2cWrite( eeprom->i2c,
			eeprom24C32I2cAddress( eeprom ),
			dataToWrite, 3, TRUE );

	eeprom24C32DelayMs(5); // Twc - Write cycle time (byte or page)

	return retVal; // Byte writed
}

bool_t eeprom24C32WriteDate( Eeprom24C32_t* eeprom24C32, uint16_t* eeprom_address, rtcDS3231_t time)
{
	eeprom24C32WriteByte( eeprom24C32, *eeprom_address, time.mday);
	(*eeprom_address)++;
	eeprom24C32WriteByte( eeprom24C32, *eeprom_address, time.month);
	(*eeprom_address)++;
	eeprom24C32WriteByte( eeprom24C32, *eeprom_address, time.year);
	(*eeprom_address)++;
	return TRUE;
}

// Page Write
bool_t eeprom24C32WritePage( Eeprom24C32_t* eeprom, uint32_t page,
		uint8_t* byteBuffer, uint32_t byteBufferSize )
{

	bool_t retVal = TRUE; // True if OK

	// Check valid buffer size
	if( byteBufferSize != eeprom->pageSize ) {
		return FALSE;
	}

	uint16_t i=0;

	uint16_t memoryAddress = page * eeprom->pageSize;

	uint8_t dataToWrite[ byteBufferSize+2 ]; // 2 bytes more for memory address

	// Memory address High
	dataToWrite[0] = EEPROM_ADDRESS_HIGH( memoryAddress );
	// Memory address Low
	dataToWrite[1] = EEPROM_ADDRESS_LOW( memoryAddress );

	// Bytes to write
	for( i=0; i<byteBufferSize; i++ ) {
		dataToWrite[i+2] = byteBuffer[i];
	}

	// uint8_t i2cNumber, uint8_t  i2cSlaveAddress,
	// uint8_t* transmitDataBuffer, uint16_t transmitDataBufferSize,
	// bool_t sendWriteStop
	retVal = i2cWrite( eeprom->i2c,
			eeprom24C32I2cAddress( eeprom),
			dataToWrite, (byteBufferSize+2), TRUE );

	eeprom24C32DelayMs(5); // Twc - Write cycle time (byte or page)

	return retVal; // Byte writed
}

//-----------------------------------------------------------------------------
// READ OPERATIONS
//-----------------------------------------------------------------------------

// Current Address Read
bool_t eeprom24C32ReadCurrentAddress( Eeprom24C32_t* eeprom,
		uint32_t memoryAddress,
		uint8_t* readedByte )
{
	bool_t retVal = TRUE; // True if OK

	// uint8_t i2cNumber, uint8_t i2cSlaveAddress,
	// uint8_t* dataToReadBuffer, uint16_t dataToReadBufferSize,
	// bool_t sendWriteStop,
	// uint8_t* reciveDataBuffer, uint16_t reciveDataBufferSize,
	// bool_t sendReadStop
	retVal = i2cRead( eeprom->i2c,
			eeprom24C32I2cAddress( eeprom ),
			(uint8_t*)0, 0,
			FALSE,
			readedByte, 1, TRUE );

	return retVal; // read correct
}

// Random Read
bool_t eeprom24C32ReadRandom( Eeprom24C32_t* eeprom,
		uint32_t memoryAddress, uint8_t* readedByte )
{
	bool_t retVal = TRUE; // True if OK

	// Check memory address
	if( memoryAddress > eeprom->memorySize ) {
		return FALSE;
	}

	uint8_t addressToRead[ 2 ]; // 2 bytes for memory address

	// Memory address High
	addressToRead[0] = EEPROM_ADDRESS_HIGH( memoryAddress );
	// Memory address Low
	addressToRead[1] = EEPROM_ADDRESS_LOW( memoryAddress );

	// uint8_t i2cNumber, uint8_t i2cSlaveAddress,
	// uint8_t* dataToReadBuffer, uint16_t dataToReadBufferSize,
	// bool_t sendWriteStop,
	// uint8_t* reciveDataBuffer, uint16_t reciveDataBufferSize,
	// bool_t sendReadStop
	retVal = i2cRead( eeprom->i2c,
			eeprom24C32I2cAddress( eeprom ),
			addressToRead, 2, FALSE,
			readedByte, 1, TRUE );

	//eeprom24C32DelayMs(1); // ??? - Read cycle time (byte or page)

	return retVal; // read correct
}

// Sequential Read
bool_t eeprom24C32ReadSequential( Eeprom24C32_t* eeprom, uint32_t address,
		uint8_t* byteBuffer, uint32_t byteBufferSize )
{
	bool_t retVal = TRUE; // True if OK

	// Check valid buffer size
	if( byteBufferSize > eeprom->memorySize  ) {
		return FALSE;
	}

	// TODO: Check valid address
	//if( address >= eeprom24C32AmountOfPagesGet(eeprom) ) {
	//   return FALSE;
	//}

	uint8_t addressToRead[ 2 ]; // 2 bytes for memory address

	// Memory address High
	addressToRead[0] = EEPROM_ADDRESS_HIGH( address );
	// Memory address Low
	addressToRead[1] = EEPROM_ADDRESS_LOW( address );

	// uint8_t i2cNumber, uint8_t i2cSlaveAddress,
	// uint8_t* dataToReadBuffer, uint16_t dataToReadBufferSize,
	// bool_t sendWriteStop,
	// uint8_t* reciveDataBuffer, uint16_t reciveDataBufferSize,
	// bool_t sendReadStop
	retVal = i2cRead( eeprom->i2c,
			eeprom24C32I2cAddress( eeprom),
			addressToRead, 2, FALSE,
			byteBuffer, byteBufferSize, TRUE );

	return retVal; // Byte writed
}

/*==================[ISR external functions definition]======================*/

/*==================[end of file]============================================*/
