//
// Created by cyber on 29.02.20.
//

#ifndef LVGL_REPPANEL_SETTINGS_H
#define LVGL_REPPANEL_SETTINGS_H

#define MAX_NUM_TOOLS   5

static char* wifi_ssid[20];
static char* wifi_pass[20];

static int temp_unit = 0;   // 0=Celsius, 1=Fahrenheit

static char *tool_names_map[] = {"E0", "E1", "E2", "E3", "E4", "E5", "E6"};

static char *bed_tmps_active = "0°C\n40°C\n60°C\n100°C";
static char *bed_tmps_standby = "0°C\n40°C\n60°C\n100°C";
static char *extruder_tmps_active = "0°C\n100°C\n160°C\n200°C";
static char *extruder_tmps_standby = "0°C\n100°C\n160°C\n200°C";

static int toolstates[MAX_NUM_TOOLS];       // 0=off, 1=standby, 2=active, 3=fault - Storage for incoming data

#endif //LVGL_REPPANEL_SETTINGS_H
