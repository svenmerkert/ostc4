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
#include "externalInterface.h"
#include "data_exchange.h"
#include <string.h>	/* memset */

/* Private variables ---------------------------------------------------------*/

#define CHUNK_SIZE			(20u)		/* the DMA will handle chunk size transfers */
#define CHUNKS_PER_BUFFER	(3u)
UART_HandleTypeDef huart1;

DMA_HandleTypeDef  hdma_usart1_rx;

uint8_t rxBuffer[CHUNK_SIZE * CHUNKS_PER_BUFFER];		/* The complete buffer has a X * chunk size to allow fariations in buffer read time */
static uint8_t rxWriteIndex;					/* Index of the data item which is analysed */
static uint8_t rxReadIndex;						/* Index at which new data is stared */
static uint8_t lastCmdIndex;					/* Index of last command which has not been completly received */
static uint8_t dmaActive;						/* Indicator if DMA receiption needs to be started */

static SSensorDataDiveO2 sensorDataDiveO2;		/* intermediate storage for additional sensor data */

char tmpRxBuf[30];
uint8_t tmpRxIdx = 0;

static uartO2Status_t Comstatus_O2 = UART_O2_INIT;

float LED_Level = 0.0;							/* Normalized LED value which may be used as indication for the health status of the sensor */
float LED_ZeroOffset = 0.0;
float pCO2 = 0.0;
/* Exported functions --------------------------------------------------------*/

void MX_USART1_UART_Init(void)
{
/* regular init */	

  huart1.Instance = USART1;

  if(externalInterface_GetUARTProtocol() == 0x04)
  {
	  huart1.Init.BaudRate = 19200;
	  Comstatus_O2 = UART_O2_INIT;
  }
  else
  {
	  huart1.Init.BaudRate = 9600;
  }
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart1);

  MX_USART1_DMA_Init();

  rxReadIndex = 0;
  lastCmdIndex = 0;
  rxWriteIndex = 0;
  dmaActive = 0;
  Comstatus_O2 = UART_O2_INIT;
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


#ifdef ENABLE_CO2_SUPPORT
void HandleUARTCO2Data(void)
{
	uint8_t localRX = rxReadIndex;
	uint8_t dataType = 0;
	uint32_t dataValue = 0;
	static receiveState_t rxState = RX_Ready;
	static uint32_t lastReceiveTick = 0;

	while(localRX != rxWriteIndex)
	{
		lastReceiveTick = HAL_GetTick();
		if(rxState == RX_Ready)		/* identify data content */
		{
			switch(rxBuffer[localRX])
			{
				case 'l':
				case 'D':
				case 'Z':
									dataType = rxBuffer[localRX];
									rxState = RX_Data0;
									dataValue = 0;
					break;

				default:			/* unknown or corrupted => ignore */
					break;
			}
		}
		else if((rxState >= RX_Data0) && (rxState <= RX_Data4))
		{
			if((rxBuffer[localRX] >= '0') && (rxBuffer[localRX] <= '9'))
			{
				dataValue = dataValue * 10 + (rxBuffer[localRX] - '0');
				rxState++;
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
					default: break;
				}
			}
			if(rxState != RX_Data0)	/* reset state machine because message in wrong format */
			{
				rxState = RX_Ready;
			}
		}

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
	}

	if((dmaActive == 0)	&& (externalInterface_isEnabledPower33()))	/* Should never happen in normal operation => restart in case of communication error */
	{
		if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
		{
			dmaActive = 1;
		}
	}
}
#endif

#ifdef ENABLE_SENTINEL_MODE
void HandleUARTSentinelData(void)
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
	char checksum_str[]="00";

	while(localRX != rxWriteIndex)
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
		}
		lastAlive = curAlive;
	}

	if((dmaActive == 0)	&& (externalInterface_isEnabledPower33()))	/* Should never happen in normal operation => restart in case of communication error */
	{
		if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
		{
			dmaActive = 1;
		}
	}
}
#endif

