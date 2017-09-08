/*
 * fand
 *
 * Copyright 2017 Raptor Engineering, LLC
 * Copyright 2014-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Daemon to manage the fan speed to ensure that we stay within a reasonable
 * temperature range.  We're using a simplistic algorithm to get started:
 *
 * If the fan is already on high, we'll move it to medium if we fall below
 * a top temperature.  If we're on medium, we'll move it to high
 * if the temperature goes over the top value, and to low if the
 * temperature falls to a bottom level.  If the fan is on low,
 * we'll increase the speed if the temperature rises to the top level.
 *
 * To ensure that we're not just turning the fans up, then back down again,
 * we'll require an extra few degrees of temperature drop before we lower
 * the fan speed.
 *
 * We check the RPM of the fans against the requested RPMs to determine
 * whether the fans are failing, in which case we'll turn up all of
 * the other fans and report the problem..
 *
 * TODO:  Implement a PID algorithm to closely track the ideal temperature.
 * TODO:  Determine if the daemon is already started.
 */

/* Yeah, the file ends in .cpp, but it's a C program.  Deal. */

/* XXX:  Both CONFIG_WEDGE and CONFIG_WEDGE100 are defined for Wedge100 */

#if !defined(CONFIG_YOSEMITE) && !defined(CONFIG_WEDGE) && \
    !defined(CONFIG_WEDGE100) && !defined(CONFIG_ASUS)
#error "No hardware platform defined!"
#endif
#if (defined(CONFIG_YOSEMITE) && defined(CONFIG_WEDGE)) || \
    (defined(CONFIG_ASUS) && defined(CONFIG_WEDGE))
#error "Two hardware platforms defined!"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#if defined(CONFIG_YOSEMITE)
#include <openbmc/ipmi.h>
#include <facebook/bic.h>
#include <facebook/yosemite_sensor.h>
#endif
#if defined(CONFIG_WEDGE) && !defined(CONFIG_WEDGE100)
#include <facebook/wedge_eeprom.h>
#endif

#if defined(CONFIG_ASUS)
#define CONFIG_SLOPPY_FANS 1
#endif

#include "watchdog.h"

/* Sensor definitions */

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
#define INTERNAL_TEMPS(x) ((x) * 1000) // stored as C * 1000
#define EXTERNAL_TEMPS(x) ((x) / 1000)
#elif defined(CONFIG_YOSEMITE)
#define INTERNAL_TEMPS(x) (x)
#define EXTERNAL_TEMPS(x) (x)
#define TOTAL_1S_SERVERS 4
#elif defined(CONFIG_ASUS)
#define INTERNAL_TEMPS(x) ((x) * 1000.0) // stored as C * 1000
#define EXTERNAL_TEMPS(x) ((x) / 1000.0)
#endif

/*
 * The sensor for the uServer CPU is not on the CPU itself, so it reads
 * a little low.  We are special casing this, but we should obviously
 * be thinking about a way to generalize these tweaks, and perhaps
 * the entire configuration.  JSON file?
 */

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
#define USERVER_TEMP_FUDGE INTERNAL_TEMPS(10)
#else
#define USERVER_TEMP_FUDGE INTERNAL_TEMPS(0)
#endif

#define BAD_TEMP INTERNAL_TEMPS(-60)
#define BAD_VOLTAGE (-50)

#define BAD_READ_THRESHOLD 4    /* How many times can reads fail */
#define FAN_FAILURE_THRESHOLD 4 /* How many times can a fan fail */
#define FAN_SHUTDOWN_THRESHOLD 20 /* How long fans can be failed before */
                                  /* we just shut down the whole thing. */

#if defined(CONFIG_WEDGE100)
#define PWM_DIR "/sys/bus/i2c/drivers/fancpld/8-0033/"

#define PWM_UNIT_MAX 31

#define LM75_DIR "/sys/bus/i2c/drivers/lm75/"
#define PANTHER_PLUS_DIR "/sys/bus/i2c/drivers/panther_plus/"

#define INTAKE_TEMP_DEVICE LM75_DIR "3-0048"
#define CHIP_TEMP_DEVICE LM75_DIR "3-004b"
#define EXHAUST_TEMP_DEVICE LM75_DIR "3-0048"
#define USERVER_TEMP_DEVICE PANTHER_PLUS_DIR "4-0040"

#define FAN_READ_RPM_FORMAT "fan%d_input"

#define FAN0_LED PWM_DIR "fantray1_led_ctrl"
#define FAN1_LED PWM_DIR "fantray2_led_ctrl"
#define FAN2_LED PWM_DIR "fantray3_led_ctrl"
#define FAN3_LED PWM_DIR "fantray4_led_ctrl"
#define FAN4_LED PWM_DIR "fantray5_led_ctrl"

#define FAN_LED_BLUE "0x1"
#define FAN_LED_RED "0x2"

#define MAIN_POWER "/sys/bus/i2c/drivers/syscpld/12-0031/pwr_main_n"
#define USERVER_POWER "/sys/bus/i2c/drivers/syscpld/12-0031/pwr_usrv_en"

#elif defined(CONFIG_WEDGE)
#define I2C_BUS_3_DIR "/sys/class/i2c-adapter/i2c-3/"
#define I2C_BUS_4_DIR "/sys/class/i2c-adapter/i2c-4/"

#define INTAKE_TEMP_DEVICE I2C_BUS_3_DIR "3-0048"
#define CHIP_TEMP_DEVICE I2C_BUS_3_DIR "3-0049"
#define EXHAUST_TEMP_DEVICE I2C_BUS_3_DIR "3-004a"
#define USERVER_TEMP_DEVICE I2C_BUS_4_DIR "4-0040"


#define FAN0_LED "/sys/class/gpio/gpio53/value"
#define FAN1_LED "/sys/class/gpio/gpio54/value"
#define FAN2_LED "/sys/class/gpio/gpio55/value"
#define FAN3_LED "/sys/class/gpio/gpio72/value"

#define FAN_LED_RED "0"
#define FAN_LED_BLUE "1"

#define GPIO_PIN_ID "/sys/class/gpio/gpio%d/value"
#define REV_IDS 3
#define GPIO_REV_ID_START 192

#define BOARD_IDS 4
#define GPIO_BOARD_ID_START 160

/*
 * With hardware revisions after 3, we use a different set of pins for
 * the BOARD_ID.
 */

#define REV_ID_NEW_BOARD_ID 3
#define GPIO_BOARD_ID_START_NEW 166

#elif defined(CONFIG_YOSEMITE)
#define FAN_LED_RED "0"
#define FAN_LED_BLUE "1"

#elif defined(CONFIG_ASUS)
#define PWM_DIR "/sys/class/i2c-adapter/i2c-1/1-002f/"

