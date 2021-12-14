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

float LED_Level = 0.0;							/* Normalized LED value which may be used as indication for the health status of the sensor */
float LED_ZeroOffset = 0.0;
float pCO2 = 0.0;
/* Exported functions --------------------------------------------------------*/

void MX_USART1_UART_Init(void)
{
/* regular init */	

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart1);

  rxReadIndex = 0;
  lastCmdIndex = 0;
  rxWriteIndex = 0;
  dmaActive = 0;
}

void MX_USART1_UART_DeInit(void)
{
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


uint32_t dataValue = 0;

void HandleUARTData(void)
{
	uint8_t localRX = rxReadIndex;
	uint8_t dataType = 0;
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
				if(HAL_OK == HAL_UART_Receive_DMA (&huart1, &rxBuffer[rxWriteIndex], CHUNK_SIZE))
				{
					dmaActive = 1;
				}
    		}
    	}
    }
}








/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
