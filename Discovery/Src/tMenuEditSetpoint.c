///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuEditSetpoint.c
/// \brief
/// \author heinrichs weikamp gmbh
/// \date   19-Dec-2014
///
/// \details
///
/// $Id$
///////////////////////////////////////////////////////////////////////////////
/// \par Copyright (c) 2014-2018 Heinrichs Weikamp gmbh
///
///     This program is free software: you can redistribute it and/or modify
///     it under the terms of the GNU General Public License as published by
///     the Free Software Foundation, either version 3 of the License, or
///     (at your option) any later version.
///
///     This program is distributed in the hope that it will be useful,
///     but WITHOUT ANY WARRANTY; without even the implied warranty of
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///     GNU General Public License for more details.
///
///     You should have received a copy of the GNU General Public License
///     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////

/* Includes ------------------------------------------------------------------*/
#include "tMenuEditSetpoint.h"

#include "check_warning.h"
#include "gfx_fonts.h"
#include "tMenuEdit.h"
#include "unit.h"
#include "tHome.h"

/* Private types -------------------------------------------------------------*/
typedef struct
{
    uint8_t spID;
    SSetpointLine * pSetpointLine;
} SEditSetpointPage;


/* Private variables ---------------------------------------------------------*/
static SEditSetpointPage editSetpointPage;

static uint8_t switchToSetpointCbar;

/* Private function prototypes -----------------------------------------------*/

/* Announced function prototypes -----------------------------------------------*/
static uint8_t OnAction_SP_Setpoint    (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
static uint8_t OnAction_SP_DM_Sensor1	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
static uint8_t OnAction_SP_DM_Sensor2	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);
static uint8_t OnAction_SP_DM_Sensor3	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action);

/* Exported functions --------------------------------------------------------*/

void checkSwitchToLoop(void)
{
    if(!isLoopMode(stateUsedWrite->diveSettings.diveMode)) {
        stateUsedWrite->diveSettings.diveMode = settingsGetPointer()->dive_mode;

        unblock_diluent_page();
    }
}


static uint8_t OnAction_SP_SetpointActive (uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    switch (action) {
    case ACTION_BUTTON_ENTER:

        return digitContent;
    case ACTION_BUTTON_ENTER_FINAL:

        checkAndFixSetpointSettings();

        return UPDATE_AND_EXIT_TO_MENU;
    case ACTION_BUTTON_NEXT:
    case ACTION_BUTTON_BACK:
        editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active = (editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active + 1) % 2;

        tMenuEdit_newInput(editId, editSetpointPage.pSetpointLine[editSetpointPage.spID].note.ub.active, 0, 0, 0);

        return UNSPECIFIC_RETURN;
    default:

        break;
    }

    return EXIT_TO_MENU;
}


int printSetpointName(char *text, uint8_t setpointId, SSettings *settings, bool useSmallFont)
{
    int charsPrinted = 0;
    if (settings->autoSetpoint) {
        switch (setpointId) {
        case SETPOINT_INDEX_AUTO_LOW:
            charsPrinted = snprintf(text, 10, "%s%c%c%s", useSmallFont ? "\016\016" : "", TXT_2BYTE, TXT2BYTE_SetpointLow, useSmallFont ? "\017" : "");

            break;
        case SETPOINT_INDEX_AUTO_HIGH:
            charsPrinted = snprintf(text, 10, "%s%c%c%s", useSmallFont ? "\016\016" : "", TXT_2BYTE, TXT2BYTE_SetpointHigh, useSmallFont ? "\017" : "");

            break;
        case SETPOINT_INDEX_AUTO_DECO:
            charsPrinted = snprintf(text, 10, "%s%c%c%s", useSmallFont ? "\016\016" : "", TXT_2BYTE, TXT2BYTE_SetpointDeco, useSmallFont ? "\017" : "");

            break;
        default:

            break;
        }
    } else {
        charsPrinted = snprintf(text, 10, "%d", setpointId);
    }

    return charsPrinted;
}


