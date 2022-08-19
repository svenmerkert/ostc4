/**
  ******************************************************************************
  * @file    uart.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    27-March-2014
  * @brief   button control
  *           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART_H
#define UART_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx_hal.h"


 typedef enum
 {
 	RX_Ready= 0,					/* Initial state */
	RX_DetectStart,					/* validate start byte */
	RX_SelectData,					/* Data contained in this frame */
 	RX_Data0,						/* Process incoming data */
	RX_Data1,
	RX_Data2,
	RX_Data3,
	RX_Data4,
	RX_Data5,
	RX_Data6,
	RX_Data7,
	RX_Data8,
	RX_Data9,
	RX_Data10,
	RX_Data11,
	RX_Data12,
	RX_DataComplete
 } receiveState_t;


void MX_USART1_UART_Init(void);
void MX_USART1_UART_DeInit(void);
void MX_USART1_DMA_Init(void);
uint8_t UART_ButtonAdjust(uint8_t *array);
#ifdef ENABLE_CO2_SUPPORT
void HandleUARTCO2Data(void);
#endif
#ifdef ENABLE_SENTINEL_MODE
void HandleUARTSentinelData(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* UART_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
