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

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://board-utils.sh \
           file://us_console.sh \
           file://sol.sh \
           file://power_led.sh \
           file://post_led.sh \
           file://setup-gpio.sh \
           file://mdio.py \
           file://mount_data0.sh \
           file://asus_power.sh \
           file://power-on.sh \
           file://asus_us_mac.sh \
           file://rc.early \
           file://rc.local \
           file://src \
           file://start_us_monitor.sh \
           file://us_monitor.sh \
           file://eth0_mac_fixup.sh \
          "

OPENBMC_UTILS_FILES += " \
  board-utils.sh us_console.sh sol.sh power_led.sh post_led.sh \
  mdio.py asus_power.sh asus_us_mac.sh us_monitor.sh \
  "

DEPENDS_append = " update-rc.d-native"

do_install_append() {
  do_install_board
}

FILES_${PN} += "${sysconfdir}"