void openEdit_Setpoint(uint8_t line)
{
	SSettings *settings = settingsGetPointer();

    uint8_t useSensorSubMenu = 0;
    char text[20];
    uint8_t sensorActive[3];

    /* dive mode */
    if (actual_menu_content != MENU_SURFACE) {
        uint8_t setpointCbar, actualGasID;
        setpointCbar = 100;

        // actualGasID
        if(!isLoopMode(stateUsedWrite->diveSettings.diveMode))
        {
            actualGasID = stateUsedWrite->lifeData.lastDiluent_GasIdInSettings;
            if((actualGasID <= NUM_OFFSET_DILUENT) || (actualGasID > NUM_GASES + NUM_OFFSET_DILUENT))
                actualGasID = NUM_OFFSET_DILUENT + 1;
        }
        else
            actualGasID = stateUsedWrite->lifeData.actualGas.GasIdInSettings;

        // setpointCbar, CCR_Mode and sensor menu
        if (line < 6 && stateUsedWrite->diveSettings.diveMode != DIVEMODE_PSCR) {
            /* setpoints inactive in PSCR mode */

            if (settings->autoSetpoint && line > SETPOINT_INDEX_AUTO_DECO) {
                return;
            }

            setpointCbar = stateUsedWrite->diveSettings.setpoint[line].setpoint_cbar;
            stateUsedWrite->diveSettings.CCR_Mode = CCRMODE_FixedSetpoint;
        } else if (stateUsedWrite->diveSettings.diveMode == DIVEMODE_PSCR && line == 2) {
            /* menu item not pointing to setpoint selection => use sensor or ppo2 simulation */
            stateUsedWrite->diveSettings.CCR_Mode = CCRMODE_Simulation;
		} else if (line == 6) {
            /* => use sensor */
			if(stateUsedWrite->diveSettings.CCR_Mode != CCRMODE_Sensors)
			{
				/* setpoint_cbar will be written by updateSetpointStateUsed() in main.c loop */
				setpointCbar = 255;
				stateUsedWrite->diveSettings.CCR_Mode = CCRMODE_Sensors;
			}
			else
			{
				useSensorSubMenu = 1;
			}
        }

        setActualGas_DM(&stateUsedWrite->lifeData,actualGasID, setpointCbar);

        checkSwitchToLoop();

        clear_warning_fallback();

        if(!useSensorSubMenu)
        {
            exitMenuEdit_to_Home();
        }
        else // entire sub menu during dive to select sensors active
        {
            set_globalState_Menu_Line(line);
            resetMenuEdit(CLUT_MenuPageGasSP);

            text[0] = '\001';
            text[1] = TXT_o2Sensors;
            text[2] = 0;
            write_topline(text);

            if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 1)
            {
            	snprintf (text,20,"Sensor 1");
            	sensorActive[0] = 0;
            }
            else
            {
            	snprintf (text,20,"Sensor 1    (%01.2f)", stateUsed->lifeData.ppO2Sensor_bar[0] );
            	sensorActive[0] = 1;
            }
            write_label_var(  96, 600, ME_Y_LINE1, &FontT48, text);
            if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 2)
            {
               	snprintf (text,20,"Sensor 2");
               	sensorActive[1] = 0;
            }
            else
            {
               	snprintf (text,20,"Sensor 2    (%01.2f)", stateUsed->lifeData.ppO2Sensor_bar[1] );
               	sensorActive[1] = 1;
            }
            write_label_var(  96, 600, ME_Y_LINE2, &FontT48, text);
            if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 4)
            {
               	snprintf (text,20,"Sensor 3");
               	sensorActive[2] = 0;
            }
            else
            {
              	snprintf (text,20,"Sensor 3    (%01.2f)", stateUsed->lifeData.ppO2Sensor_bar[2] );
              	sensorActive[2] = 1;
            }
            write_label_var(  96, 600, ME_Y_LINE3, &FontT48, text);

            write_field_on_off(StMSP_Sensor1,	 30, 95, ME_Y_LINE1,  &FontT48, "", sensorActive[0]);
            write_field_on_off(StMSP_Sensor2,	 30, 95, ME_Y_LINE2,  &FontT48, "", sensorActive[1]);
            write_field_on_off(StMSP_Sensor3,	 30, 95, ME_Y_LINE3,  &FontT48, "", sensorActive[2]);

            setEvent(StMSP_Sensor1, (uint32_t)OnAction_SP_DM_Sensor1);
            setEvent(StMSP_Sensor2, (uint32_t)OnAction_SP_DM_Sensor2);
            setEvent(StMSP_Sensor3, (uint32_t)OnAction_SP_DM_Sensor3);
        }
    } else {
        /* surface mode */
        uint8_t spId, setpoint_cbar, depthDeco, first;
        char text[70];
        uint8_t textPointer;
        uint16_t y_line;

        if ((!settings->autoSetpoint && line <= 5) || line <= SETPOINT_INDEX_AUTO_DECO) {
			set_globalState_Menu_Line(line);

			resetMenuEdit(CLUT_MenuPageGasSP);

			spId = line;
			editSetpointPage.spID = spId;
			editSetpointPage.pSetpointLine = settings->setpoint;

			setpoint_cbar = editSetpointPage.pSetpointLine[spId].setpoint_cbar;
			depthDeco = editSetpointPage.pSetpointLine[spId].depth_meter;
			first = editSetpointPage.pSetpointLine[spId].note.ub.first;

			uint8_t setpointBar = setpoint_cbar / 100;

			textPointer = snprintf(text, 20, "\001%c%c ", TXT_2BYTE, TXT2BYTE_Setpoint);
            textPointer += printSetpointName(&text[textPointer], line, settings, false);
            snprintf(&text[textPointer], 20, " %c", TXT_Setpoint_Edit);
			write_topline(text);

			y_line = ME_Y_LINE_BASE + (line * ME_Y_LINE_STEP);

			textPointer = snprintf(text, 4, "%c%c", TXT_2BYTE, TXT2BYTE_SetpointShort);
            textPointer += printSetpointName(&text[textPointer], line, settings, true);
            textPointer += snprintf(&text[textPointer], 60, "  %s*        \016\016 bar\017", first ? "" : "\177");

            if (settings->autoSetpoint && line == SETPOINT_INDEX_AUTO_DECO) {
                textPointer += snprintf(&text[textPointer], 4, "\n\r");
			    write_label_var(20, 800, y_line, &FontT48, text);

			    write_field_udigit(StMSP_ppo2_setting, 160, 800, y_line, &FontT48, "#.##", (uint32_t)setpointBar, (uint32_t)(setpoint_cbar - (100 * setpointBar)), settings->setpoint[line].note.ub.active, 0);

                snprintf(text, 60, "\034        \035%c%c\n\r", TXT_2BYTE, TXT2BYTE_Enabled);
			    write_label_var(20, 800, y_line + ME_Y_LINE_STEP, &FontT48, text);
                write_field_select(StMSP_Active, 160, 800, y_line + ME_Y_LINE_STEP, &FontT48, "#", settings->setpoint[line].note.ub.active, 0, 0, 0);
            } else {
                textPointer += snprintf(&text[textPointer], 40, "\034   \016\016 \017           \016\016meter\017\035\n\r");
			    write_label_var(20, 800, y_line, &FontT48, text);
			    write_field_udigit(StMSP_ppo2_setting,	160, 800, y_line, &FontT48, "#.##            ###", (uint32_t)setpointBar, (uint32_t)(setpoint_cbar - (100 * setpointBar)), depthDeco, 0);
            }
			setEvent(StMSP_ppo2_setting, (uint32_t)OnAction_SP_Setpoint);
			setEvent(StMSP_Active, (uint32_t)OnAction_SP_SetpointActive);
			startEdit();
		} else if (line == 5) {
            settings->delaySetpointLow = !settings->delaySetpointLow;

            exitMenuEdit_to_Menu_with_Menu_Update_do_not_write_settings_for_this_only();
        } else if (line == 6) {
            settings->autoSetpoint = (settings->autoSetpoint + 1) % 2;

            checkAndFixSetpointSettings();

            exitMenuEdit_to_Menu_with_Menu_Update_do_not_write_settings_for_this_only();
        }
    }
}

