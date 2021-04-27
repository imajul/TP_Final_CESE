/*=============================================================================
 * Program: DS18B20
 * Date: 2021/04/18
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef APPLICATION_INC_ONEWIRE_H_
#define APPLICATION_INC_ONEWIRE_H_
/** @brief Brief for this header file.
 **
 ** Full description for this header file.
 **
 **/

/** \addtogroup groupName Group Name
 ** @{ */


/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define HIGH_TEMP_ALARM		0x7F
#define LOW_TEMP_ALARM		0x80
#define RES_9_BIT 			0x1F
#define RES_10_BIT 			0x3F
#define RES_11_BIT 			0x5F
#define RES_12_BIT 			0x7F

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void owInit(gpioMap_t gpioPin);
int owReadTemperature(gpioMap_t gpioPin);
int owSetBitResolution(gpioMap_t gpioPin, uint8_t res);

int owReadScratch(void * buffer9, int port, int pin);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/


#endif /* APPLICATION_INC_ONEWIRE_H_ */
