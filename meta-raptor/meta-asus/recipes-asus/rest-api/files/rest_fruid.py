#!/usr/bin/env python
#
# Copyright 2017 Raptor Engineering, LLC
# Copyright 2014-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#

import subprocess

def get_fruid():

    mac2str = lambda mac: ':'.join(['{:02X}'.format(b) for b in mac])

    fruinfo = { "Version": subprocess.Popen(['fw_printenv', 'fru_version'], stdout=subprocess.PIPE).communicate()[0],
                "Product Name": subprocess.Popen(['fw_printenv', 'fru_product'], stdout=subprocess.PIPE).communicate()[0],
                "Product Part Number": subprocess.Popen(['fw_printenv', 'fru_product_pn'], stdout=subprocess.PIPE).communicate()[0],
                "System Assembly Part Number": subprocess.Popen(['fw_printenv', 'fru_subassembly'], stdout=subprocess.PIPE).communicate()[0],
                "ODM PCB Part Number": subprocess.Popen(['fw_printenv', 'fru_odm_pn'], stdout=subprocess.PIPE).communicate()[0],
                "ODM PCB Serial Number": subprocess.Popen(['fw_printenv', 'fru_odm_serial'], stdout=subprocess.PIPE).communicate()[0],
                "Product Production State": subprocess.Popen(['fw_printenv', 'fru_prod_stat'], stdout=subprocess.PIPE).communicate()[0],
                "Product Version": subprocess.Popen(['fw_printenv', 'fru_prod_major_version'], stdout=subprocess.PIPE).communicate()[0],
                "Product Sub-Version": subprocess.Popen(['fw_printenv', 'fru_prod_minor_version'], stdout=subprocess.PIPE).communicate()[0],
                "Product Serial Number": subprocess.Popen(['fw_printenv', 'fru_prod_serial'], stdout=subprocess.PIPE).communicate()[0],
                "Product Asset Tag": subprocess.Popen(['fw_printenv', 'fru_asset_tag'], stdout=subprocess.PIPE).communicate()[0],
                "System Manufacturer": subprocess.Popen(['fw_printenv', 'fru_prod_manufacturer'], stdout=subprocess.PIPE).communicate()[0],
                "System Manufacturing Date": subprocess.Popen(['fw_printenv', 'fru_manufacturing_date'], stdout=subprocess.PIPE).communicate()[0],
                "PCB Manufacturer": subprocess.Popen(['fw_printenv', 'fru_pcb_manufacturer'], stdout=subprocess.PIPE).communicate()[0],
                "Assembled At": subprocess.Popen(['fw_printenv', 'fru_assembled_at'], stdout=subprocess.PIPE).communicate()[0],
                "Local MAC": subprocess.Popen(['fw_printenv', 'ethaddr'], stdout=subprocess.PIPE).communicate()[0],
                "Location": subprocess.Popen(['fw_printenv', 'fru_location'], stdout=subprocess.PIPE).communicate()[0]
                }

    result = {
                "Information": fruinfo,
                "Actions": [],
                "Resources": [],
             }

    return result