static uint8_t OnAction_SP_Setpoint(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
	SSettings *settings = settingsGetPointer();

    int8_t digitContentNew;
    uint32_t new_integer_part, new_fractional_part, new_cbar, newDepth;

    if(action == ACTION_BUTTON_ENTER)
        return digitContent;

    if(action == ACTION_BUTTON_ENTER_FINAL)
    {
        evaluateNewString(editId, &new_integer_part, &new_fractional_part, &newDepth, 0);

        new_cbar = (new_integer_part * 100) + new_fractional_part;

        if(new_cbar < MIN_PPO2_SP_CBAR)
            new_cbar = MIN_PPO2_SP_CBAR;

        if(new_cbar > 160)
            new_cbar = 160;

        new_integer_part = new_cbar / 100;
        new_fractional_part = new_cbar - (new_integer_part * 100);

        editSetpointPage.pSetpointLine[editSetpointPage.spID].setpoint_cbar = new_cbar;

        if (settings->autoSetpoint && editSetpointPage.spID == SETPOINT_INDEX_AUTO_DECO) {
            tMenuEdit_newInput(editId, new_integer_part, new_fractional_part, newDepth, 0);

            checkAndFixSetpointSettings();

            return EXIT_TO_NEXT_MENU;
        } else {
            if (newDepth > 255) {
                newDepth = 255;
            }

            editSetpointPage.pSetpointLine[editSetpointPage.spID].depth_meter = newDepth;
            checkAndFixSetpointSettings();

            return UPDATE_AND_EXIT_TO_MENU;
        }
    }

    if(action == ACTION_BUTTON_NEXT)
    {
        digitContentNew = digitContent + 1;
        if((blockNumber == 0) && (digitContentNew > '1'))
            digitContentNew = '0';
        if(digitContentNew > '9')
            digitContentNew = '0';
        return digitContentNew;
    }

    if(action == ACTION_BUTTON_BACK)
    {
        digitContentNew = digitContent - 1;
        if((blockNumber == 0) && (digitContentNew < '0'))
            digitContentNew = '1';
        if(digitContentNew < '0')
            digitContentNew = '9';
        return digitContentNew;
    }

    return EXIT_TO_MENU;
}

