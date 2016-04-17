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

VLOG_DEFINE_THIS_MODULE(vtysh_system_cli);

extern struct ovsdb_idl *idl;

/* Show version detail help strings */
#define SHOW_VERSION_STR           "Displays switch version\n"
#define SHOW_VERSION_DETAIL_STR    "Show package information\n"

const char *psu_state_string[] = {
    "Absent",
    "Input Fault",
    "Output Fault",
    "OK",
    "Unknown"
};

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

    if (0 == strcmp (status,OVSREC_POWER_SUPPLY_STATUS_FAULT_ABSENT))
        return psu_state_string[POWER_SUPPLY_STATUS_FAULT_ABSENT];
    else if (0 == strcmp (status,OVSREC_POWER_SUPPLY_STATUS_FAULT_INPUT))
        return psu_state_string[POWER_SUPPLY_STATUS_FAULT_INPUT];
    else if (0 == strcmp (status,OVSREC_POWER_SUPPLY_STATUS_FAULT_OUTPUT))
        return psu_state_string[POWER_SUPPLY_STATUS_FAULT_OUTPUT];

    return NULL;
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
    if (0 != n)
    {
        OVSREC_POWER_SUPPLY_FOR_EACH (pPSU,idl)
        {
            vty_out(vty,"%-10s",pPSU->name);
            vty_out(vty,"%-10s",format_psu_string(pPSU->status));
            vty_out(vty,"%s",VTY_NEWLINE);
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

/*
 * The get command to read from the ovsdb system table
 * switch_version column.
 */
const char *
vtysh_ovsdb_switch_version_get(void)
{
    const struct ovsrec_system *ovs;

    ovs = ovsrec_system_first(idl);
    if (ovs == NULL) {
        VLOG_ERR("unable to retrieve any system table rows");
        return "";
    }

    return ovs->switch_version ? ovs->switch_version : "";
}

/*
 * The get command to read from the ovsdb system table
 * software_info:os_name value.
 */
const char *
vtysh_ovsdb_os_name_get(void)
{
    const struct ovsrec_system *ovs;
    const char *os_name = NULL;

    ovs = ovsrec_system_first(idl);
    if (ovs) {
        os_name = smap_get(&ovs->software_info, SYSTEM_SOFTWARE_INFO_OS_NAME);
    }

    return os_name ? os_name : "OpenSwitch";
}

/* Show version detail */
void
vtysh_ovsdb_show_version_detail(void)
{
    const struct ovsrec_package_info *row = NULL;

    OVSREC_PACKAGE_INFO_FOR_EACH(row, idl) {
        if (row) {
            vty_out(vty, "PACKAGE     : %-128s\n",  row->name);
            vty_out(vty, "VERSION     : %-128s\n",
                (row->version[0] == '\0') ? "Not Available" : row->version);
            vty_out(vty, "SOURCE TYPE : %-128s\n",
                (row->src_type[0] == '\0') ? "Not Available" : row->src_type);
            vty_out(vty, "SOURCE URL  : %-128s\n\n",
                (row->src_url[0] == '\0') ? "Not Available" : row->src_url);
        }
    }
}


DEFUN (cli_platform_show_system,
        cli_platform_show_system_cmd,
        "show system",
        SHOW_STR
        SYS_STR)
{
    return cli_system_get_all();
}

/* Show version. */
#ifndef ENABLE_OVSDB
DEFUN (show_version,
       show_version_cmd,
       "show version",
       SHOW_STR
       "Displays zebra version\n")
{
    vty_out (vty, "%s %s (%s).%s", QUAGGA_PROGNAME, QUAGGA_VERSION,
             host.name?host.name:"", VTY_NEWLINE);
    vty_out (vty, "%s%s", GIT_INFO, VTY_NEWLINE);

    return CMD_SUCCESS;
}
#else
DEFUN (show_version,
       show_version_cmd,
       "show version",
       SHOW_STR
       SHOW_VERSION_STR)
{
    vty_out (vty, "%s %s%s", vtysh_ovsdb_os_name_get(),
             vtysh_ovsdb_switch_version_get(), VTY_NEWLINE);
    return CMD_SUCCESS;
}
#endif /* ENABLE_OVSDB */

/* Show version detail. */
DEFUN (show_version_detail,
       show_version_detail_cmd,
       "show version detail",
       SHOW_STR
       SHOW_VERSION_STR
       SHOW_VERSION_DETAIL_STR)
{
  vtysh_ovsdb_show_version_detail();
  return CMD_SUCCESS;
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

    /* Versioning */
    /* show version */
    /* Add software_info column */
    ovsdb_idl_add_column(idl, &ovsrec_system_col_software_info);

    /* Add switch version column */
    ovsdb_idl_add_column(idl, &ovsrec_system_col_switch_version);

    /* show version detail */
    /* Add Package_Info table for show version detail. */
    ovsdb_idl_add_table(idl, &ovsrec_table_package_info);

    /* Add name, src_url, src_type version column for show version detail. */
    ovsdb_idl_add_column(idl, &ovsrec_package_info_col_name);
    ovsdb_idl_add_column(idl, &ovsrec_package_info_col_version);
    ovsdb_idl_add_column(idl, &ovsrec_package_info_col_src_type);
    ovsdb_idl_add_column(idl, &ovsrec_package_info_col_src_url);
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

    install_element (ENABLE_NODE, &show_version_cmd);
    install_element (VIEW_NODE, &show_version_cmd);

    install_element (ENABLE_NODE, &show_version_detail_cmd);
    install_element (VIEW_NODE, &show_version_detail_cmd);
}
