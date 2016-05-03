/* System CLI commands
 *
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 * Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * File: system_vty.c
 *
 * Purpose:  To add system CLI configuration and display commands.
 */

#include "vtysh/command.h"
#include "vtysh/vtysh.h"
#include "vtysh/vtysh_user.h"
#include "system_vty.h"
#include "vswitch-idl.h"
#include "ovsdb-idl.h"
#include "smap.h"
#include "vtysh/memory.h"
#include "openvswitch/vlog.h"
#include "openswitch-idl.h"
#include "vtysh/vtysh_ovsdb_if.h"
#include "vtysh/vtysh_ovsdb_config.h"
#include "vtysh/utils/system_vtysh_utils.h"

VLOG_DEFINE_THIS_MODULE(vtysh_system_cli);

extern struct ovsdb_idl *idl;

/*
 * Function        : compare_fan
 * Resposibility     : Fan sort function for qsort
 * Parameters
 *  a   : Pointer to 1st element in the array
 *  b   : Pointer to next element in the array
 * Return      : comparative difference between names.
 */
static inline int
compare_fan (const void* a,const void* b)
{
    struct ovsrec_fan* s1 = (struct ovsrec_fan*)a;
    struct ovsrec_fan* s2 = (struct ovsrec_fan*)b;

    return (strcmp(s1->name,s2->name));
}

/*
 * Function        : compare_psu
 * Resposibility    : Power Supply sort function for qsort
 * Parameters
 *   a   : Pointer to 1st element in the array
 *   b   : Pointer to next element in the array
 * Return      : comparative difference between names.
 */
static inline int
compare_psu(const void* a,const void* b)
{
    struct ovsrec_power_supply* s1 = (struct ovsrec_power_supply*)a;
    struct ovsrec_power_supply* s2 = (struct ovsrec_power_supply*)b;

    return (strcmp(s1->name,s2->name));
}

/*
 * Function        : format_psu_string
 * Resposibility     : Change status string in OVSDB to more
 *        readable string
 * Parameters
 *      status  : Pointer to status string
 * Return      : Pointer to formatted status string
 */
static const char*
format_psu_string (char* status)
{
    if (!status)
        return NULL;

    if (0 == strcmp(status,OVSREC_POWER_SUPPLY_STATUS_FAULT_ABSENT))
        return POWER_SUPPLY_FAULT_ABSENT;
    else if (0 == strcmp(status,OVSREC_POWER_SUPPLY_STATUS_FAULT_INPUT))
        return POWER_SUPPLY_FAULT_INPUT;
    else if (0 == strcmp(status,OVSREC_POWER_SUPPLY_STATUS_FAULT_OUTPUT))
        return POWER_SUPPLY_FAULT_OUTPUT;
    else if (0 == strcmp(status,OVSREC_POWER_SUPPLY_STATUS_OK))
        return POWER_SUPPLY_OK;
    else if (0 == strcmp(status,OVSREC_POWER_SUPPLY_STATUS_UNKNOWN))
        return POWER_SUPPLY_UNKNOWN;

    return status;
}

/*
 * Function        : format_sys_output
 * Resposibility     : Format and Print output for system info
 * Parameters
 *      vty : Pointer to vty structure
 *  pSys    : Pointer to ovsrec_subsystem structure
 *  pVswitch: Pointer to ovsrec_system structure
 */