void openEdit_DiveSelectBetterSetpoint(bool useLastDiluent)
{
    uint8_t spId;

    if(stateUsedWrite->diveSettings.diveMode != DIVEMODE_PSCR)		/* no setpoints in PSCR mode */
    {
		spId = actualBetterSetpointId();

        uint8_t gasId;
        if (useLastDiluent) {
            gasId = stateUsed->lifeData.lastDiluent_GasIdInSettings;
        } else {
            gasId = stateUsed->lifeData.actualGas.GasIdInSettings;
        }

		// change in lifeData
		setActualGas_DM(&stateUsedWrite->lifeData, gasId, stateUsedWrite->diveSettings.setpoint[spId].setpoint_cbar);
    }
}


bool findSwitchToSetpoint(void)
{
    uint8_t setpointLowId = getSetpointLowId();
    uint8_t setpointLowCBar = 0;
    if (setpointLowId) {
        setpointLowCBar = stateUsed->diveSettings.setpoint[setpointLowId].setpoint_cbar;
    }

    uint8_t setpointHighId = getSetpointHighId();
    uint8_t setpointHighCBar = 0;
    if (setpointHighId) {
        setpointHighCBar = stateUsed->diveSettings.setpoint[setpointHighId].setpoint_cbar;
    }

    uint8_t setpointDecoId = getSetpointDecoId();
    uint8_t setpointDecoCBar = 0;
    if (setpointDecoId) {
        setpointDecoCBar = stateUsed->diveSettings.setpoint[setpointDecoId].setpoint_cbar;
    }
    uint8_t nextDecoStopDepthM;
    uint16_t nextDecoStopTimeRemainingS;
    const SDecoinfo *decoInfo = getDecoInfo();
    tHome_findNextStop(decoInfo->output_stop_length_seconds, &nextDecoStopDepthM, &nextDecoStopTimeRemainingS);

    uint8_t setpointCurrentCbar = stateUsed->lifeData.actualGas.setPoint_cbar;

    // We cycle SPdeco => SPhigh => SPlow => SPdeco when we have a decompression obligation
    if (setpointDecoCBar && setpointCurrentCbar != setpointDecoCBar && nextDecoStopDepthM && setpointCurrentCbar != setpointHighCBar) {
        switchToSetpointCbar = setpointDecoCBar;
    } else if (setpointLowCBar && setpointCurrentCbar != setpointLowCBar && (!setpointHighCBar || setpointCurrentCbar == setpointHighCBar || stateUsed->lifeData.depth_meter < stateUsed->diveSettings.setpoint[setpointLowId].depth_meter)) {
        switchToSetpointCbar = setpointLowCBar;
    } else if (setpointHighCBar && setpointCurrentCbar != setpointHighCBar) {
        switchToSetpointCbar = setpointHighCBar;
    } else {
        // We don't have a setpoint to switch to
        switchToSetpointCbar = 0;

        return false;
    }

    return true;
}


uint8_t getSwitchToSetpointCbar(void)
{
    return switchToSetpointCbar;
}


void checkSwitchSetpoint(void)
{
    if (switchToSetpointCbar) {
        setActualGas_DM(&stateUsedWrite->lifeData, stateUsed->lifeData.lastDiluent_GasIdInSettings, switchToSetpointCbar);
    }
}


static uint8_t OnAction_SP_DM_Sensor1	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 1)
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated &= 4+2;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated |= 1;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UNSPECIFIC_RETURN;
}

static uint8_t OnAction_SP_DM_Sensor2	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 2)
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated &= 4+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated |= 2;
        tMenuEdit_set_on_off(editId, 0);
    }

    return UNSPECIFIC_RETURN;
}

static uint8_t OnAction_SP_DM_Sensor3	(uint32_t editId, uint8_t blockNumber, uint8_t digitNumber, uint8_t digitContent, uint8_t action)
{
    if(stateUsedWrite->diveSettings.ppo2sensors_deactivated & 4)
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated &= 2+1;
        tMenuEdit_set_on_off(editId, 1);
    }
    else
    {
    	stateUsedWrite->diveSettings.ppo2sensors_deactivated |= 4;
        tMenuEdit_set_on_off(editId, 0);
    }
    return UNSPECIFIC_RETURN;
}
