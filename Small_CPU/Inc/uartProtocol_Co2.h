/**
  ******************************************************************************
  * @file    uartProtocol_Co2.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    31-Jul-2023
  * @brief	 Interface functionality to handle external, UART based CO2 sensors
  *
  @verbatim
  ==============================================================================
                        ##### How to use #####
  ==============================================================================
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART_PROTOCOL_CO2_H
#define UART_PROTOCOL_CO2_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "configuration.h"
#include "stm32f4xx_hal.h"

 typedef enum
  {
	UART_CO2_INIT = 0,		/* Default Status for every sensor type */
	UART_CO2_IDLE,			/* sensor detected and no communication pending */
	UART_CO2_ERROR,
 	UART_CO2_SETUP = 10,	/* collecting data needed to be read out of the sensor once at startup */
  	UART_CO2_OPERATING,		/* normal operation */
	UART_CO2_CALIBRATE		/* request calibration */
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


void uartCo2_Control(void);
void uartCo2_ProcessData(uint8_t data);
void uartCo2_SendCmd(uint8_t CO2Cmd, uint8_t *cmdString, uint8_t *cmdLength);
uint8_t uartCo2_isSensorConnected();

#endif /* UART_PROTOCOL_CO2_H */