static void
format_sys_output (struct vty* vty,
                const struct ovsrec_subsystem* pSys,
                const struct ovsrec_system* pVswitch)
{
    const char* buf = NULL;
    vty_out(vty, "%-20s%s%-30s%s", "OpenSwitch Version", ": ",
                 (pVswitch->switch_version) ? pVswitch->switch_version : " ",
                 VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"Product Name");
    (buf) ? vty_out(vty,"%-20s%s%-30s%s%s",
            "Product Name",": ",buf,VTY_NEWLINE,VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-30s%s%s",
                "Product Name",": "," ",VTY_NEWLINE,VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"vendor");
    (buf) ? vty_out(vty,"%-20s%s%-30s%s","Vendor",": ", buf, VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-30s%s","Vendor",": "," ", VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"platform_name");
    (buf) ? vty_out(vty,"%-20s%s%-30s%s","Platform",": ", buf, VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-30s%s","Platform",": "," ", VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"manufacturer");
    (buf) ? vty_out(vty,"%-20s%s%-20s%s","Manufacturer",": ",buf,VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-20s%s","Manufacturer",": "," ", VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"manufacture_date");
    (buf) ? vty_out(vty,"%-20s%s%-20s%s%s",
            "Manufacturer Date",": ", buf, VTY_NEWLINE, VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-20s%s%s",
                "Manufacturer Date",": "," ", VTY_NEWLINE, VTY_NEWLINE);


    buf = smap_get (&pSys->other_info,"serial_number");
    (buf) ? vty_out(vty,"%-20s%s%-20s","Serial Number",": ", buf):\
        vty_out(vty,"%-20s%s%-20s","Serial Number",": "," ");

    buf = smap_get (&pSys->other_info,"label_revision");
    (buf) ? vty_out(vty,"%-20s%s%-10s%s%s",
            "Label Revision",": ", buf, VTY_NEWLINE,VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-10s%s%s",
                "Label Revision",": "," ", VTY_NEWLINE,VTY_NEWLINE);


    buf = smap_get (&pSys->other_info,"onie_version");
    (buf) ? vty_out(vty,"%-20s%s%-20s","ONIE Version",": ", buf):\
        vty_out(vty,"%-20s%s%-20s","ONIE Version",": "," ");

    buf = smap_get (&pSys->other_info,"diag_version");
    (buf) ? vty_out(vty,"%-20s%s%-10s%s",
            "DIAG Version",": ", buf, VTY_NEWLINE):\
        vty_out(vty,"%-20s%s%-10s%s","DIAG Version",": "," ", VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"base_mac_address");
    (buf) ? vty_out(vty, "%-20s%s%-20s","Base MAC Address",": ", buf):\
        vty_out(vty,"%-20s%s%-20s","Base MAC Address",": "," ");

    buf = smap_get (&pSys->other_info,"number_of_macs");
    (buf) ? vty_out(vty, "%-20s%s%-5s%s",
            "Number of MACs",": ", buf, VTY_NEWLINE):\
        vty_out(vty, "%-20s%s%-5s%s","Number of MACs",": "," ", VTY_NEWLINE);

    buf = smap_get (&pSys->other_info,"interface_count");
    (buf) ? vty_out(vty, "%-20s%s%-20s","Interface Count",": ", buf):\
        vty_out(vty, "%-20s%s%-20s","Interface Count",": "," ");

    buf = smap_get (&pSys->other_info,"max_interface_speed");
    (buf) ? vty_out(vty, "%-20s%s%-6sMbps%s",
            "Max Interface Speed",": ", buf, VTY_NEWLINE):\
        vty_out(vty, "%-20s%s%-6sMbps%s",
                "Max Interface Speed",": "," ", VTY_NEWLINE);
}


/*
 * Function        : cli_system_get_all
 * Resposibility     : Get System overview information from OVSDB
 * Return      : 0 on success 1 otherwise
 */
int
cli_system_get_all()
{
    const struct ovsrec_subsystem* pSys = NULL;
    const struct ovsrec_system* pVswitch = NULL;
    const struct ovsrec_fan* pFan = NULL;
    struct ovsrec_fan* pFanSort = NULL;
    const struct ovsrec_led* pLed = NULL;
    const struct ovsrec_power_supply* pPSU = NULL;
    struct ovsrec_power_supply* pPSUsort = NULL;
    const struct ovsrec_temp_sensor* pTempSen = NULL;
    int n = 0, i = 0;

    pSys = ovsrec_subsystem_first(idl);
    pVswitch = ovsrec_system_first(idl);

    if (pSys && pVswitch) {
        format_sys_output(vty, pSys,pVswitch);
    }
    else {
        VLOG_ERR("Unable to retrieve data\n");
    }


    vty_out(vty,"%sFan details:%s%s",VTY_NEWLINE,VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty,"%-15s%-10s%-10s%s","Name","Speed","Status",VTY_NEWLINE);
    vty_out(vty,"%s%s","--------------------------------",VTY_NEWLINE);
    n = pSys->n_fans;
    if (0 != n)
    {
        pFanSort = (struct ovsrec_fan*)calloc (n,sizeof(struct ovsrec_fan));

        OVSREC_FAN_FOR_EACH (pFan,idl)
        {
            memcpy (pFanSort+i,pFan,sizeof(struct ovsrec_fan));
            i++;
        }

        qsort((void*)pFanSort,n,sizeof(struct ovsrec_fan),compare_fan);

        for (i = 0; i < n ; i++)
        {
            vty_out(vty,"%-15s",(pFanSort+i)->name);
            vty_out(vty,"%-10s",(pFanSort+i)->speed);
            vty_out(vty,"%-10s",(pFanSort+i)->status);
            vty_out(vty,"%s",VTY_NEWLINE);
        }
    }

    if (pFanSort)
    {
        free(pFanSort);
        pFanSort = NULL;
    }

    vty_out(vty,"%sLED details:%s%s",VTY_NEWLINE,VTY_NEWLINE, VTY_NEWLINE);
    vty_out(vty,"%-10s%-10s%-8s%s","Name","State","Status",VTY_NEWLINE);
    vty_out(vty,"%s%s","-------------------------",VTY_NEWLINE);

    n = pSys->n_leds;
    if (0 != n)
    {

        OVSREC_LED_FOR_EACH (pLed,idl)
        {
            vty_out(vty,"%-10s",pLed->id);
            vty_out(vty,"%-10s",pLed->state);
            vty_out(vty,"%-8s",pLed->status);
            vty_out(vty,"%s",VTY_NEWLINE);
        }
    }

    vty_out(vty,"%sPower supply details:%s%s",VTY_NEWLINE,VTY_NEWLINE,
            VTY_NEWLINE);
    vty_out(vty,"%-10s%-10s%s","Name","Status",VTY_NEWLINE);
    vty_out(vty,"%s%s","-----------------------",VTY_NEWLINE);
    n = pSys->n_power_supplies;

    if (n > 0)
    {
        pPSUsort = (struct ovsrec_power_supply*)calloc(n,
					sizeof(struct ovsrec_power_supply));

        i = 0;
        OVSREC_POWER_SUPPLY_FOR_EACH (pPSU,idl)
        {
            if (pPSU)
            {
                memcpy(pPSUsort+i,pPSU,sizeof(struct ovsrec_power_supply));
                i++;
            }
	}

        qsort((void*)pPSUsort,n,sizeof(struct ovsrec_power_supply),compare_psu);

        for (i = 0; i < n ; i++)
        {
            vty_out(vty,"%-15s",(pPSUsort+i)->name);
            vty_out(vty,"%-10s",format_psu_string((pPSUsort+i)->status));
            vty_out(vty,"%s",VTY_NEWLINE);
        }

        if(pPSUsort)
	{
            free(pPSUsort);
            pPSUsort = NULL;
	}
    }

    vty_out(vty,"%sTemperature Sensors:%s%s",VTY_NEWLINE,VTY_NEWLINE,
            VTY_NEWLINE);
    n = pSys->n_temp_sensors;
    if ( 0 !=  n)
    {

        vty_out(vty,"%-50s%-10s%-18s%s","Location","Name",
                "Reading(celsius)",VTY_NEWLINE);
        vty_out(vty,"%s%s",
                "---------------------------------------------------------------------------",VTY_NEWLINE);
        OVSREC_TEMP_SENSOR_FOR_EACH (pTempSen,idl)
        {
            vty_out(vty,"%-50s",pTempSen->location);
            vty_out(vty,"%-10s",pTempSen->name);
            vty_out(vty,"%3.2f",(double)((pTempSen->temperature)/1000));
            vty_out(vty,"%s",VTY_NEWLINE);
        }
    }
    else
    {
        vty_out(vty,"%-10s%-10s%-18s%s","Location","Name",
                "Reading(celsius)",VTY_NEWLINE);
        vty_out(vty,"%s%s","------------------------------------",VTY_NEWLINE);
    }

    return CMD_SUCCESS;
}


DEFUN (cli_platform_show_system,
        cli_platform_show_system_cmd,
        "show system",
        SHOW_STR
        SYS_STR)
{
    return cli_system_get_all();
}

/*******************************************************************
 * @func        : system_ovsdb_init
 * @detail      : Add system related table & columns to ops-cli
 *                idl cache
 *******************************************************************/
static void
system_ovsdb_init()
{
    /* Add Platform Related Tables. */
    ovsdb_idl_add_table(idl, &ovsrec_table_fan);
    ovsdb_idl_add_table(idl, &ovsrec_table_led);
    ovsdb_idl_add_table(idl, &ovsrec_table_subsystem);

    /* Add Columns for System Related Tables. */

    /* LED. */
    ovsdb_idl_add_column(idl, &ovsrec_led_col_id);
    ovsdb_idl_add_column(idl, &ovsrec_led_col_state);
    ovsdb_idl_add_column(idl, &ovsrec_led_col_status);
    ovsdb_idl_add_column(idl, &ovsrec_led_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_led_col_external_ids);

    /* Subsystem .*/
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_interfaces);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_leds);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_fans);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_asset_tag_number);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_type);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_hw_desc_dir);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_other_info);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_subsystem_col_external_ids);

    /* Fan. */
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_status);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_direction);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_rpm);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_other_config);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_hw_config);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_external_ids);
    ovsdb_idl_add_column(idl, &ovsrec_fan_col_speed);

}


/* Initialize ops-sysd cli nodd.
 */
void
cli_pre_init(void)
{
   system_ovsdb_init();
}

/* Initialize ops-sysd cli element.
 */
void
cli_post_init(void)
{
    install_element (ENABLE_NODE, &cli_platform_show_system_cmd);
    install_element (VIEW_NODE, &cli_platform_show_system_cmd);
}
