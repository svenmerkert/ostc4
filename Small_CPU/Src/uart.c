/**
  ******************************************************************************
  * @file    uart.c 
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    27-March-2014
  * @brief   button control
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 
  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "uartProtocol_O2.h"
#include "externalInterface.h"
#include "data_exchange.h"
#include <string.h>	/* memset */

/* Private variables ---------------------------------------------------------*/



#define CHUNK_SIZE				(25u)		/* the DMA will handle chunk size transfers */
#define CHUNKS_PER_BUFFER		(5u)

UART_HandleTypeDef huart1;

DMA_HandleTypeDef  hdma_usart1_rx;

uint8_t rxBuffer[CHUNK_SIZE * CHUNKS_PER_BUFFER];		/* The complete buffer has a X * chunk size to allow fariations in buffer read time */
static uint8_t rxWriteIndex;							/* Index of the data item which is analysed */
static uint8_t rxReadIndex;								/* Index at which new data is stared */
static uint8_t lastCmdIndex;							/* Index of last command which has not been completly received */
static uint8_t dmaActive;								/* Indicator if DMA reception needs to be started */

static uint8_t CO2Connected = 0;						/* Binary indicator if a sensor is connected or not */
static uint8_t SentinelConnected = 0;					/* Binary indicator if a sensor is connected or not */



static uartCO2Status_t ComStatus_CO2 = UART_CO2_INIT;

float LED_Level = 0.0;							/* Normalized LED value which may be used as indication for the health status of the sensor */
float LED_ZeroOffset = 0.0;
float pCO2 = 0.0;
/* Exported functions --------------------------------------------------------*/



//huart.Instance->BRR = UART_BRR_SAMPLING8(HAL_RCC_GetPCLK2Freq(), new_baudrate);

void MX_USART1_UART_Init(void)
{
/* regular init */	

  huart1.Instance = USART1;

  if(externalInterface_GetUARTProtocol() == 0x04)
  {
	  huart1.Init.BaudRate = 19200;
  }
  else
  {
	  huart1.Init.BaudRate = 9600;
	  ComStatus_CO2 = UART_CO2_INIT;
  }
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart1);

  MX_USART1_DMA_Init();

  memset(rxBuffer,BUFFER_NODATA,sizeof(rxBuffer));
  rxReadIndex = 0;
  lastCmdIndex = 0;
  rxWriteIndex = 0;
  dmaActive = 0;

  CO2Connected = 0;
  SentinelConnected = 0;

}

void MX_USART1_UART_DeInit(void)
{
	HAL_DMA_Abort(&hdma_usart1_rx);
	HAL_DMA_DeInit(&hdma_usart1_rx);
	HAL_UART_DeInit(&huart1);
}

void  MX_USART1_DMA_Init()
{
  /* DMA controller clock enable */
  __DMA2_CLK_ENABLE();

  /* Peripheral DMA init*/
  hdma_usart1_rx.Instance = DMA2_Stream5;
  hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
  hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY; //DMA_MEMORY_TO_PERIPH;
  hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_rx.Init.PeriphDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_rx.Init.Mode = DMA_NORMAL;
  hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
  hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  HAL_DMA_Init(&hdma_usart1_rx);

  __HAL_LINKDMA(&huart1,hdmarx,hdma_usart1_rx);

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}

void  UART_MUX_SelectAddress(uint8_t muxAddress)
{
	uint8_t indexstr[4];

	if(muxAddress <= MAX_MUX_CHANNEL)
	{
		indexstr[0] = '~';
		indexstr[1] = muxAddress;
		indexstr[2] = 0x0D;
		indexstr[3] = 0x0A;

		HAL_UART_Transmit(&huart1,indexstr,4,10);
	}
}


void UART_SendCmdString(uint8_t *cmdString)
{
	uint8_t cmdLength = strlen((char*)cmdString);

	if(cmdLength < 20)		/* A longer string is an indication for a missing 0 termination */
	{
		if(dmaActive == 0)
		{
			UART_StartDMA_Receiption();
		}
		HAL_UART_Transmit(&huart1,cmdString,cmdLength,10);
	}
}

