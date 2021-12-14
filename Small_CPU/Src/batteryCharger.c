/**
  ******************************************************************************
  * @file    batteryCharger.c 
  * @author  heinrichs weikamp gmbh
  * @date    09-Dec-2014
  * @version V0.0.1
  * @since   09-Dec-2014
  * @brief   LTC4054 Battery Charger
  *           
  @verbatim                 
  ============================================================================== 
                        ##### How to use #####
  ============================================================================== 

The bq5105x provides one status output, CHG. This output is an open-drain NMOS device that is rated to 20 V.
The open-drain FET connected to the CHG pin will be turned on whenever the output (BAT) of the charger is
enabled. As a note, the output of the charger supply will not be enabled if the VRECT-REG does not converge to the
no-load target voltage.

CHG F4 7 O Open-drain output � active when BAT is enabled. Float if not used.

@endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 heinrichs weikamp</center></h2>
  *
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "batteryCharger.h"
#include "batteryGasGauge.h"
#include "stm32f4xx_hal.h"
#include "scheduler.h"


/* Use This compile switch to select the new charger status control implementation */
#define ENABLE_CHARGER_STATUS_V2

#define CHARGER_DEBOUNCE_SECONDS	(6u)		/* 6 seconds used to avoid problems with charger interrupts / disconnections */

uint8_t battery_i_charge_status = 0;
uint16_t battery_charger_counter = 0;

#ifdef ENABLE_CHARGER_STATUS_V2
static chargerState_t batteryChargerState = Charger_NotConnected;
#endif

/* can be 0, 1 or 255
 * 0 is disconnected
 * 1 is charging
 * 255 is full
 */
uint8_t get_charge_status(void)
{
	return battery_i_charge_status;
}

void set_charge_state(uint8_t newState)
{
#ifdef ENABLE_CHARGER_STATUS_V2
	if(newState < Charger_END)
	{
		batteryChargerState = newState;
	}
#endif
}

uint8_t get_charge_state(void)
{
	return batteryChargerState;
}

void init_battery_charger_status(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif

  CHARGE_IN_GPIO_ENABLE();
  CHARGE_OUT_GPIO_ENABLE();
	
	ReInit_battery_charger_status_pins();
}

void ReInit_battery_charger_status_pins(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif

  GPIO_InitTypeDef   GPIO_InitStructure;

  GPIO_InitStructure.Pin = CHARGE_IN_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(CHARGE_IN_GPIO_PORT, &GPIO_InitStructure); 

  GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure); 
}


void DeInit_battery_charger_status_pins(void)
{
	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif
  GPIO_InitTypeDef   GPIO_InitStructure;


	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  GPIO_InitStructure.Pin = CHARGE_IN_PIN;
  HAL_GPIO_Init(CHARGE_IN_GPIO_PORT, &GPIO_InitStructure); 

  GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
  HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure); 
}

/* static counter is used to avoid multiple counts of charge startings
	 and after that it is used, starting at 127 to count for the charge full signal

	there a short disconnections with the QI charger
	therefore the battery_charger_counter has a countdown instead of = 0.

	battery_gas_gauge_set_charge_full and  scheduleUpdateDeviceDataChargerFull are
	set after disconnection as the charging process continues as long as not disconnected
  to prevent the short disconnections the battery_charger_counter is used too including
	upcounting again while battery_i_charge_status == 255 and the connection is established

 */

