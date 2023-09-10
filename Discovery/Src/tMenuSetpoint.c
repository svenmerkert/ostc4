///////////////////////////////////////////////////////////////////////////////
/// -*- coding: UTF-8 -*-
///
/// \file   Discovery/Src/tMenuSetpoint.c
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
#include "tMenu.h"
#include "tMenuEditSetpoint.h"
#include "tMenuSetpoint.h"
#include "unit.h"

/* Private function prototypes -----------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

uint32_t tMSP_refresh(char *text, uint16_t *tab, char *subtext)
{
    SSettings *settings = settingsGetPointer();

    const SSetpointLine * pSetpointLine;

    uint8_t textPointer, setpoint_cbar, depthUp, first;

    if(actual_menu_content == MENU_SURFACE)
        pSetpointLine = settings->setpoint;
    else
        pSetpointLine = stateUsed->diveSettings.setpoint;

    textPointer = 0;
    *tab = 130;
    *subtext = 0;

    if((actual_menu_content == MENU_SURFACE) || (stateUsed->diveSettings.diveMode != DIVEMODE_PSCR))	/* do not show setpoints in PSCR mode */
    {
        if (settings->autoSetpoint) {
            disableLine(StMSP_Blank);
        } else {
            enableLine(StMSP_Blank);
        }

		for(int spId=1;spId<=NUM_GASES;spId++)
		{
            if (settings->autoSetpoint) {
                if (spId == 5) {
                    if (actual_menu_content == MENU_SURFACE) {
                        textPointer += snprintf(&text[textPointer], 40, "\020%c%c\016\016%c%c\017 %c%c\002%c\n\r", TXT_2BYTE, TXT2BYTE_SetpointShort, TXT_2BYTE, TXT2BYTE_SetpointLow, TXT_2BYTE, TXT2BYTE_SetpointDelayed, settings->delaySetpointLow ? '\005' : '\006');
                    } else {
                        textPointer += snprintf(&text[textPointer], 40, "\020%c%c %c%c\n\r", TXT_2BYTE, TXT2BYTE_Custom, TXT_2BYTE, TXT2BYTE_Setpoint);
                    }

                    continue;
                } else if (spId > SETPOINT_INDEX_AUTO_DECO) {
                    textPointer += snprintf(&text[textPointer], 3, "\n\r");

                    continue;
                }
            }

			setpoint_cbar = pSetpointLine[spId].setpoint_cbar;
			depthUp = pSetpointLine[spId].depth_meter;
			first = pSetpointLine[spId].note.ub.first;

            char colour;
            if (settings->autoSetpoint && spId == SETPOINT_INDEX_AUTO_DECO  && !pSetpointLine[spId].note.ub.active) {
			    colour = '\031';
            } else {
				colour = '\020';
            }

			uint8_t setpointBar = setpoint_cbar / 100;

            textPointer += snprintf(&text[textPointer], 4, "%c%c%c", colour, TXT_2BYTE, TXT2BYTE_SetpointShort);
            textPointer += printSetpointName(&text[textPointer], spId, settings, true);

            text[textPointer++] = '\t';

			if (first == 0 || actual_menu_content != MENU_SURFACE) {
				strcpy(&text[textPointer++],"\177");
            }

			textPointer += snprintf(&text[textPointer], 40, "* %u.%02u\016\016 bar\017\034   \016\016 \017", setpointBar, setpoint_cbar - (100 * setpointBar));
            if (!settings->autoSetpoint || spId < SETPOINT_INDEX_AUTO_DECO) {
			    if (depthUp) {
				    colour = '\020';
                } else {
			        colour = '\031';
                }

			    textPointer += snprintf(&text[textPointer], 40, "%c%3u\016\016 \035%c%c\017\n\r", colour, unit_depth_integer(depthUp), unit_depth_char1(), unit_depth_char2());
            } else {
			    textPointer += snprintf(&text[textPointer], 14, " %c\016\016 \035%c%c\017\n\r", pSetpointLine[spId].note.ub.active ? '\005' : '\006', TXT_2BYTE, TXT2BYTE_Enabled);
            }
		}
    }
    if (actual_menu_content != MENU_SURFACE) {
        text[textPointer++] = '\020';
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_UseSensor;
        text[textPointer++] = '\n';
        text[textPointer++] = '\r';


        if(stateUsed->diveSettings.diveMode == DIVEMODE_PSCR)
        {
            textPointer += snprintf(&text[textPointer], 20,"\020%c", TXT_SimPpo2);
        }
        text[textPointer++] = 0;
    }
    else
    {
        text[textPointer++] = '\020';
        text[textPointer++] = TXT_2BYTE;
        text[textPointer++] = TXT2BYTE_AutomaticSP;
        text[textPointer++] = '\002';
        if (settings->autoSetpoint)
                    text[textPointer++] = '\005';
                else
                    text[textPointer++] = '\006';
        text[textPointer++] = 0;
    }
    return StMSP;
}