void DigitalCO2_SendCmd(uint8_t CO2Cmd, uint8_t *cmdString, uint16_t *cmdLength)
{
	switch (CO2Cmd)
	{
		case CO2CMD_MODE_POLL:		*cmdLength = snprintf((char*)cmdString, 10, "K 2\r\n");
				break;
		case CO2CMD_MODE_STREAM:	*cmdLength = snprintf((char*)cmdString, 10, "K 1\r\n");
				break;
		case CO2CMD_CALIBRATE:		*cmdLength = snprintf((char*)cmdString, 10, "G\r\n");
				break;
		case CO2CMD_GETDATA:		*cmdLength = snprintf((char*)cmdString, 10, "Q\r\n");
				break;
		case CO2CMD_GETSCALE:		*cmdLength = snprintf((char*)cmdString, 10, ".\r\n");
				break;
		default: *cmdLength = 0;
			break;
	}
	if(cmdLength != 0)
	{
		HAL_UART_Transmit(&huart1,cmdString,*cmdLength,10);
	}
}

void StringToInt(char *pstr, uint32_t *puInt32)
{
	uint8_t index = 0;
	uint32_t result = 0;
	while((pstr[index] >= '0') && (pstr[index] <= '9'))
	{
		result *=10;
		result += pstr[index] - '0';
		index++;
	}
	*puInt32 = result;
}

void StringToUInt64(char *pstr, uint64_t *puint64)
{
	uint8_t index = 0;
	uint64_t result = 0;
	while((pstr[index] >= '0') && (pstr[index] <= '9'))
	{
		result *=10;
		result += pstr[index] - '0';
		index++;
	}
	*puint64 = result;
}
void ConvertByteToHexString(uint8_t byte, char* str)
{
	uint8_t worker = 0;
	uint8_t digit = 0;
	uint8_t digitCnt = 1;

	worker = byte;
	while((worker!=0) && (digitCnt != 255))
	{
		digit = worker % 16;
		if( digit < 10)
		{
			digit += '0';
		}
		else
		{
			digit += 'A' - 10;
		}
		str[digitCnt--]= digit;
		worker = worker / 16;
	}
}

void UART_StartDMA_Receiption()
{
	if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
	{
		dmaActive = 1;
	}
}

