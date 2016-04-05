# -*- coding: utf-8 -*-
#
# Copyright (C) 2015-2016 Hewlett Packard Enterprise Development LP
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

"""
OpenSwitch Test for vlan related configurations.
"""

from __future__ import unicode_literals, absolute_import
from __future__ import print_function, division

import re

TOPOLOGY = """
# +-------+
# |  ops1 |
# +-------+

# Nodes
[type=openswitch name="OpenSwitch 1"] ops1

"""


def test_pintf_mac(topology, step):
    """
    Test per interface mac address assignment
    configuration is functional with a OpenSwitch switch.

    Build a topology of stand alone switch.
    Check each interface has got unique mac address
    which is different from system-mac.
    """
    ops1 = topology.get('ops1')

    assert ops1 is not None

    step('Test to check per interface mac address started')

    bufferout = ops1(
        'list subsystem | grep macs_remaining',
        shell='vsctl'
    )
#    print(bufferout)
    result = re.search(r'\s*macs_remaining\s*:\s*(?P<mac_remaining>[0-9])\s*',
                       bufferout)
    result = result.groupdict()
    assert(int(result['mac_remaining']) == 0), \
        'Error mac remaining is non zero'

    bufferout = ops1(
        'list system | grep system_mac',
        shell='vsctl'
    )
    result = re.search(r'\s*system_mac\s*:\s*\"(?P<sys_mac>[^\"]+)\s*',
                       bufferout)
    result = result.groupdict()
#    print(result['sys_mac'])
    sys_mac_int = int(result['sys_mac'].replace(':', ''), 16)
#    print(sys_mac_int)

    bufferout = ops1(
        'list subsystem | grep next_mac_address',
        shell='vsctl'
    )
    result = re.search(r'\s*next_mac_address\s*:\s*\"(?P<next_mac>[^\"]+)\s*',
                       bufferout)
    result = result.groupdict()
#    print(result['next_mac'])
    next_mac_int = int(result['next_mac'].replace(':', ''), 16)
#    print(next_mac_int)

    assert(next_mac_int == sys_mac_int), \
        'Error after consuming all mac address next mac addr is not sys mac'

    interface = 1
    while interface <= 48:
        ops1(
            'set interface {} user_config:admin=up'.format(interface),
            shell='vsctl'
        )

        bufferout = ops1(
            'list interface {} | grep mac_in_use'.format(interface),
            shell='vsctl'
        )

        result = re.search(r'\s*mac_in_use\s*:\s*\"(?P<intf_mac>[^\"]+)\s*',
                           bufferout)
        result = result.groupdict()
#        print(result['intf_mac'])
        intf_mac_int = int(result['intf_mac'].replace(':', ''), 16)
#        print(intf_mac_int)

        assert(intf_mac_int == sys_mac_int + interface), \
            'Error invalid interface {interface} mac address'

        bufferout = ops1(
            'ip netns exec swns ifconfig {} | grep HWaddr'.format(interface),
            shell='bash'
        )

        result = \
            re.search(r'Ethernet\s*HWaddr\s*(?P<intf_ifconfig_mac>[^ ]+)\s*',
                      bufferout)
        result = result.groupdict()
#        print(result['intf_ifconfig_mac'])
        intf_ifconfig_mac_int = int(result['intf_ifconfig_mac']
                                    .replace(':', ''), 16)
#        print(intf_ifconfig_mac_int)

        assert(intf_mac_int == intf_ifconfig_mac_int), \
            'Error invalid interface {interface} ifconfig mac address'
        interface = interface + 1

    interface = 49
    multiplier = 0
    while interface <= 54:
        split = 0
        while split <= 4:
            if not split:
                ops1(
                    'set interface {} user_config:admin=up'.format(interface),
                    shell='vsctl'
                )
                bufferout = ops1(
                    'list interface {} | grep mac_in_use'.format(interface),
                    shell='vsctl'
                )
#                print(bufferout)
            else:
                ops1(
                    'set interface {}-{} user_config:admin=up'
                    .format(interface, split), shell='vsctl'
                )
                bufferout = ops1(
                    'list interface {}-{} | grep mac_in_use'
                    .format(interface, split), shell='vsctl'
                )
#                print(bufferout)
            result = \
                re.search(r'\s*mac_in_use\s*:\s*\"(?P<intf_mac>[^\"]+)\s*',
                          bufferout)
            result = result.groupdict()
            print(result['intf_mac'])
            intf_mac_int = int(result['intf_mac'].replace(':', ''), 16)
            print(intf_mac_int)

            if split > 1:
                assert(intf_mac_int == sys_mac_int + interface + split - 1 +
                       (multiplier * 3)), \
                    'Error invalid interface {interface} mac address'
            else:
                assert(intf_mac_int == sys_mac_int + interface +
                       (multiplier * 3)), \
                    'Error invalid interface {interface} mac address'
            split = split + 1
        interface = interface + 1
        multiplier = multiplier + 1

    step('Test to check per interface mac address passed')
