/* System CLI commands.
 *
 * Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * File: system_vty.h
 *
 * Purpose: To add system CLI configuration and display commands.
 */

#ifndef _SYSTEM_VTY_H
#define _SYSTEM_VTY_H

#ifndef SYS_STR
#define SYS_STR	         "System information\n"
#endif
#define CLOCK_STR  "Shows system clock information\n"
typedef enum
{
        CLI_FAN,
        CLI_PSU,
        CLI_LED,
        CLI_TEMP
}cli_subsystem;

int cli_system_get_all();

void cli_pre_init(void);
void cli_post_init(void);

#endif //_SYSTEM_VTY_H