#ifdef ENABLE_CO2_SUPPORT
void UART_HandleCO2Data(void)
{
	uint8_t localRX = rxReadIndex;
	static uint8_t dataType = 0;
	static uint32_t dataValue = 0;
	static receiveState_t rxState = RX_Ready;
	static uint32_t lastReceiveTick = 0;
	static uint32_t lastTransmitTick = 0;
	static uint8_t cmdString[10];
	static uint16_t cmdLength = 0;

	uint32_t Tick = HAL_GetTick();

	uint8_t *pmap = externalInterface_GetSensorMapPointer(0);

	if(ComStatus_CO2 == UART_CO2_INIT)
	{
		UART_StartDMA_Receiption();
		ComStatus_CO2 = UART_CO2_SETUP;
	}

	if(ComStatus_CO2 == UART_CO2_SETUP)
	{
		if(time_elapsed_ms(lastTransmitTick,Tick) > 200)
		{
			if(externalInterface_GetCO2Scale() == 0.0)
			{
				DigitalCO2_SendCmd(CO2CMD_GETDATA, cmdString, &cmdLength);
				lastTransmitTick = Tick;
			}
			else
			{
				ComStatus_CO2 = UART_CO2_OPERATING;
			}
		}
	}
	else
	{
		if(pmap[EXT_INTERFACE_SENSOR_CNT-1] == SENSOR_MUX)		/* sensor is working in polling mode if mux is connected to avoid interference with other sensors */
		{
			if(time_elapsed_ms(lastTransmitTick,Tick) > 2000)	/* poll every two seconds */
			{
				lastTransmitTick = Tick;
				if(cmdLength == 0)							/* poll data */
				{
					DigitalCO2_SendCmd(CO2CMD_GETDATA, cmdString, &cmdLength);
				}
				else											/* resend last command */
				{
					HAL_UART_Transmit(&huart1,cmdString,strlen((char*)cmdString),10);
					cmdLength = 0;
				}
			}
		}
	}
	while((rxBuffer[localRX]!=BUFFER_NODATA))
	{
		lastReceiveTick = Tick;
		if(rxState == RX_Ready)		/* identify data content */
		{
			switch(rxBuffer[localRX])
			{
				case 'l':
				case 'D':
				case 'Z':
				case '.':
									dataType = rxBuffer[localRX];
									rxState = RX_Data0;
									dataValue = 0;
					break;

				default:			/* unknown or corrupted => ignore */
					break;
			}
		}
		else if((rxBuffer[localRX] >= '0') && (rxBuffer[localRX] <= '9'))
		{
			if((rxState >= RX_Data0) && (rxState <= RX_Data4))
			{
				dataValue = dataValue * 10 + (rxBuffer[localRX] - '0');
				rxState++;
				if(rxState == RX_Data5)
				{
					rxState = RX_DataComplete;
					CO2Connected = 1;
				}
			}
			else	/* protocol error data has max 5 digits */
			{
				rxState = RX_Ready;
			}
		}
		if((rxBuffer[localRX] == ' ') || (rxBuffer[localRX] == '\n'))	/* Abort data detection */
		{
			if(rxState == RX_DataComplete)
			{
				if(externalInterface_GetCO2State() == 0)
				{
					externalInterface_SetCO2State(EXT_INTERFACE_33V_ON);
				}
				switch(dataType)
				{
					case 'D':			externalInterface_SetCO2SignalStrength(dataValue);
						break;
					case 'l':			LED_ZeroOffset = dataValue;
						break;
					case 'Z':			externalInterface_SetCO2Value(dataValue);
						break;
					case '.':			externalInterface_SetCO2Scale(dataValue);
						break;
					default:			rxState = RX_Ready;
						break;
				}
			}
			if(rxState != RX_Data0)	/* reset state machine because message in wrong format */
			{
				rxState = RX_Ready;
			}
		}
		rxBuffer[localRX] = BUFFER_NODATA;
		localRX++;
		rxReadIndex++;
		if(rxReadIndex >= CHUNK_SIZE * CHUNKS_PER_BUFFER)
		{
			localRX = 0;
			rxReadIndex = 0;
		}
	}

	if(time_elapsed_ms(lastReceiveTick,HAL_GetTick()) > 2000)	/* check for communication timeout */
	{
		externalInterface_SetCO2State(0);
		CO2Connected = 0;
	}

	if((dmaActive == 0)	&& (externalInterface_isEnabledPower33()))	/* Should never happen in normal operation => restart in case of communication error */
	{
		UART_StartDMA_Receiption();
	}
}
#endif

