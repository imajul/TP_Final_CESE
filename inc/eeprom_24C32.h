/*=============================================================================
 * Author: Ignacio Majul <imajul89@gmail.com>
 * Date: 2019/12/10
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _24C32_H_
#define _24C32_H_

/*==================[inclusions]=============================================*/

#include "sapi.h"


/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

// EEPROM24C32 total memory size in bytes
//#define EEPROM_32_K_BIT              4096 // Memory size [bytes]

// EEPROM24C32 single page size (in bytes)
#define EEPROM24C32_PAGE_SIZE       32 // [bytes per page]

// EEPROM24C32 I2C address
#define EEPROM24C32_CTRL_CODE       0x0A // 0b1010

// Function aliases
#define eeprom24C32ReadByte         eeprom24C32ReadRandom

// Function utilities
#define EEPROM_ADDRESS_HIGH(address)   ( (uint8_t)((address&(0xFFFFFF00))>>8) )
#define EEPROM_ADDRESS_LOW(address)    ( (uint8_t)( address&(0x000000FF)) )

// ----------------------------------------------------------------------------

// EEPROM24C32 total memory size in bytes
// #define EEPROM24C32_MEMORY_SIZE   4 KBytes
#define EEPROM24C32_FIRST_MEMORY_ADDRESS 	 (EEPROM24C32_HEADER_POINTER + EEPROM24C32_HEADER_SIZE)
#define EEPROM24C32_LAST_MEMORY_ADDRESS  	 0x0FFF
#define EEPROM24C32_MEMORY_SIZE          	 (EEPROM24C32_LAST_MEMORY_ADDRESS  + 1)
#define EEPROM24C32_HEADER_POINTER			 0x0000  // pointer to first available address
#define EEPROM24C32_HEADER_SIZE			     2  	 // size in bytes

// EEPROM24C32_MEMORY_SIZE / EEPROM24C32_PAGE_SIZE
#define EEPROM24C32_PAGE_AMOUNT           EEPROM24C32_MEMORY_SIZE / EEPROM24C32_PAGE_SIZE

/*==================[typedef]================================================*/

typedef struct
{
   int32_t i2c; // I2C port connected to EEPROM, example I2C0
   // Use this if fixed address
   bool_t A0;          // EEPROM I2C address
   bool_t A1;          // EEPROM I2C address
   bool_t A2;		   // EEPROM I2C address
   // Use this if controlling address from MCU
   int32_t gpioA0;     // GPIO conected to A0 of EEPROM
   int32_t gpioA1;     // GPIO conected to A1 of EEPROM  
   // Use this if controlling EEPROM power from MCU
   int32_t gpioPower;  // GPIO to manage power of EEPROM
   // Use this if controlling WP pin from MCU
   int32_t gpioWP;     // GPIO conected to Write Proyection Pin of EEPROM
   // EEPROM capacity
   int32_t pageSize;   // EEPROM page size [bytes]
   int32_t memorySize; // EEPROM total memory size [bytes]

} Eeprom24C32_t;

/*==================[external functions declaration]=========================*/

//-----------------------------------------------------------------------------
//  RTC and EEPROM MANAGEMENT
//-----------------------------------------------------------------------------

uint8_t eeprom24C32I2cAddress( Eeprom24C32_t* eeprom);

//-----------------------------------------------------------------------------
// INITIALIZATION
//-----------------------------------------------------------------------------

bool_t eeprom24C32Init( Eeprom24C32_t* eeprom, int32_t i2c, bool_t A0, bool_t A1, bool_t A2, int32_t pageSize, int32_t memorySize );

bool_t eeprom24C32Reset( Eeprom24C32_t* eeprom);

uint16_t eeprom24C32GetCurrentAddress( Eeprom24C32_t* eeprom);

bool_t eeprom24C32UpdateDataAddress( Eeprom24C32_t* eeprom, uint16_t address );

//-----------------------------------------------------------------------------
// WRITE OPERATIONS
//-----------------------------------------------------------------------------

// Byte Write
bool_t eeprom24C32WriteByte( Eeprom24C32_t* eeprom, uint32_t memoryAddress, uint8_t byteToWrite );

bool_t eeprom24C32WriteData( Eeprom24C32_t* eeprom24C32, uint8_t *dataToWrite, uint32_t dataSize);

// Page Write
bool_t eeprom24C32WritePage( Eeprom24C32_t* eeprom, uint32_t page, uint8_t* byteBuffer, uint32_t byteBufferSize );

//-----------------------------------------------------------------------------
// READ OPERATIONS
//-----------------------------------------------------------------------------

// Current Address Read
bool_t eeprom24C32ReadCurrentAddress( Eeprom24C32_t* eeprom, uint32_t memoryAddress, uint8_t* readedByte );
                                         
// Random Read
bool_t eeprom24C32ReadRandom( Eeprom24C32_t* eeprom, uint32_t memoryAddress, uint8_t* readedByte );

// Sequential Read
bool_t eeprom24C32ReadSequential( Eeprom24C32_t* eeprom, uint32_t address, uint8_t* byteBuffer, uint32_t byteBufferSize );


/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _24C32_DS3132_H_ */