#define FAN_READ_RPM_FORMAT "fan%d_input"
#define PWM_UNIT_MAX 255

#define CHIPSET_TEMP_DEVICE PWM_DIR "temp1_input"
#define CPU1_TEMP_DEVICE PWM_DIR "temp7_input"
#define CPU2_TEMP_DEVICE PWM_DIR "temp8_input"
#define CPU2_VCORE_DEVICE PWM_DIR "in1_input"

#define MAIN_POWER_OFF_DIRECTION "/sys/class/gpio/gpio40/direction"
#define MAIN_POWER_OFF_CTL "/sys/class/gpio/gpio40/value"

#define FAN_LED_RED "1"
#define FAN_LED_BLUE "0"

#endif

#if (defined(CONFIG_YOSEMITE) || defined(CONFIG_WEDGE)) && \
    !defined(CONFIG_WEDGE100) && !defined(CONFIG_ASUS)
#define PWM_DIR "/sys/devices/platform/ast_pwm_tacho.0"

#define PWM_UNIT_MAX 96
#define FAN_READ_RPM_FORMAT "tacho%d_rpm"

#define GPIO_USERVER_POWER_DIRECTION "/sys/class/gpio/gpio25/direction"
#define GPIO_USERVER_POWER "/sys/class/gpio/gpio25/value"
#define GPIO_T2_POWER_DIRECTION "/tmp/gpionames/T2_POWER_UP/direction"
#define GPIO_T2_POWER "/tmp/gpionames/T2_POWER_UP/value"
#endif

#define LARGEST_DEVICE_NAME 120

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
const char *fan_led[] = {FAN0_LED, FAN1_LED, FAN2_LED, FAN3_LED,
#if defined(CONFIG_WEDGE100)
                         FAN4_LED,
#endif
};
#endif

#define REPORT_TEMP 720  /* Report temp every so many cycles */

/* Sensor limits and tuning parameters */

#define INTAKE_LIMIT INTERNAL_TEMPS(60)
#define SWITCH_LIMIT INTERNAL_TEMPS(80)
#define CHIPSET_LIMIT INTERNAL_TEMPS(80)
#if defined(CONFIG_YOSEMITE)
#define USERVER_LIMIT INTERNAL_TEMPS(110)
#else
#define USERVER_LIMIT INTERNAL_TEMPS(90)
#endif

#if defined(CONFIG_ASUS)
#define CPU_LIMIT INTERNAL_TEMPS(110)
#endif

#define TEMP_TOP INTERNAL_TEMPS(70)
#define TEMP_BOTTOM INTERNAL_TEMPS(40)

/*
 * Toggling the fan constantly will wear it out (and annoy anyone who
 * can hear it), so we'll only turn down the fan after the temperature
 * has dipped a bit below the point at which we'd otherwise switch
 * things up.
 */

#define COOLDOWN_SLOP INTERNAL_TEMPS(6)

#define WEDGE_FAN_LOW 35
#define WEDGE_FAN_MEDIUM 50
#define WEDGE_FAN_HIGH 70
#if defined(CONFIG_WEDGE100) || defined(CONFIG_ASUS)
#define WEDGE_FAN_MAX 100
#else
#define WEDGE_FAN_MAX 99
#endif

#define SIXPACK_FAN_LOW 35
#define SIXPACK_FAN_MEDIUM 55
#define SIXPACK_FAN_HIGH 75
#define SIXPACK_FAN_MAX 99

/*
 * Mapping physical to hardware addresses for fans;  it's different for
 * RPM measuring and PWM setting, naturally.  Doh.
 */

#if defined(CONFIG_WEDGE100)
int fan_population_map[] = {1, 1, 1, 1, 1};
int fan_to_rpm_map[]     = {1, 3, 5, 7, 9};
int fan_to_pwm_map[]     = {1, 2, 3, 4, 5};
#define FANS 5
// Tacho offset between front and rear fans:
#define REAR_FAN_OFFSET 1
#define BACK_TO_BACK_FANS

#elif defined(CONFIG_WEDGE)
int fan_population_map[] = {1, 1, 1, 1};
int fan_to_rpm_map[]     = {3, 2, 0, 1};
int fan_to_pwm_map[]     = {7, 6, 0, 1};
#define FANS 4
// Tacho offset between front and rear fans:
#define REAR_FAN_OFFSET 4
#define BACK_TO_BACK_FANS
#elif defined(CONFIG_YOSEMITE)
int fan_population_map[] = {1, 1};
int fan_to_rpm_map[]     = {0, 1};
int fan_to_pwm_map[]     = {0, 1};
#define FANS 2
// Tacho offset between front and rear fans:
#define REAR_FAN_OFFSET 1

#elif defined(CONFIG_ASUS)
float fan_scale_speed_map[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};	// Chassis fans may have a different max rpm than 2000; scale the stock profile accordingly.
int fan_population_map[] = {1, 1, 1, 1, 1, 1, 1, 1};	// 1 == fan populated, 0 == fan disconnected
int fan_to_rpm_map[]     = {1, 2, 3, 4, 5, 6, 7, 8};
int fan_to_pwm_map[]     = {1, 1, 2, 2, 2, 2, 2, 2};	// 1 == 4 pin fans, 2 == 3 pin fans
#define FANS 8

#endif


/*
 * The measured RPM of the fans doesn't match linearly to the requested
 * rate.  In addition, there are coaxially mounted fans, so the rear fans
 * feed into the front fans.  The rear fans will run slower since they're
 * grabbing still air, and the front fants are getting an extra boost.
 *
 * We'd like to measure the fan RPM and compare it to the expected RPM
 * so that we can detect failed fans, so we have a table (derived from
 * hardware testing):
 */

struct rpm_to_pct_map {
  uint pct;
  uint rpm;
};

#if defined(CONFIG_WEDGE100)
struct rpm_to_pct_map rpm_front_map[] = {{20, 4200},
                                         {25, 5550},
                                         {30, 6180},
                                         {35, 7440},
                                         {40, 8100},
                                         {45, 9300},
                                         {50, 10410},
                                         {55, 10920},
                                         {60, 11910},
                                         {65, 12360},
                                         {70, 13260},
                                         {75, 14010},
                                         {80, 14340},
                                         {85, 15090},
                                         {90, 15420},
                                         {95, 15960},
                                         {100, 16200}};
#define FRONT_MAP_SIZE (sizeof(rpm_front_map) / sizeof(struct rpm_to_pct_map))

struct rpm_to_pct_map rpm_rear_map[] = {{20, 2130},
                                        {25, 3180},
                                        {30, 3690},
                                        {35, 4620},
                                        {40, 5130},
                                        {45, 6120},
                                        {50, 7050},
                                        {55, 7560},
                                        {60, 8580},
                                        {65, 9180},
                                        {70, 10230},
                                        {75, 11280},
                                        {80, 11820},
                                        {85, 12870},
                                        {90, 13350},
                                        {95, 14370},
                                        {100, 14850}};