#ifdef ENABLE_SENTINEL_MODE
void UART_HandleSentinelData(void)
{
	uint8_t localRX = rxReadIndex;
	static uint8_t dataType = 0;
	static uint32_t dataValue[3];
	static uint8_t dataValueIdx = 0;
	static receiveState_t rxState = RX_Ready;
	static uint32_t lastReceiveTick = 0;
	static uint8_t lastAlive = 0;
	static uint8_t curAlive = 0;
	static uint8_t checksum = 0;
	static char checksum_str[]="00";

	while((rxBuffer[localRX]!=0))
	{
		lastReceiveTick = HAL_GetTick();

		switch(rxState)
		{
			case RX_Ready:	if((rxBuffer[localRX] >= 'a') && (rxBuffer[localRX] <= 'z'))
							{
								rxState = RX_DetectStart;
								curAlive = rxBuffer[localRX];
								checksum = 0;
							}
					break;

			case RX_DetectStart: 	checksum += rxBuffer[localRX];
									if(rxBuffer[localRX] == '1')
								 	{
								 		rxState = RX_SelectData;
								 		dataType = 0xFF;

								 	}
									else
									{
										rxState = RX_Ready;
									}
					break;

			case RX_SelectData:		checksum += rxBuffer[localRX];
									switch(rxBuffer[localRX])
									{
										case 'T':	dataType = rxBuffer[localRX];
											break;
										case '0': 	if(dataType != 0xff)
													{
														rxState = RX_Data0;
														dataValueIdx = 0;
														dataValue[0] = 0;

													}
													else
													{
														rxState = RX_Ready;
													}
											break;
										default:	rxState = RX_Ready;
									}
					break;

			case RX_Data0:
			case RX_Data1:
			case RX_Data2:
			case RX_Data4:
			case RX_Data5:
			case RX_Data6:
			case RX_Data8:
			case RX_Data9:
			case RX_Data10: checksum += rxBuffer[localRX];
							if((rxBuffer[localRX] >= '0') && (rxBuffer[localRX] <= '9'))
							{
								dataValue[dataValueIdx] = dataValue[dataValueIdx] * 10 + (rxBuffer[localRX] - '0');
								rxState++;
							}
							else
							{
								rxState = RX_Ready;
							}
					break;

			case RX_Data3:
			case RX_Data7:	checksum += rxBuffer[localRX];
							if(rxBuffer[localRX] == '0')
							{
								rxState++;
								dataValueIdx++;
								dataValue[dataValueIdx] = 0;
							}
							else
							{
								rxState = RX_Ready;
							}
					break;
			case RX_Data11: rxState = RX_DataComplete;
							ConvertByteToHexString(checksum,checksum_str);
							if(rxBuffer[localRX] == checksum_str[0])
							{
								rxState = RX_DataComplete;
							}
							else
							{
								rxState = RX_Ready;
							}

				break;

			case RX_DataComplete:	if(rxBuffer[localRX] == checksum_str[1])
									{
										setExternalInterfaceChannel(0,(float)(dataValue[0] / 10.0));
										setExternalInterfaceChannel(1,(float)(dataValue[1] / 10.0));
										setExternalInterfaceChannel(2,(float)(dataValue[2] / 10.0));
										SentinelConnected = 1;
									}
									rxState = RX_Ready;
				break;


			default:				rxState = RX_Ready;
				break;

		}
		localRX++;
		rxReadIndex++;
		if(rxReadIndex >= CHUNK_SIZE * CHUNKS_PER_BUFFER)
		{
			localRX = 0;
			rxReadIndex = 0;
		}
	}

	if(time_elapsed_ms(lastReceiveTick,HAL_GetTick()) > 4000)	/* check for communication timeout */
	{
		if(curAlive == lastAlive)
		{
			setExternalInterfaceChannel(0,0.0);
			setExternalInterfaceChannel(1,0.0);
			setExternalInterfaceChannel(2,0.0);
			SentinelConnected = 0;
		}
		lastAlive = curAlive;
	}

	if((dmaActive == 0)	&& (externalInterface_isEnabledPower33()))	/* Should never happen in normal operation => restart in case of communication error */
	{
		UART_StartDMA_Receiption();
	}
}
#endif


uint8_t UART_isCO2Connected()
{
	return CO2Connected;
}
uint8_t UART_isSentinelConnected()
{
	return SentinelConnected;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart1)
    {
    	dmaActive = 0;
    	rxWriteIndex+=CHUNK_SIZE;
    	if(rxWriteIndex >= CHUNK_SIZE * CHUNKS_PER_BUFFER)
    	{
    		rxWriteIndex = 0;
    	}
    	if((rxWriteIndex / CHUNK_SIZE) != (rxReadIndex / CHUNK_SIZE) || (rxWriteIndex == rxReadIndex))	/* start next transfer if we did not catch up with read index */
    	{
    		if(externalInterface_GetUARTProtocol() != 0)
    		{
				UART_StartDMA_Receiption();
    		}
    	}
    }
}

void UART_ReadData(uint8_t sensorType)
{
	uint8_t localRX = rxReadIndex;

	while((rxBuffer[localRX]!=BUFFER_NODATA))
	{
		switch (sensorType)
		{
			case SENSOR_MUX:
			case SENSOR_DIGO2:	uartO2_ProcessData(rxBuffer[localRX]);
				break;
	//	case SENSOR_CO2:	uartCO2_Control();
				break;
			default:
				break;
		}

		rxBuffer[localRX] = BUFFER_NODATA;
		localRX++;
		rxReadIndex++;
		if(rxReadIndex >= CHUNK_SIZE * CHUNKS_PER_BUFFER)
		{
			localRX = 0;
			rxReadIndex = 0;
		}
	}
}

void UART_FlushRxBuffer(void)
{
	while(rxBuffer[rxReadIndex] != BUFFER_NODATA)
	{
		rxBuffer[rxReadIndex] = BUFFER_NODATA;
		rxReadIndex++;
		if(rxReadIndex >= CHUNK_SIZE * CHUNKS_PER_BUFFER)
		{
			rxReadIndex = 0;
		}
	}

}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
