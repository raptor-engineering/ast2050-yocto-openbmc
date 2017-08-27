/*
 * Copyright (C) 2017 Raptor Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "w83795.h"

#define I2C_BUS_DEVICE_NODE	"/dev/i2c-1"
#define I2C_SLAVE_DEVICE_ADDR	0x2f

int smbus_fd = -1;

int main() {
	struct drivers_i2c_w83795_config platform_configuration;

	// ASUS KGPE-D16
	platform_configuration.fanin_ctl1 = 0xff;
	platform_configuration.fanin_ctl2 = 0x00;
	platform_configuration.temp_ctl1 = 0x2a;
	platform_configuration.temp_ctl2 = 0x01;
	platform_configuration.temp_dtse = 0x03;
	platform_configuration.volt_ctl1 = 0xff;
	platform_configuration.volt_ctl2 = 0xf7;
	platform_configuration.temp1_fan_select = 0x00;
	platform_configuration.temp2_fan_select = 0x00;
	platform_configuration.temp3_fan_select = 0x00;
	platform_configuration.temp4_fan_select = 0x00;
	platform_configuration.temp5_fan_select = 0x00;
	platform_configuration.temp6_fan_select = 0x00;
	platform_configuration.temp1_source_select = 0x00;
	platform_configuration.temp2_source_select = 0x00;
	platform_configuration.temp3_source_select = 0x00;
	platform_configuration.temp4_source_select = 0x00;
	platform_configuration.temp5_source_select = 0x00;
	platform_configuration.temp6_source_select = 0x00;
	platform_configuration.tr1_critical_temperature = 85;
	platform_configuration.tr1_critical_hysteresis = 80;
	platform_configuration.tr1_warning_temperature = 70;
	platform_configuration.tr1_warning_hysteresis = 65;
	platform_configuration.dts_critical_temperature = 85;
	platform_configuration.dts_critical_hysteresis = 80;
	platform_configuration.dts_warning_temperature = 70;
	platform_configuration.dts_warning_hysteresis = 65;
	platform_configuration.temp1_critical_temperature = 80;
	platform_configuration.temp2_critical_temperature = 80;
	platform_configuration.temp3_critical_temperature = 80;
	platform_configuration.temp4_critical_temperature = 80;
	platform_configuration.temp5_critical_temperature = 80;
	platform_configuration.temp6_critical_temperature = 80;
	platform_configuration.temp1_target_temperature = 80;
	platform_configuration.temp2_target_temperature = 80;
	platform_configuration.temp3_target_temperature = 80;
	platform_configuration.temp4_target_temperature = 80;
	platform_configuration.temp5_target_temperature = 80;
	platform_configuration.temp6_target_temperature = 80;
	platform_configuration.fan1_nonstop = 7;
	platform_configuration.fan2_nonstop = 7;
	platform_configuration.fan3_nonstop = 7;
	platform_configuration.fan4_nonstop = 7;
	platform_configuration.fan5_nonstop = 7;
	platform_configuration.fan6_nonstop = 7;
	platform_configuration.fan7_nonstop = 7;
	platform_configuration.fan8_nonstop = 7;
	platform_configuration.default_speed = 100;
	platform_configuration.fan1_duty = 100;
	platform_configuration.fan2_duty = 100;
	platform_configuration.fan3_duty = 100;
	platform_configuration.fan4_duty = 100;
	platform_configuration.fan5_duty = 100;
	platform_configuration.fan6_duty = 100;
	platform_configuration.fan7_duty = 100;
	platform_configuration.fan8_duty = 100;
	platform_configuration.vcore1_high_limit_mv = 1500;
	platform_configuration.vcore1_low_limit_mv = 900;
	platform_configuration.vcore2_high_limit_mv = 1500;
	platform_configuration.vcore2_low_limit_mv = 900;
	platform_configuration.vsen3_high_limit_mv = 1600;
	platform_configuration.vsen3_low_limit_mv = 1100;
	platform_configuration.vsen4_high_limit_mv = 1600;
	platform_configuration.vsen4_low_limit_mv = 1100;
	platform_configuration.vsen5_high_limit_mv = 1250;
	platform_configuration.vsen5_low_limit_mv = 1150;
	platform_configuration.vsen6_high_limit_mv = 1250;
	platform_configuration.vsen6_low_limit_mv = 1150;
	platform_configuration.vsen7_high_limit_mv = 1250;
	platform_configuration.vsen7_low_limit_mv = 1050;
	platform_configuration.vsen8_high_limit_mv = 1900;
	platform_configuration.vsen8_low_limit_mv = 1700;
	platform_configuration.vsen9_high_limit_mv = 1250;
	platform_configuration.vsen9_low_limit_mv = 1150;
	platform_configuration.vsen10_high_limit_mv = 1150;
	platform_configuration.vsen10_low_limit_mv = 1050;
	platform_configuration.vsen11_high_limit_mv = 1625;
	platform_configuration.vsen11_low_limit_mv = 1500;
	platform_configuration.vsen12_high_limit_mv = 1083;
	platform_configuration.vsen12_low_limit_mv = 917;
	platform_configuration.vsen13_high_limit_mv = 1625;
	platform_configuration.vsen13_low_limit_mv = 1500;
	platform_configuration.vdd_high_limit_mv = 3500;
	platform_configuration.vdd_low_limit_mv = 3100;
	platform_configuration.vsb_high_limit_mv = 3500;
	platform_configuration.vsb_low_limit_mv = 3100;
	platform_configuration.vbat_high_limit_mv = 3500;
	platform_configuration.vbat_low_limit_mv = 2500;
	platform_configuration.smbus_aux = 1;

	smbus_fd = open(I2C_BUS_DEVICE_NODE, O_RDWR);
	if (smbus_fd < 0) {
		printf("Unable to open I2C device node %s for read/write!\n", I2C_BUS_DEVICE_NODE);
		return 1;
	}

	if (ioctl(smbus_fd, I2C_SLAVE, I2C_SLAVE_DEVICE_ADDR) < 0) {
		close(smbus_fd);
		printf("Unable to set up communication with I2C device at address %02x!\n", I2C_SLAVE_DEVICE_ADDR);
		return 1;
	}

	w83795_init(MANUAL_MODE, DTS_SRC_AMD_SBTSI, &platform_configuration);

	close(smbus_fd);
}