#define REAR_MAP_SIZE (sizeof(rpm_rear_map) / sizeof(struct rpm_to_pct_map))
#elif defined(CONFIG_WEDGE)
struct rpm_to_pct_map rpm_front_map[] = {{30, 6150},
                                         {35, 7208},
                                         {40, 8195},
                                         {45, 9133},
                                         {50, 10017},
                                         {55, 10847},
                                         {60, 11612},
                                         {65, 12342},
                                         {70, 13057},
                                         {75, 13717},
                                         {80, 14305},
                                         {85, 14869},
                                         {90, 15384},
                                         {95, 15871},
                                         {100, 16095}};
#define FRONT_MAP_SIZE (sizeof(rpm_front_map) / sizeof(struct rpm_to_pct_map))

struct rpm_to_pct_map rpm_rear_map[] = {{30, 3911},
                                        {35, 4760},
                                        {40, 5587},
                                        {45, 6434},
                                        {50, 7295},
                                        {55, 8187},
                                        {60, 9093},
                                        {65, 10008},
                                        {70, 10949},
                                        {75, 11883},
                                        {80, 12822},
                                        {85, 13726},
                                        {90, 14690},
                                        {95, 15516},
                                        {100, 15897}};
#define REAR_MAP_SIZE (sizeof(rpm_rear_map) / sizeof(struct rpm_to_pct_map))
#elif defined(CONFIG_YOSEMITE)

/* XXX:  Note that 0% is far from 0 RPM. */
struct rpm_to_pct_map rpm_map[] = {{0, 989},
                                   {10, 1654},
                                   {20, 2650},
                                   {30, 3434},
                                   {40, 4318},
                                   {50, 5202},
                                   {60, 5969},
                                   {70, 6869},
                                   {80, 7604},
                                   {90, 8525},
                                   {100, 9325}};

struct rpm_to_pct_map *rpm_front_map = rpm_map;
struct rpm_to_pct_map *rpm_rear_map = rpm_map;
#define MAP_SIZE (sizeof(rpm_map) / sizeof(struct rpm_to_pct_map))
#define FRONT_MAP_SIZE MAP_SIZE
#define REAR_MAP_SIZE MAP_SIZE

#elif defined(CONFIG_ASUS)
struct rpm_to_pct_map rpm_front_map[] = {{15, 1804},
                                         {20, 1864},
                                         {25, 2011},
                                         {30, 2268},
                                         {35, 2683},
                                         {40, 3110},
                                         {45, 3619},
                                         {50, 4017},
                                         {55, 4455},
                                         {60, 4720},
                                         {65, 5113},
                                         {70, 5487},
                                         {75, 5973},
                                         {80, 6428},
                                         {85, 6994},
                                         {90, 7417},
                                         {95, 7941},
                                         {100, 8490}};
#define FRONT_MAP_SIZE (sizeof(rpm_front_map) / sizeof(struct rpm_to_pct_map))

struct rpm_to_pct_map rpm_rear_map[] = {{15, 450},
                                        {20, 600},
                                        {25, 750},
                                        {30, 900},
                                        {35, 1050},
                                        {40, 1200},
                                        {45, 1350},
                                        {50, 1500},
                                        {55, 1550},
                                        {60, 1600},
                                        {65, 1650},
                                        {70, 1700},
                                        {75, 1750},
                                        {80, 1800},
                                        {85, 1850},
                                        {90, 1900},
                                        {95, 1950},
                                        {100, 2000}};
#define REAR_MAP_SIZE (sizeof(rpm_rear_map) / sizeof(struct rpm_to_pct_map))

#endif

/*
 * Mappings from temperatures recorded from sensors to fan speeds;
 * note that in some cases, we want to be able to look at offsets
 * from the CPU temperature margin rather than an absolute temperature,
 * so we use ints.
 */

struct temp_to_pct_map {
  int temp;
  unsigned speed;
};

#if defined(CONFIG_YOSEMITE)
struct temp_to_pct_map intake_map[] = {{25, 15},
                                       {27, 16},
                                       {29, 17},
                                       {31, 18},
                                       {33, 19},
                                       {35, 20},
                                       {37, 21},
                                       {39, 22},
                                       {41, 23},
                                       {43, 24},
                                       {45, 25}};
#define INTAKE_MAP_SIZE (sizeof(intake_map) / sizeof(struct temp_to_pct_map))

struct temp_to_pct_map cpu_map[] = {{-28, 10},
                                    {-26, 20},
                                    {-24, 25},
                                    {-22, 30},
                                    {-20, 35},
                                    {-18, 40},
                                    {-16, 45},
                                    {-14, 50},
                                    {-12, 55},
                                    {-10, 60},
                                    {-8, 65},
                                    {-6, 70},
                                    {-4, 80},
                                    {-2, 100}};
#define CPU_MAP_SIZE (sizeof(cpu_map) / sizeof(struct temp_to_pct_map))
#endif

#if defined(CONFIG_ASUS)
struct temp_to_pct_map chipset_map[] = {{30, 1},
                                       {32, 5},
                                       {35, 10},
                                       {37, 15},
                                       {40, 20},
                                       {42, 25},
                                       {45, 30},
                                       {47, 35},
                                       {50, 40},
                                       {52, 45},
                                       {55, 50},
                                       {57, 55},
                                       {60, 60},
                                       {62, 65},
                                       {65, 70},
                                       {67, 80},
                                       {70, 100}};
#define CHIPSET_MAP_SIZE (sizeof(chipset_map) / sizeof(struct temp_to_pct_map))

struct temp_to_pct_map cpu_map[] = {{38, 1},
                                    {40, 5},
                                    {42, 10},
                                    {44, 15},
                                    {46, 20},
                                    {48, 25},
                                    {50, 30},
                                    {52, 35},
                                    {54, 40},
                                    {56, 45},
                                    {58, 50},
                                    {60, 55},
                                    {62, 60},
                                    {64, 65},
                                    {66, 70},
                                    {68, 80},
                                    {70, 100}};
#define CPU_MAP_SIZE (sizeof(cpu_map) / sizeof(struct temp_to_pct_map))
#endif

#define FAN_FAILURE_OFFSET 30

int fan_low = WEDGE_FAN_LOW;
int fan_medium = WEDGE_FAN_MEDIUM;
int fan_high = WEDGE_FAN_HIGH;
int fan_max = WEDGE_FAN_MAX;
int total_fans = FANS;
int fan_offset = 0;

int temp_bottom = TEMP_BOTTOM;
int temp_top = TEMP_TOP;

int report_temp = REPORT_TEMP;
bool verbose = false;

