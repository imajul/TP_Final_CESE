/*=============================================================================
 * Program: ACS712
 * Date: 2021/02/06
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "ACS712.h"
#include "sapi.h"

char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/*=====[Definition macros of private constants]==============================*/

/*=====[Definitions of extern global variables]==============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
   // ----- Setup -----------------------------------
   boardInit();

   /* Inicializar UART_USB a 115200 baudios */
   uartConfig( UART_USB, 115200 );

   /* Inicializar AnalogIO */
   /* Posibles configuraciones:
    *    ADC_ENABLE,  ADC_DISABLE,
    *    ADC_ENABLE,  ADC_DISABLE,
    */
   adcConfig( ADC_ENABLE ); /* ADC */
   bool_t ledState1 = OFF;
   uint32_t i = 0;

      /* Buffer */
      static char uartBuff[10];

      /* Variable para almacenar el valor leido del ADC CH1 */
      uint16_t muestra = 0, muestra2 = 0;

      /* Variables de delays no bloqueantes */
      delay_t delay1;
      delay_t delay2;

      /* Inicializar Retardo no bloqueante con tiempo en ms */
      delayConfig( &delay1, 5000 );
      delayConfig( &delay2, 200 );

      /*---------- REPETIR POR SIEMPRE ------------- */
      while(1) {

         /* delayRead retorna TRUE cuando se cumple el tiempo de retardo */
         if ( delayRead( &delay1 ) ){

            /* Leo la Entrada Analogica AI0 - ADC0 CH1 */
            muestra = adcRead( CH1 );
            muestra2 = adcRead( CH2 );

            /* Envío la primer parte del mnesaje a la Uart */
            uartWriteString( UART_USB, "ADC CH1 value: " );

            /* Conversión de muestra entera a ascii con base decimal */
            itoa( muestra, uartBuff, 10 ); /* 10 significa decimal */

            /* Enviar muestra y Enter */
            uartWriteString( UART_USB, uartBuff );
            uartWriteString( UART_USB, ";\r\n" );

            uartWriteString( UART_USB, "ADC CH2 value: " );

                        /* Conversión de muestra entera a ascii con base decimal */
                        itoa( muestra2, uartBuff, 10 ); /* 10 significa decimal */

                        /* Enviar muestra y Enter */
                        uartWriteString( UART_USB, uartBuff );
                        uartWriteString( UART_USB, ";\r\n" );

            /* Escribo la muestra en la Salida AnalogicaAO - DAC */
            // dacWrite( DAC, muestra );
         }

         /* delayRead retorna TRUE cuando se cumple el tiempo de retardo */
//         if ( delayRead( &delay2 ) ){
//            ledState1 = !ledState1;
//            gpioWrite( LED1, ledState1 );
//
//            /* Si pasaron 20 delays le aumento el tiempo */
//            i++;
//            if( i == 20 )
//               delayWrite( &delay2, 1000 );
//         }

      }


   // YOU NEVER REACH HERE, because this program runs directly or on a
   // microcontroller and is not called by any Operating System, as in the 
   // case of a PC program.
   return 0;
}