void battery_charger_get_status_and_contral_battery_gas_gauge(uint8_t cycleTimeBase)
{
#ifdef ENABLE_CHARGER_STATUS_V2
	static uint8_t notifyChargeComplete = 0;
	static float chargeValueAtStart = 0;
#endif 

	#ifdef OSTC_ON_DISCOVERY_HARDWARE
		return;
	#endif
	
#ifdef ENABLE_CHARGER_STATUS_V2

	if(batteryChargerState == Charger_ColdStart)	/* wait for the first valid voltage meassurement */
	{
		if(global.lifeData.battery_voltage != BATTERY_DEFAULT_VOLTAGE)
		{
			if(global.lifeData.battery_voltage > BATTERY_ENDOF_CHARGE_VOLTAGE) 						/* Voltage close to full state => maybe new battery inserted 	*/
			{
				battery_gas_gauge_set_charge_full();
			}
			else										/* unknown state => reset to 0% */
			{
				battery_gas_gauge_set(0);
			}
			batteryChargerState = Charger_NotConnected;
		}
	}
	else
	{	/* on disconnection or while disconnected */
		if(HAL_GPIO_ReadPin(CHARGE_IN_GPIO_PORT,CHARGE_IN_PIN))
		{
			switch(batteryChargerState)
			{
				case Charger_Active:				global.dataSendToMaster.chargeStatus = CHARGER_lostConnection;
													global.deviceDataSendToMaster.chargeStatus = CHARGER_lostConnection;
													batteryChargerState = Charger_LostConnection;
													battery_charger_counter = CHARGER_DEBOUNCE_SECONDS;
											break;
				case Charger_LostConnection:		if(get_voltage() >= BATTERY_ENDOF_CHARGE_VOLTAGE)	 	/* the charger stops charging when charge current is 1/10 	*/
																											/* Basically it is OK to rate a charging as complete if a defined voltage is reached */
													{
														batteryChargerState = Charger_Finished;
														global.dataSendToMaster.chargeStatus = CHARGER_complete;
														global.deviceDataSendToMaster.chargeStatus = CHARGER_complete;
														notifyChargeComplete = 1;
													}
				/* no break */
				case Charger_WarmUp:
				case Charger_Finished:				if(battery_charger_counter >= cycleTimeBase)
													{
														battery_charger_counter -= cycleTimeBase;
														global.dataSendToMaster.chargeStatus = CHARGER_lostConnection;
														global.deviceDataSendToMaster.chargeStatus = CHARGER_lostConnection;
														batteryChargerState = Charger_LostConnection;
													}
													else
													{
														battery_charger_counter = 0;
														battery_i_charge_status = 0;
														global.dataSendToMaster.chargeStatus = CHARGER_off;
														global.deviceDataSendToMaster.chargeStatus = CHARGER_off;

														if(notifyChargeComplete)
														{
															battery_gas_gauge_set_charge_full();
															scheduleUpdateDeviceDataChargerFull();
															notifyChargeComplete = 0;
															if(cycleTimeBase > 2)
															{
																HAL_Delay(50);		/* I2C operations are pending in the background. Wait to avoid data loose in caused to potential change to sleep state */
															}
														}
														else
														{
															if(chargeValueAtStart < 1.0) /* charging started with unknown value => reset charge state reported by charger */
															{
																battery_gas_gauge_set(0);
															}
														}
														batteryChargerState = Charger_NotConnected;
													}
											break;
				default: break;
			}
		}
		else
		{
			/* connected */
			/* wait for disconnection to write and reset */
			switch(batteryChargerState)
			{
					case Charger_NotConnected:		battery_i_charge_status = 1;
													battery_charger_counter = 0;
													batteryChargerState = Charger_WarmUp;
													chargeValueAtStart = global.lifeData.battery_charge;
											break;
					case Charger_LostConnection:		batteryChargerState = Charger_Active;
											break;
					case Charger_WarmUp:			battery_charger_counter += cycleTimeBase;
													if(battery_charger_counter >= CHARGER_DEBOUNCE_SECONDS )
													{
														battery_i_charge_status = 2;
														scheduleUpdateDeviceDataChargerCharging();
														batteryChargerState = Charger_Active;
													}
							/* no break */
					case Charger_Finished:
					case Charger_Active:			global.dataSendToMaster.chargeStatus = CHARGER_running;
													global.deviceDataSendToMaster.chargeStatus = CHARGER_running;

													/* drive the output pin high to determine the state of the charger */
													GPIO_InitTypeDef   GPIO_InitStructure;
													GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
													GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
													GPIO_InitStructure.Pull = GPIO_NOPULL;
													GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
													HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure);
													HAL_GPIO_WritePin(CHARGE_OUT_GPIO_PORT, CHARGE_OUT_PIN,GPIO_PIN_SET);
													HAL_Delay(1);

													if(HAL_GPIO_ReadPin(CHARGE_IN_GPIO_PORT,CHARGE_IN_PIN))		/* high => charger stopped charging */
													{
														batteryChargerState = Charger_Finished;
														global.dataSendToMaster.chargeStatus = CHARGER_complete;
														global.deviceDataSendToMaster.chargeStatus = CHARGER_complete;
														battery_charger_counter = 30;
														notifyChargeComplete = 1;
													}
													else
													{
														if(batteryChargerState == Charger_Finished)				/* voltage dropped below the hysteresis again => charging restarted */
														{
															batteryChargerState = Charger_Active;
															notifyChargeComplete = 0;
														}
													}

													/* restore high impedance to be able to detect disconnection */
													GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
													GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
													GPIO_InitStructure.Pull = GPIO_NOPULL;
													GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
													HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure);
													HAL_Delay(1);
											break;

					default:						/* wait for disconnection */
						break;
			}
		}
	}