#if defined(CONFIG_WEDGE)
  int cpu2_installed = 0;
#elif defined(CONFIG_ASUS)
  int cpu2_installed = 1;
#else
  int cpu2_installed = 0;
#endif

void usage() {
  fprintf(stderr,
          "fand [-v] [-l <low-pct>] [-m <medium-pct>] "
          "[-h <high-pct>]\n"
          "\t[-b <temp-bottom>] [-t <temp-top>] [-r <report-temp>]\n\n"
          "\tlow-pct defaults to %d%% fan\n"
          "\tmedium-pct defaults to %d%% fan\n"
          "\thigh-pct defaults to %d%% fan\n"
          "\ttemp-bottom defaults to %dC\n"
          "\ttemp-top defaults to %dC\n"
          "\treport-temp defaults to every %d measurements\n\n"
          "fand compensates for uServer temperature reading %d degrees low\n"
          "kill with SIGUSR1 to stop watchdog\n",
          fan_low,
          fan_medium,
          fan_high,
          EXTERNAL_TEMPS(temp_bottom),
          EXTERNAL_TEMPS(temp_top),
          report_temp,
          EXTERNAL_TEMPS(USERVER_TEMP_FUDGE));
  exit(1);
}

/* We need to open the device each time to read a value */

int read_device(const char *device, int *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp) {
    int err = errno;

    syslog(LOG_INFO, "failed to open device %s", device);
    return err;
  }

  rc = fscanf(fp, "%d", value);
  fclose(fp);

  if (rc != 1) {
    syslog(LOG_INFO, "failed to read device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

/* We need to open the device again each time to write a value */

int write_device(const char *device, const char *value) {
  FILE *fp;
  int rc;

  fp = fopen(device, "w");
  if (!fp) {
    int err = errno;

    syslog(LOG_INFO, "failed to open device for write %s", device);
    return err;
  }

  rc = fputs(value, fp);
  fclose(fp);

  if (rc < 0) {
    syslog(LOG_INFO, "failed to write device %s", device);
    return ENOENT;
  } else {
    return 0;
  }
}

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
int read_temp(const char *device, int *value) {
  char full_name[LARGEST_DEVICE_NAME + 1];

  /* We set an impossible value to check for errors */
  *value = BAD_TEMP;
  snprintf(
      full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);
  return read_device(full_name, value);
}
#endif

#if defined(CONFIG_ASUS)
int read_temp(const char *device, int *value) {
  char full_name[LARGEST_DEVICE_NAME + 1];

  /* We set an impossible value to check for errors */
  *value = BAD_TEMP;
  snprintf(
      full_name, LARGEST_DEVICE_NAME, "%s", device);
  return read_device(full_name, value);
}
#endif

int read_voltage(const char *device, int *value) {
  char full_name[LARGEST_DEVICE_NAME + 1];

  /* We set an impossible value to check for errors */
  *value = BAD_VOLTAGE;
  snprintf(
      full_name, LARGEST_DEVICE_NAME, "%s", device);
  return read_device(full_name, value);
}

#if defined(CONFIG_WEDGE) && !defined(CONFIG_WEDGE100)
int read_gpio_value(const int id, const char *device, int *value) {
  char full_name[LARGEST_DEVICE_NAME];

  snprintf(full_name, LARGEST_DEVICE_NAME, device, id);
  return read_device(full_name, value);
}

int read_gpio_values(const int start, const int count,
                     const char *device, int *result) {
  int status = 0;
  int value;

  *result = 0;
  for (int i = 0; i < count; i++) {
      status |= read_gpio_value(start + i, GPIO_PIN_ID, &value);
      *result |= value << i;
  }
  return status;
}

int read_ids(int *rev_id, int *board_id) {
  int status = 0;
  int value;

  status = read_gpio_values(GPIO_REV_ID_START, REV_IDS, GPIO_PIN_ID, rev_id);
  if (status != 0) {
    syslog(LOG_INFO, "failed to read rev_id");
    return status;
  }

  int board_id_start;
  if (*rev_id >= REV_ID_NEW_BOARD_ID) {
    board_id_start = GPIO_BOARD_ID_START_NEW;
  } else {
    board_id_start = GPIO_BOARD_ID_START;
  }

  status = read_gpio_values(board_id_start, BOARD_IDS, GPIO_PIN_ID, board_id);
  if (status != 0) {
    syslog(LOG_INFO, "failed to read board_id");
  }
  return status;
}

bool is_two_fan_board(bool verbose) {
  struct wedge_eeprom_st eeprom;
  /* Retrieve the board type from EEPROM */
  if (wedge_eeprom_parse(NULL, &eeprom) == 0) {
    /* able to parse EEPROM */
    if (verbose) {
      syslog(LOG_INFO, "board type is %s", eeprom.fbw_location);
    }
    /* only WEDGE is NOT two-fan board */
    return strncasecmp(eeprom.fbw_location, "wedge",
                       sizeof(eeprom.fbw_location));
  } else {
    int status;
    int board_id = 0;
    int rev_id = 0;
    /*
     * Could not parse EEPROM. Most likely, it is an old HW without EEPROM.
     * In this case, use board ID to distinguish if it is wedge or 6-pack.
     */
    status = read_ids(&rev_id, &board_id);
    if (verbose) {
      syslog(LOG_INFO, "rev ID %d, board id %d", rev_id, board_id);
    }
    if (status == 0 && board_id != 0xf) {
      return true;
    } else {
      return false;
    }
  }
}
#endif

int read_fan_value(const int fan, const char *device, int *value) {
  char device_name[LARGEST_DEVICE_NAME];
  char output_value[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];

  snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR,device_name);
  return read_device(full_name, value);
}

int write_fan_value(const int fan, const char *device, const int value) {
  char full_name[LARGEST_DEVICE_NAME];
  char device_name[LARGEST_DEVICE_NAME];
  char output_value[LARGEST_DEVICE_NAME];

  snprintf(device_name, LARGEST_DEVICE_NAME, device, fan);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", PWM_DIR, device_name);
  snprintf(output_value, LARGEST_DEVICE_NAME, "%d", value);
  return write_device(full_name, output_value);
}

/* Return fan speed as a percentage of maximum -- not necessarily linear. */

int fan_rpm_to_pct(const struct rpm_to_pct_map *table,
                   const int table_len,
                   int rpm) {
  int i;

  for (i = 0; i < table_len; i++) {
    if (table[i].rpm > rpm) {
      break;
    }
  }

  /*
   * If the fan RPM is lower than the lowest value in the table,
   * we may have a problem -- fans can only go so slow, and it might
   * have stopped.  In this case, we'll return an interpolated
   * percentage, as just returning zero is even more problematic.
   */

  if (i == 0) {
    return (rpm * table[i].pct) / table[i].rpm;
  } else if (i == table_len) { // Fell off the top?
    return table[i - 1].pct;
  }

  // Interpolate the right percentage value:

  int percent_diff = table[i].pct - table[i - 1].pct;
  int rpm_diff = table[i].rpm - table[i - 1].rpm;
  int fan_diff = table[i].rpm - rpm;

  return table[i].pct - (fan_diff * percent_diff / rpm_diff);
}

int fan_speed_okay(const int fan, const int speed, const int slop) {
#if defined(CONFIG_ASUS)
  int real_fan_speed;
#endif
  int front_fan, rear_fan;
  int front_pct, rear_pct;
  int real_fan;
  int okay;

  /*
   * The hardware fan numbers are different from the physical order
   * in the box, so we have to map them:
   */

  real_fan = fan_to_rpm_map[fan];

#if defined(CONFIG_ASUS)
  real_fan_speed = 0;
  read_fan_value(real_fan, FAN_READ_RPM_FORMAT, &real_fan_speed);
  if (fan < 2) {
    front_pct = fan_rpm_to_pct(rpm_front_map, FRONT_MAP_SIZE, real_fan_speed);
  }
  else {
    float scale = fan_scale_speed_map[fan];
    front_pct = fan_rpm_to_pct(rpm_rear_map, REAR_MAP_SIZE, (int)(((float)real_fan_speed) / scale));
  }
  front_fan = real_fan_speed;
#else
  front_fan = 0;
  read_fan_value(real_fan, FAN_READ_RPM_FORMAT, &front_fan);
  front_pct = fan_rpm_to_pct(rpm_front_map, FRONT_MAP_SIZE, front_fan);
#ifdef BACK_TO_BACK_FANS
  rear_fan = 0;
  read_fan_value(real_fan + REAR_FAN_OFFSET, FAN_READ_RPM_FORMAT, &rear_fan);
  rear_pct = fan_rpm_to_pct(rpm_rear_map, REAR_MAP_SIZE, rear_fan);
#endif
#endif

  /*
   * If the fans are broken, the measured rate will be rather
   * different from the requested rate, and we can turn up the
   * rest of the fans to compensate.  The slop is the percentage
   * of error that we'll tolerate.
   *
   * XXX:  I suppose that we should only measure negative values;
   * running too fast isn't really a problem.
   */

#ifdef BACK_TO_BACK_FANS
  okay = (abs(front_pct - speed) * 100 / speed < slop &&
          abs(rear_pct - speed) * 100 / speed < slop);
#else
  okay = (abs(front_pct - speed) * 100 / speed < slop);
#endif

#if defined(CONFIG_SLOPPY_FANS)
   // Don't alert if fan is spinning faster than expected...
   if (!okay && (front_pct > speed)) {
       okay = 1;
   }
#endif

  if (!okay || verbose) {
    syslog(!okay ? LOG_WARNING : LOG_INFO,
#ifdef BACK_TO_BACK_FANS
           "fan %d rear %d (%d%%), front %d (%d%%), expected %d",
#else
           "fan %d %d RPM (%d%%), expected %d",
#endif
           fan,
#ifdef BACK_TO_BACK_FANS
           rear_fan,
           rear_pct,
#endif
           front_fan,
           front_pct,
           speed);
  }

  return okay;
}

#if defined(CONFIG_ASUS)
int averaged_pwm_values[2] = {0, 0};
#endif

/* Set fan speed as a percentage */

int write_fan_speed(const int fan, const int value) {
  /*
   * The hardware fan numbers for pwm control are different from
   * both the physical order in the box, and the mapping for reading
   * the RPMs per fan, above.
   */

  int real_fan = fan_to_pwm_map[fan];

  if (value == 0) {
#if defined(CONFIG_WEDGE100)
    return write_fan_value(real_fan, "fantray%d_pwm", 0);
#elif defined(CONFIG_ASUS)
    return write_fan_value(real_fan, "pwm%d_enable", 0);
#else
    return write_fan_value(real_fan, "pwm%d_en", 0);
#endif
  } else {
    int unit = (value * PWM_UNIT_MAX) / 100;
    int status;

#if defined(CONFIG_ASUS)
    // The KGPE-D16 / KCMA-D8 only has two fan outputs; one controls all 4 pin fans,
    // and the other controls all 3 pin fans.  Average the requested PWM values
    // as a middle-of-the-road thermal strategy...
    if (fan == 0) {
        if (cpu2_installed) {
            averaged_pwm_values[0] = unit;
            return 0;
        }
    }
    else if (fan == 1) {
        if (cpu2_installed) {
          averaged_pwm_values[0] += unit;
          averaged_pwm_values[0] /= 2;
          unit = averaged_pwm_values[0];
       }
       else {
         return 0;
       }
    }
    else if (fan == 2) {
        averaged_pwm_values[1] = unit;
        return 0;
    }
    else if ((fan > 2) && (fan < 7)) {
        averaged_pwm_values[1] += unit;
        return 0;
    }
    else if (fan == 7) {
        averaged_pwm_values[1] += unit;
        averaged_pwm_values[1] /= 6;
        unit = averaged_pwm_values[1];
    }
#endif

#if defined(CONFIG_WEDGE100)
    // Note that PWM for Wedge100 is in 32nds of a cycle
    return write_fan_value(real_fan, "fantray%d_pwm", unit);
#elif defined(CONFIG_ASUS)
    if ((status = write_fan_value(real_fan, "pwm%d_enable", 1)) != 0) {
      return status;
    }
    if ((status = write_fan_value(real_fan, "pwm%d", unit)) != 0) {
      return status;
    }
#else
    if (unit == PWM_UNIT_MAX)
      unit = 0;

    if ((status = write_fan_value(real_fan, "pwm%d_type", 0)) != 0 ||
      (status = write_fan_value(real_fan, "pwm%d_rising", 0)) != 0 ||
      (status = write_fan_value(real_fan, "pwm%d_falling", unit)) != 0 ||
      (status = write_fan_value(real_fan, "pwm%d_en", 1)) != 0) {
      return status;
    }
#endif
  }
}

#if defined(CONFIG_YOSEMITE) || defined(CONFIG_ASUS)
int temp_to_fan_speed(int temp, struct temp_to_pct_map *map, int map_size) {
  int i = map_size - 1;

  while (i > 0 && temp < map[i].temp) {
    --i;
  }
  return map[i].speed;
}
#endif

/* Set up fan LEDs */

int write_fan_led(const int fan, const char *color) {
#if defined(CONFIG_WEDGE100) || defined(CONFIG_WEDGE)
	return write_device(fan_led[fan], color);
#else
        return 0;
#endif
}

int server_shutdown(const char *why) {
  int fan;
  for (fan = 0; fan < total_fans; fan++) {
    write_fan_speed(fan + fan_offset, fan_max);
  }

  syslog(LOG_EMERG, "Shutting down:  %s", why);
#if defined(CONFIG_WEDGE100)
  write_device(USERVER_POWER, "0");
  sleep(5);
  write_device(MAIN_POWER, "0");
#endif
#if defined(CONFIG_WEDGE) && !defined(CONFIG_WEDGE100)
  write_device(GPIO_USERVER_POWER_DIRECTION, "out");
  write_device(GPIO_USERVER_POWER, "0");
  /*
   * Putting T2 in reset generates a non-maskable interrupt to uS,
   * the kernel running on uS might panic depending on its version.
   * sleep 5s here to make sure uS is completely down.
   */
  sleep(5);

  if (write_device(GPIO_T2_POWER_DIRECTION, "out") ||
      write_device(GPIO_T2_POWER, "1")) {
    /*
     * We're here because something has gone badly wrong.  If we
     * didn't manage to shut down the T2, cut power to the whole box,
     * using the PMBus OPERATION register.  This will require a power
     * cycle (removal of both power inputs) to recover.
     */
    syslog(LOG_EMERG, "T2 power off failed;  turning off via ADM1278");
    system("rmmod adm1275");
    system("i2cset -y 12 0x10 0x01 00");
  }
#elif defined(CONFIG_ASUS)
    write_device(MAIN_POWER_OFF_DIRECTION, "out");
    write_device(MAIN_POWER_OFF_CTL, "0");
    sleep(1);
    write_device(MAIN_POWER_OFF_CTL, "1");
#else
  // TODO(7088822):  try throttling, then shutting down server.
  syslog(LOG_EMERG, "Need to implement actual shutdown!\n");
#endif

#if !defined(CONFIG_ASUS)
  /*
   * We have to stop the watchdog, or the system will be automatically
   * rebooted some seconds after fand exits (and stops kicking the
   * watchdog).
   */

  stop_watchdog();

  sleep(2);
  exit(2);
#endif
}

/* Gracefully shut down on receipt of a signal */

void fand_interrupt(int sig)
{
  int fan;
  for (fan = 0; fan < total_fans; fan++) {
    write_fan_speed(fan + fan_offset, fan_max);
  }

  syslog(LOG_WARNING, "Shutting down fand on signal %s", strsignal(sig));
  if (sig == SIGUSR1) {
    stop_watchdog();
  }
  exit(3);
}

int main(int argc, char **argv) {
  /* Sensor values */

#if defined(CONFIG_WEDGE)
  int intake_temp;
  int exhaust_temp;
  int switch_temp;
  int userver_temp;
#elif defined(CONFIG_ASUS)
  int chipset_temp;
  int cpu1_temp;
  int cpu2_temp;
  int cpu2_vcore_mv;
#else
  float intake_temp;
  float exhaust_temp;
  float userver_temp;
#endif

  int fan_speed = fan_high;
  int bad_reads = 0;
  int fan_failure = 0;
  int fan_speed_changes = 0;
  int old_speed;

  int cpu_fan_speed = fan_high;
  int chassis_fan_speed = fan_high;
  int cpu_fan_speed_changes = 0;
  int chassis_fan_speed_changes = 0;
  int cpu_old_speed = fan_high;
  int chassis_old_speed = fan_high;

  int fan_bad[FANS];
  int fan;

  unsigned log_count = 0; // How many times have we logged our temps?
  int opt;
  int prev_fans_bad = 0;

  struct sigaction sa;

  sa.sa_handler = fand_interrupt;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);

  // Start writing to syslog as early as possible for diag purposes.

  openlog("fand", LOG_CONS, LOG_DAEMON);

#if defined(CONFIG_WEDGE) && !defined(CONFIG_WEDGE100)
  if (is_two_fan_board(false)) {
    /* Alternate, two fan configuration */
    total_fans = 2;
    fan_offset = 2; /* fan 3 is the first */

    fan_low = SIXPACK_FAN_LOW;
    fan_medium = SIXPACK_FAN_MEDIUM;
    fan_high = SIXPACK_FAN_HIGH;
    fan_max = SIXPACK_FAN_MAX;
    fan_speed = fan_high;
  }
#endif

  while ((opt = getopt(argc, argv, "l:m:h:b:t:r:v")) != -1) {
    switch (opt) {
    case 'l':
      fan_low = atoi(optarg);
      break;
    case 'm':
      fan_medium = atoi(optarg);
      break;
    case 'h':
      fan_high = atoi(optarg);
      break;
    case 'b':
      temp_bottom = INTERNAL_TEMPS(atoi(optarg));
      break;
    case 't':
      temp_top = INTERNAL_TEMPS(atoi(optarg));
      break;
    case 'r':
      report_temp = atoi(optarg);
      break;
    case 'v':
      verbose = true;
      break;
    default:
      usage();
      break;
    }
  }

  if (optind > argc) {
    usage();
  }

  if (temp_bottom > temp_top) {
    fprintf(stderr,
            "Should temp-bottom (%d) be higher than "
            "temp-top (%d)?  Starting anyway.\n",
            EXTERNAL_TEMPS(temp_bottom),
            EXTERNAL_TEMPS(temp_top));
  }

  if (fan_low > fan_medium || fan_low > fan_high || fan_medium > fan_high) {
    fprintf(stderr,
            "fan RPMs not strictly increasing "
            "-- %d, %d, %d, starting anyway\n",
            fan_low,
            fan_medium,
            fan_high);
  }

  daemon(1, 0);

  if (verbose) {
    syslog(LOG_DEBUG, "Starting up;  system should have %d fans.",
           total_fans);
  }

  for (fan = 0; fan < total_fans; fan++) {
    if (!cpu2_installed && (fan == 1)) {
      continue;
    }
    fan_bad[fan] = 0;
    write_fan_speed(fan + fan_offset, fan_speed);
    write_fan_led(fan + fan_offset, FAN_LED_BLUE);
  }

#if defined(CONFIG_YOSEMITE)
  /* Ensure that we can read from sensors before proceeding. */

  int found = 0;
  userver_temp = 100;
  while (!found) {
    for (int node = 1; node <= TOTAL_1S_SERVERS && !found; node++) {
      if (!yosemite_sensor_read(node, BIC_SENSOR_SOC_THERM_MARGIN,
                               &userver_temp) &&
          userver_temp < 0) {
        syslog(LOG_DEBUG, "SOC_THERM_MARGIN first valid read of %f.",
               userver_temp);
        found = 1;
      }
      sleep(5);
    }
    // XXX:  Will it ever be a problem that we don't exit this until
    //       we see a valid value?
  }
#endif

  /* Start watchdog in manual mode */
  start_watchdog(0);

  /* Set watchdog to persistent mode so timer expiry will happen independent
   * of this process's liveliness. */
  set_persistent_watchdog(WATCHDOG_SET_PERSISTENT);

  sleep(5);  /* Give the fans time to come up to speed */

  while (1) {
    int max_temp;
    old_speed = fan_speed;
    cpu_old_speed = cpu_fan_speed;
    chassis_old_speed = chassis_fan_speed;

    /* Read sensors */

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
    read_temp(INTAKE_TEMP_DEVICE, &intake_temp);
    read_temp(EXHAUST_TEMP_DEVICE, &exhaust_temp);
    read_temp(CHIP_TEMP_DEVICE, &switch_temp);
    read_temp(USERVER_TEMP_DEVICE, &userver_temp);

    /*
     * uServer can be powered down, but all of the rest of the sensors
     * should be readable at any time.
     */

    if ((intake_temp == BAD_TEMP || exhaust_temp == BAD_TEMP ||
         switch_temp == BAD_TEMP)) {
      bad_reads++;
    }
#elif defined(CONFIG_ASUS)
    read_temp(CHIPSET_TEMP_DEVICE, &chipset_temp);
    read_temp(CPU1_TEMP_DEVICE, &cpu1_temp);
    read_temp(CPU2_TEMP_DEVICE, &cpu2_temp);
    read_voltage(CPU2_VCORE_DEVICE, &cpu2_vcore_mv);

    /*
     * uServer can be powered down, but all of the rest of the sensors
     * should be readable at any time.
     */

    if ((chipset_temp == BAD_TEMP) || (cpu1_temp == BAD_TEMP) ||
         (cpu2_temp == BAD_TEMP) || (cpu2_vcore_mv == BAD_VOLTAGE)) {
      bad_reads++;
    }
    else {
      bad_reads = 0;	// Only care about continuous bad reads
    }

    if (cpu2_vcore_mv == 0) {
      cpu2_installed = 0;
    }
    else {
      cpu2_installed = 1;
    }
#else
    intake_temp = exhaust_temp = userver_temp = BAD_TEMP;
    if (yosemite_sensor_read(FRU_SPB, SP_SENSOR_INLET_TEMP, &intake_temp) ||
        yosemite_sensor_read(FRU_SPB, SP_SENSOR_OUTLET_TEMP, &exhaust_temp))
      bad_reads++;

    /*
     * There are a number of 1S servers;  any or all of them
     * could be powered off and returning no values.  Ignore these
     * invalid values.
     */
    for (int node = 1; node <= TOTAL_1S_SERVERS; node++) {
      float new_temp;
      if (!yosemite_sensor_read(node, BIC_SENSOR_SOC_THERM_MARGIN,
			        &new_temp)) {
        if (userver_temp < new_temp) {
          userver_temp = new_temp;
        }
      }

      // Since the yosemite_sensor_read() times out after 8secs, keep WDT from expiring
      kick_watchdog();
    }
#endif

    if (bad_reads > BAD_READ_THRESHOLD) {
      server_shutdown("Some sensors couldn't be read");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }

#if defined(CONFIG_ASUS)
    if (log_count++ % report_temp == 0) {
      syslog(LOG_DEBUG,
             "Temp chipset %f, CPU1 %f, CPU2 %f, "
             "CPU fan speed %d, CPU speed changes %d "
             "chassis fan speed %d, chassis speed changes %d",
             EXTERNAL_TEMPS((float)chipset_temp),
             EXTERNAL_TEMPS((float)cpu1_temp),
             EXTERNAL_TEMPS((float)cpu2_temp),
             cpu_fan_speed,
             cpu_fan_speed_changes,
             chassis_fan_speed,
             chassis_fan_speed_changes);
    }

    /* Protection heuristics */

    if (chipset_temp > CHIPSET_LIMIT) {
      server_shutdown("Chipset temp limit reached");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }

    if (cpu1_temp > CPU_LIMIT) {
      server_shutdown("CPU1 temp limit reached");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }

    if (cpu2_installed) {
      if (cpu2_temp > CPU_LIMIT) {
        server_shutdown("CPU2 temp limit reached");
        bad_reads = 0;
        kick_watchdog();
        continue;
      }
    }
#else
    if (log_count++ % report_temp == 0) {
      syslog(LOG_DEBUG,
#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
             "Temp intake %d, t2 %d, "
             " userver %d, exhaust %d, "
             "fan speed %d, speed changes %d",
#else
             "Temp intake %f, max server %f, exhaust %f, "
             "fan speed %d, speed changes %d",
#endif
             intake_temp,
#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
             switch_temp,
#endif
             userver_temp,
             exhaust_temp,
             fan_speed,
             fan_speed_changes);
    }

    /* Protection heuristics */

    if (intake_temp > INTAKE_LIMIT) {
      server_shutdown("Intake temp limit reached");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
    if (switch_temp > SWITCH_LIMIT) {
      server_shutdown("T2 temp limit reached");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }
#endif

    if (userver_temp + USERVER_TEMP_FUDGE > USERVER_LIMIT) {
      server_shutdown("uServer temp limit reached");
      bad_reads = 0;
      kick_watchdog();
      continue;
    }
#endif

    /*
     * Calculate change needed -- we should eventually
     * do something more sophisticated, like PID.
     *
     * We should use the intake temperature to adjust this
     * as well.
     */

#if defined(CONFIG_YOSEMITE)
    /* Use tables to lookup the new fan speed for Yosemite. */

    int intake_speed = temp_to_fan_speed(intake_temp, intake_map,
                                         INTAKE_MAP_SIZE);
    int cpu_speed = temp_to_fan_speed(userver_temp, cpu_map, CPU_MAP_SIZE);

    if (fan_speed == fan_max && fan_failure != 0) {
      /* Don't change a thing */
    } else if (intake_speed > cpu_speed) {
      fan_speed = intake_speed;
    } else {
      fan_speed = cpu_speed;
    }
#elif defined(CONFIG_ASUS)
    int max_cpu_temp = cpu1_temp + USERVER_TEMP_FUDGE;

    if (cpu2_installed) {
      if (cpu2_temp + USERVER_TEMP_FUDGE > max_cpu_temp) {
        max_cpu_temp = cpu2_temp + USERVER_TEMP_FUDGE;
      }
    }

    int chassis_speed = temp_to_fan_speed(EXTERNAL_TEMPS(chipset_temp), chipset_map,
                                         CHIPSET_MAP_SIZE);
    int cpu_speed = temp_to_fan_speed(EXTERNAL_TEMPS(max_cpu_temp), cpu_map, CPU_MAP_SIZE);

    if (cpu_fan_speed == fan_max && fan_failure != 0) {
      /* Don't change a thing */
    } else {
      cpu_fan_speed = cpu_speed;
    }
    if (chassis_fan_speed == fan_max && fan_failure != 0) {
      /* Don't change a thing */
    } else {
      chassis_fan_speed = chassis_speed;
    }
#else
    /* Other systems use a simpler built-in table to determine fan speed. */
    if (switch_temp > userver_temp + USERVER_TEMP_FUDGE) {
      max_temp = switch_temp;
    } else {
      max_temp = userver_temp + USERVER_TEMP_FUDGE;
    }

    /*
     * If recovering from a fan problem, spin down fans gradually in case
     * temperatures are still high. Gradual spin down also reduces wear on
     * the fans.
     */
    if (fan_speed == fan_max) {
      if (fan_failure == 0) {
        fan_speed = fan_high;
      }
    } else if (fan_speed == fan_high) {
      if (max_temp + COOLDOWN_SLOP < temp_top) {
        fan_speed = fan_medium;
      }
    } else if (fan_speed == fan_medium) {
      if (max_temp > temp_top) {
        fan_speed = fan_high;
      } else if (max_temp + COOLDOWN_SLOP < temp_bottom) {
        fan_speed = fan_low;
      }
    } else {/* low */
      if (max_temp > temp_bottom) {
        fan_speed = fan_medium;
      }
    }
#endif

    /*
     * Update fans only if there are no failed ones. If any fans failed
     * earlier, all remaining fans should continue to run at max speed.
     */

#if defined(CONFIG_ASUS)
    if (fan_failure == 0 && cpu_fan_speed != cpu_old_speed) {
      syslog(LOG_NOTICE,
             "CPU fan speed changing from %d to %d",
             cpu_old_speed,
             cpu_fan_speed);
      cpu_fan_speed_changes++;
    }
    if (fan_failure == 0 && chassis_fan_speed != chassis_old_speed) {
      syslog(LOG_NOTICE,
             "Chassis fan speed changing from %d to %d",
             chassis_old_speed,
             chassis_fan_speed);
      chassis_fan_speed_changes++;
    }
    for (fan = 0; fan < 2; fan++) {
      if (!cpu2_installed && (fan == 1)) {
        continue;
      }
      write_fan_speed(fan + fan_offset, cpu_fan_speed);
    }
    for (fan = 2; fan < total_fans; fan++) {
      write_fan_speed(fan + fan_offset, chassis_fan_speed);
    }
#else
    if (fan_failure == 0 && fan_speed != old_speed) {
      syslog(LOG_NOTICE,
             "Fan speed changing from %d to %d",
             old_speed,
             fan_speed);
      fan_speed_changes++;
      for (fan = 0; fan < total_fans; fan++) {
        if (!cpu2_installed && (fan == 1)) {
          continue;
        }
        write_fan_speed(fan + fan_offset, fan_speed);
      }
    }
#endif

    /*
     * Wait for some change.  Typical I2C temperature sensors
     * only provide a new value every second and a half, so
     * checking again more quickly than that is a waste.
     *
     * We also have to wait for the fan changes to take effect
     * before measuring them.
     */

    sleep(5);

    /* Check fan RPMs */

    for (fan = 0; fan < total_fans; fan++) {
      if (!cpu2_installed && (fan == 1)) {
        continue;
      }
      if (fan_population_map[fan] == 0) {
        continue;
      }
      /*
       * Make sure that we're within some percentage
       * of the requested speed.
       */
#if defined(CONFIG_ASUS)
      int desired_fan_speed = cpu_fan_speed;
      if (fan > 1) {
        desired_fan_speed = chassis_fan_speed;
      }
      if (fan_speed_okay(fan + fan_offset, desired_fan_speed, FAN_FAILURE_OFFSET)) {
#else
      if (fan_speed_okay(fan + fan_offset, fan_speed, FAN_FAILURE_OFFSET)) {
#endif
        if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
          write_fan_led(fan + fan_offset, FAN_LED_BLUE);
          syslog(LOG_CRIT,
                 "Fan %d has recovered",
                 fan);
        }
        fan_bad[fan] = 0;
      } else {
        fan_bad[fan]++;
      }
    }

    fan_failure = 0;
    for (fan = 0; fan < total_fans; fan++) {
      if (!cpu2_installed && (fan == 1)) {
        continue;
      }
      if (fan_population_map[fan] == 0) {
        continue;
      }
      if (fan_bad[fan] > FAN_FAILURE_THRESHOLD) {
        fan_failure++;
        write_fan_led(fan + fan_offset, FAN_LED_RED);
      }
    }

    if (fan_failure > 0) {
      if (prev_fans_bad != fan_failure) {
        syslog(LOG_CRIT, "%d fans failed", fan_failure);
      }

      /*
       * If fans are bad, we need to blast all of the
       * fans at 100%;  we don't bother to turn off
       * the bad fans, in case they are all that is left.
       *
       * Note that we have a temporary bug with setting fans to
       * 100% so we only do fan_max = 99%.
       */

      fan_speed = fan_max;
      cpu_fan_speed = fan_max;
      chassis_fan_speed = fan_max;
      for (fan = 0; fan < total_fans; fan++) {
        if (!cpu2_installed && (fan == 1)) {
          continue;
        }
        if (fan_population_map[fan] == 0) {
          continue;
        }
        write_fan_speed(fan + fan_offset, fan_max);
      }

#if defined(CONFIG_WEDGE) || defined(CONFIG_WEDGE100)
      /*
       * On Wedge, we want to shut down everything if none of the fans
       * are visible, since there isn't automatic protection to shut
       * off the server or switch chip.  On other platforms, the CPUs
       * generating the heat will automatically turn off, so this is
       * unnecessary.
       */

      if (fan_failure == total_fans) {
        int count = 0;
        for (fan = 0; fan < total_fans; fan++) {
          if (!cpu2_installed && (fan == 1)) {
            continue;
          }
          if (fan_population_map[fan] == 0) {
            continue;
          }
          if (fan_bad[fan] > FAN_SHUTDOWN_THRESHOLD)
            count++;
        }
        if (count == total_fans) {
          server_shutdown("all fans are bad for more than 12 cycles");
          bad_reads = 0;
          kick_watchdog();
          continue;
        }
      }
#endif

      /*
       * Fans can be hot swapped and replaced; in which case the fan daemon
       * will automatically detect the new fan and (assuming the new fan isn't
       * itself faulty), automatically readjust the speeds for all fans down
       * to a more suitable rpm. The fan daemon does not need to be restarted.
       */
    }

    /* Suppress multiple warnings for similar number of fan failures. */
    prev_fans_bad = fan_failure;

    /* if everything is fine, restart the watchdog countdown. If this process
     * is terminated, the persistent watchdog setting will cause the system
     * to reboot after the watchdog timeout. */
    kick_watchdog();
  }
}
