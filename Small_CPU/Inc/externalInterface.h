/**
  ******************************************************************************
  * @file    externalInterface.h
  * @author  heinrichs weikamp gmbh
  * @version V0.0.1
  * @date    07-Nov-2020
  * @brief	 Interface functionality to proceed external analog signal via i2c connection
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
#ifndef EXTERNAL_INTERFACE_H
#define EXTERNAL_INTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#define MAX_ADC_CHANNEL		(3u)		/* number of channels to be read */
#define EXTERNAL_ADC_NO_DATA	0xFF

#define EXT33V_CONTROL_PIN				GPIO_PIN_7	/* PortC */

void externalInterface_Init(void);
void externalInterface_InitPower33(void);
uint8_t externalInterface_StartConversion(uint8_t channel);
uint8_t externalInterface_ReadAndSwitch();
float externalInterface_CalculateADCValue(uint8_t channel);
float getExternalInterfaceChannel(uint8_t channel);
uint8_t setExternalInterfaceChannel(uint8_t channel, float value);
void externalInterface_SwitchPower33(uint8_t state);
void externalInterface_SwitchADC(uint8_t state);
uint8_t externalInterface_isEnabledPower33(void);
uint8_t externalInterface_isEnabledADC(void);

void externalInterface_SetCO2Value(uint16_t CO2_ppm);
void externalInterface_SetCO2SignalStrength(uint16_t LED_qa);
uint16_t externalInterface_GetCO2Value(void);
uint16_t externalInterface_GetCO2SignalStrength(void);
void externalInterface_SetCO2State(uint16_t state);
uint16_t externalInterface_GetCO2State(void);

void externalInterface_ExecuteCmd(uint16_t Cmd);

#endif /* EXTERNAL_INTERFACE_H */