#else
	/* on disconnection or while disconnected */
	if(HAL_GPIO_ReadPin(CHARGE_IN_GPIO_PORT,CHARGE_IN_PIN))
	{
		if(battery_charger_counter)
		{
			battery_charger_counter--;
			global.dataSendToMaster.chargeStatus = CHARGER_lostConnection;
			global.deviceDataSendToMaster.chargeStatus = CHARGER_lostConnection;
		}
		/* max count down to 127+5 or 127+20 */
		if((battery_i_charge_status == 255) && battery_charger_counter < 127)
		{
//			battery_gas_gauge_set_charge_full();
//			scheduleUpdateDeviceDataChargerFull();
			battery_charger_counter = 0;
		}
		
		if(battery_charger_counter == 0)
		{
			battery_i_charge_status = 0;
			global.dataSendToMaster.chargeStatus = CHARGER_off;
			global.deviceDataSendToMaster.chargeStatus = CHARGER_off;

		}
		return;
	}

	/* connected */
	
	/* wait for disconnection to write and reset */
	if(battery_i_charge_status == 255)
	{
		global.dataSendToMaster.chargeStatus = CHARGER_complete;
		global.deviceDataSendToMaster.chargeStatus = CHARGER_complete;
		
		if(((cycleTimeBase > 1) && (battery_charger_counter < 127+5)) || (battery_charger_counter < 127+20))
		battery_charger_counter++;
		return;
	}

	if(battery_charger_counter == 0)
		battery_i_charge_status = 1;

	/* charger is connected and didn't signal full yet */
	global.dataSendToMaster.chargeStatus = CHARGER_running;
	global.deviceDataSendToMaster.chargeStatus = CHARGER_running;

	GPIO_InitTypeDef   GPIO_InitStructure;
    GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure); 
	HAL_GPIO_WritePin(CHARGE_OUT_GPIO_PORT, CHARGE_OUT_PIN,GPIO_PIN_SET);
	HAL_Delay(1);

	
	if(battery_charger_counter < 120)
	{
		if(cycleTimeBase == 1)
			battery_charger_counter++;
		else
		{
			battery_charger_counter += 30;
			if(battery_charger_counter >= 127)
				battery_charger_counter = 126;
		}
	}
	else
	if(battery_charger_counter < 127)
	{
		battery_charger_counter = 127;
		if(battery_i_charge_status < 2)
		{
			battery_i_charge_status = 2;
			scheduleUpdateDeviceDataChargerCharging();
		}
	}

	if(battery_charger_counter >= 127)
	{
		if(HAL_GPIO_ReadPin(CHARGE_IN_GPIO_PORT,CHARGE_IN_PIN) || (get_voltage() >= 4.1f))
		{
			battery_charger_counter++;
			if(((cycleTimeBase > 1) && (battery_charger_counter > 127+5)) || (battery_charger_counter > 127+20))
			{
				battery_charger_counter = 127;
				if(get_voltage() >= 4.1f)
				{
					battery_i_charge_status = 255;
					battery_gas_gauge_set_charge_full();
					scheduleUpdateDeviceDataChargerFull();
				}					
			}
		}
		else
			battery_charger_counter = 127;
	}

  GPIO_InitStructure.Pin = CHARGE_OUT_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(CHARGE_OUT_GPIO_PORT, &GPIO_InitStructure); 
#endif
}

/************************ (C) COPYRIGHT heinrichs weikamp *****END OF FILE****/
