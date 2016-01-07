/****************************************************************************
 * (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *    License for the specific language governing permissions and limitations
 *    under the License.
 *
 *    Set initial values for QOS in the database.
 *
 ***************************************************************************/

#include <config-yaml.h>
#include <ops-utils.h>
#include <stdlib.h>
#include <sys/types.h>

#include "qos_init.h"
#include "qos_utils.h"
#include "smap.h"
#include "util.h"
#include "vswitch-idl.h"

/*
 * Initialize QOS trust.
 */
void
qos_init_trust(struct ovsdb_idl_txn *txn,
                           struct ovsrec_system *system_row) {
    struct smap smap;
    smap_clone(&smap, &system_row->qos_config);
    smap_replace(&smap, QOS_TRUST_KEY, QOS_TRUST_DEFAULT);
    ovsrec_system_set_qos_config(system_row, &smap);
    smap_destroy(&smap);
}

/*
 * Initialize default COS map.
 */
void
qos_init_cos_map(struct ovsdb_idl_txn *txn,
                           struct ovsrec_system *system_row) {

    struct ovsrec_qos_cos_map_entry *default_cos_map =
                                        qos_create_default_cos_map();

    /* Create the cos-map rows. */
    struct ovsrec_qos_cos_map_entry *cos_map_rows[QOS_COS_MAP_ENTRY_COUNT];
    int i;
    for (i = 0; i < QOS_COS_MAP_ENTRY_COUNT; i++) {
        struct ovsrec_qos_cos_map_entry *cos_map_row =
                                ovsrec_qos_cos_map_entry_insert(txn);

        ovsrec_qos_cos_map_entry_set_code_point(cos_map_row,
                default_cos_map[i].code_point);
        ovsrec_qos_cos_map_entry_set_local_priority(cos_map_row,
                default_cos_map[i].local_priority);
        ovsrec_qos_cos_map_entry_set_color(cos_map_row,
                default_cos_map[i].color);
        ovsrec_qos_cos_map_entry_set_description(cos_map_row,
                default_cos_map[i].description);

        cos_map_rows[i] = cos_map_row;
    }

    /* Update the system row. */
    ovsrec_system_set_qos_cos_map_entries(system_row, cos_map_rows,
            QOS_COS_MAP_ENTRY_COUNT);

    qos_destroy_default_cos_map(default_cos_map);
}

/*
 * Initialize default DSCP map.
 */
void
qos_init_dscp_map(struct ovsdb_idl_txn *txn,
                           struct ovsrec_system *system_row) {

    struct ovsrec_qos_dscp_map_entry *default_dscp_map =
                                qos_create_default_dscp_map();

    /* Create the dscp-map rows. */
    struct ovsrec_qos_dscp_map_entry *dscp_map_rows[QOS_DSCP_MAP_ENTRY_COUNT];
    int i;
    for (i = 0; i < QOS_DSCP_MAP_ENTRY_COUNT; i++) {

        struct ovsrec_qos_dscp_map_entry *dscp_map_row =
                            ovsrec_qos_dscp_map_entry_insert(txn);

        ovsrec_qos_dscp_map_entry_set_code_point(dscp_map_row,
                default_dscp_map[i].code_point);
        ovsrec_qos_dscp_map_entry_set_local_priority(dscp_map_row,
                default_dscp_map[i].local_priority);
        ovsrec_qos_dscp_map_entry_set_priority_code_point(dscp_map_row,
                default_dscp_map[i].priority_code_point, 1);
        ovsrec_qos_dscp_map_entry_set_color(dscp_map_row,
                default_dscp_map[i].color);
        ovsrec_qos_dscp_map_entry_set_description(dscp_map_row,
                default_dscp_map[i].description);

        dscp_map_rows[i] = dscp_map_row;
    }

    /* Update the system row. */
    ovsrec_system_set_qos_dscp_map_entries(system_row, dscp_map_rows,
            QOS_DSCP_MAP_ENTRY_COUNT);

    qos_destroy_default_dscp_map(default_dscp_map);
}