void DigitalO2_SetupCmd(uint8_t O2State, uint8_t *cmdString, uint8_t *cmdLength)
{
	switch (O2State)
	{
		case UART_O2_CHECK:		*cmdLength = snprintf((char*)cmdString, 10, "#LOGO");
			break;
		case UART_O2_REQ_INFO: 	*cmdLength = snprintf((char*)cmdString, 10, "#VERS");
					break;
		case UART_O2_REQ_ID: 	*cmdLength = snprintf((char*)cmdString, 10, "#IDNR");
			break;
		case UART_O2_REQ_O2: 	*cmdLength = snprintf((char*)cmdString, 10, "#DOXY");
			break;

		default: *cmdLength = 0;
			break;
	}
	if(*cmdLength != 0)
	{
		cmdString[*cmdLength] = 0x0D;
		*cmdLength = *cmdLength + 1;
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

void HandleUARTDigitalO2(void)
{
	static uint32_t lastO2ReqTick = 0;

	static uartO2RxState_t rxState = O2RX_IDLE;
	static uint32_t lastReceiveTick = 0;
	static uint8_t lastAlive = 0;
	static uint8_t curAlive = 0;

	static uint8_t cmdLength = 0;
	static uint8_t cmdString[10];
	static uint8_t cmdReadIndex = 0;

	uint32_t tmpO2 = 0;
	uint32_t tmpData = 0;
	uint8_t localRX = rxReadIndex;
	uint32_t tick =  HAL_GetTick();


	if(Comstatus_O2 == UART_O2_INIT)
	{
		memset((char*)&rxBuffer[rxWriteIndex],(int)0,CHUNK_SIZE);
		memset((char*) &sensorDataDiveO2, 0, sizeof(sensorDataDiveO2));
		externalInterface_SetSensorData(0,(uint8_t*)&sensorDataDiveO2);

		lastAlive = 0;
		curAlive = 0;

		Comstatus_O2 = UART_O2_CHECK;
		DigitalO2_SetupCmd(Comstatus_O2,cmdString,&cmdLength);
		HAL_UART_Transmit(&huart1,cmdString,cmdLength,10);

		rxState = O2RX_CONFIRM;
		cmdReadIndex = 0;
		lastO2ReqTick = tick;

		if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
		{
			dmaActive = 1;
		}
	}
	if(time_elapsed_ms(lastO2ReqTick,tick) > 1000)		/* repeat request once per second */
	{
		lastO2ReqTick = tick;
		if(Comstatus_O2 == UART_O2_IDLE)				/* cyclic request of o2 value */
		{
			Comstatus_O2 = UART_O2_REQ_O2;
			rxState = O2RX_CONFIRM;
		}
		DigitalO2_SetupCmd(Comstatus_O2,cmdString,&cmdLength);

		HAL_UART_Transmit(&huart1,cmdString,cmdLength,10);
	}

	while((rxBuffer[localRX]!=0))
	{

		lastReceiveTick = tick;
		switch(rxState)
		{
			case O2RX_CONFIRM:	if(rxBuffer[localRX] == '#')
								{
									cmdReadIndex = 0;
								}
								if(rxBuffer[localRX] == cmdString[cmdReadIndex])
							    {
								cmdReadIndex++;
								if(cmdReadIndex == cmdLength - 1)
								{
									tmpRxIdx = 0;
									memset((char*) tmpRxBuf, 0, sizeof(tmpRxBuf));
									switch (Comstatus_O2)
									{
											case UART_O2_CHECK:	Comstatus_O2 = UART_O2_REQ_ID;
																rxState = O2RX_CONFIRM;
																DigitalO2_SetupCmd(Comstatus_O2,cmdString,&cmdLength);
																HAL_UART_Transmit(&huart1,cmdString,cmdLength,10);
												break;
											case UART_O2_REQ_ID: rxState = O2RX_GETNR;
												break;
											case UART_O2_REQ_INFO: rxState = O2RX_GETTYPE;
												break;
											case UART_O2_REQ_O2:	rxState = O2RX_GETO2;
												break;
											default:	Comstatus_O2 = UART_O2_IDLE;
														rxState = O2RX_IDLE;
													break;
									}
								}
						  	}
				break;

			case O2RX_GETSTATUS:
			case O2RX_GETTEMP:
			case O2RX_GETTYPE:
			case O2RX_GETVERSION:
			case O2RX_GETCHANNEL:
			case O2RX_GETSUBSENSORS:
			case O2RX_GETO2:
			case O2RX_GETNR:	if(rxBuffer[localRX] != 0x0D)
								{
									if(rxBuffer[localRX] != ' ')
									{
										tmpRxBuf[tmpRxIdx++] = rxBuffer[localRX];
									}
									else
									{
										if(tmpRxIdx != 0)
										{
											switch(rxState)
											{
												case O2RX_GETCHANNEL:	StringToInt(tmpRxBuf,&tmpData);
																		rxState = O2RX_GETVERSION;
														break;
												case O2RX_GETVERSION:	StringToInt(tmpRxBuf,&tmpData);
																		rxState = O2RX_GETSUBSENSORS;
														break;
												case O2RX_GETTYPE: 		StringToInt(tmpRxBuf,&tmpData);
																		rxState = O2RX_GETCHANNEL;
														break;

												case O2RX_GETO2: 		StringToInt(tmpRxBuf,&tmpO2);
																		setExternalInterfaceChannel(0,(float)(tmpO2 / 10000.0));
																		rxState = O2RX_GETTEMP;
													break;
												case O2RX_GETTEMP:		StringToInt(tmpRxBuf,(uint32_t*)&sensorDataDiveO2.temperature);
																		rxState = O2RX_GETSTATUS;
													break;
												default:
													break;
											}
											memset((char*) tmpRxBuf, 0, tmpRxIdx);
											tmpRxIdx = 0;
										}
									}
								}
								else
								{
									switch (rxState)
									{
										case O2RX_GETSTATUS:		StringToInt(tmpRxBuf,&sensorDataDiveO2.status);
																	externalInterface_SetSensorData(1,(uint8_t*)&sensorDataDiveO2);
																	Comstatus_O2 = UART_O2_IDLE;
																	rxState = O2RX_IDLE;
												break;
										case O2RX_GETSUBSENSORS:	StringToInt(tmpRxBuf,&tmpData);
																	Comstatus_O2 = UART_O2_IDLE;
																	rxState = O2RX_IDLE;
												break;
										case  O2RX_GETNR: 			StringToUInt64((char*)tmpRxBuf,&sensorDataDiveO2.sensorId);
											/* no break */
										default:		Comstatus_O2 = UART_O2_IDLE;
														rxState = O2RX_IDLE;
											break;
									}
								}
					break;
			default:				rxState = O2RX_IDLE;
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
		}
		lastAlive = curAlive;
	}

	if((dmaActive == 0)	&& (externalInterface_isEnabledPower33()))	/* Should never happen in normal operation => restart in case of communication error */
	{
		if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
		{
			dmaActive = 1;
		}
	}
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
    	if((rxWriteIndex / CHUNK_SIZE) != (rxReadIndex / CHUNK_SIZE))	/* start next transfer if we did not catch up with read index */
    	{
    		if(externalInterface_isEnabledPower33())
    		{
    			memset((char*)&rxBuffer[rxWriteIndex],(int)0,CHUNK_SIZE);
				if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
				{
					dmaActive = 1;
				}
    		}
    	}
    }
}








/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
