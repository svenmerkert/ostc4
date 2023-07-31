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


#define BUFFER_NODATA			(7u)		/* The read function needs a byte which indecated that no data for processing is available.*/
											/* This byte shall never appear in a normal data steam */

 typedef enum
 {
 	UART_CO2_INIT = 0,
	UART_CO2_SETUP,			/* collecting data needed to be read out of the sensor once at startup */
 	UART_CO2_OPERATING,		/* normal operation */
 } uartCO2Status_t;

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


typedef enum
{
	CO2CMD_MODE_POLL,		/* Set operation mode of sensor to polling => only send data if requested */
	CO2CMD_MODE_STREAM,		/* Set operation mode of sensor to streaming => send data every two seconds */
	CO2CMD_CALIBRATE,		/* Calibrate sensor */
	CO2CMD_GETSCALE,		/* Get scaling factor */
	CO2CMD_GETDATA			/* Read sensor data */
} co2SensorCmd_t;

void MX_USART1_UART_Init(void);
void MX_USART1_UART_DeInit(void);
void MX_USART1_DMA_Init(void);
uint8_t UART_ButtonAdjust(uint8_t *array);
void UART_StartDMA_Receiption(void);
#ifdef ENABLE_CO2_SUPPORT
void UART_HandleCO2Data(void);
void DigitalCO2_SendCmd(uint8_t CO2Cmd, uint8_t *cmdString, uint8_t *cmdLength);
#endif
#ifdef ENABLE_SENTINEL_MODE
void UART_HandleSentinelData(void);
#endif
uint8_t UART_isCO2Connected();
uint8_t UART_isSentinelConnected();
void UART_setTargetChannel(uint8_t channel);
void  UART_MUX_SelectAddress(uint8_t muxAddress);
void UART_SendCmdString(uint8_t *cmdString);
void UART_ReadData(uint8_t sensorType);
void UART_FlushRxBuffer(void);


void StringToInt(char *pstr, uint32_t *puInt32);
void StringToUInt64(char *pstr, uint64_t *puint64);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
