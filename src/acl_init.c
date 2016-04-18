/****************************************************************************
 * (c) Copyright 2015-2016 Hewlett Packard Enterprise Development LP
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
 ***************************************************************************/

#include <config.h>

#include "acl_init.h"

#include <ops-utils.h>
#include <stdlib.h>
#include <sys/types.h>

#include "config-yaml.h"
#include "sysd_cfg_yaml.h"
#include "smap.h"
#include "util.h"
#include "vswitch-idl.h"
#include "openvswitch/vlog.h"

VLOG_DEFINE_THIS_MODULE(acl_init_hw_limits);

/**
 * Initializes acl max acls and max aces for the given txn and system_row.
 */
void
acl_init_hw_limits(struct ovsdb_idl_txn *txn,
               struct ovsrec_system *system_row)
{
    YamlAclInfo *acl_info;
    struct smap smap;

    acl_info = sysd_cfg_yaml_get_acl_info();

    /* max acls could be NULL only if YAML init has failed. */
    if ((NULL != acl_info->max_acls) && (NULL != acl_info->max_aces) &&
		(NULL != acl_info->max_aces_per_acl))
    {
        /* Store acl limitations in other_config column */
        smap_clone(&smap, &system_row->other_config);
        smap_replace(&smap, "max_acls", acl_info->max_acls);
        smap_replace(&smap, "max_aces", acl_info->max_aces);
        smap_replace(&smap, "max_aces_per_acl", acl_info->max_aces_per_acl);
        ovsrec_system_set_other_config(system_row, &smap);
        smap_destroy(&smap);
    }
    return;
}
