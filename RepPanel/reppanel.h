//
// Created by cyber on 16.02.20.
//

#ifndef LVGL_REPPANEL_H
#define LVGL_REPPANEL_H

#include <stdint.h>
#include "reppanel_helper.h"
#include "reppanel_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BTN_BED_TMP_ACTIVE 0
#define BTN_BED_TMP_STANDBY 1
#define BTN_TOOL_TMP_ACTIVE 2
#define BTN_TOOL_TMP_STANDBY 3

#define VERSION_STR             "v0.1.0"

#define NUM_TEMPS_BUFF      15
#define MAX_FILA_NAME_LEN   64
#define MAX_TOOL_NAME_LEN   32
#define MAX_LEN_STR_FILAMENT_LIST   1024*2
#define MAX_NUM_MACROS      64
#define MAX_NUM_JOBS        MAX_NUM_MACROS
#define MAX_LEN_MACRO_STR   256


#define TREE_EMPTY_ELEM     -1
#define TREE_FOLDER_ELEM    0
#define TREE_FILE_ELEM      1

void rep_panel_ui_create();
void update_rep_panel_conn_status();
void draw_header(lv_obj_t *parent_screen);
void display_jobstatus();

extern lv_obj_t *jobstatus_scr;
extern uint8_t reppanel_conn_status;

extern lv_obj_t *cont_header;
extern lv_obj_t *process_scr;              // screen for the process settings
extern lv_obj_t *mainmenu_scr;              // screen for the main_menue

extern lv_obj_t *label_status;
extern lv_obj_t *label_chamber_temp;
extern lv_obj_t *label_bed_temp;
extern lv_obj_t *label_tool_temp;
extern lv_obj_t *label_progress_percent;
extern lv_obj_t *main_menu_button;
extern lv_obj_t *console_button;
extern lv_obj_t *label_extruder_name;

extern lv_obj_t *btn_bed_temp_active;
extern lv_obj_t *btn_bed_temp_standby;
extern lv_obj_t *btn_tool_temp_active;
extern lv_obj_t *btn_tool_temp_standby;

extern lv_obj_t *label_bed_temp_active;
extern lv_obj_t *label_bed_temp_standby;
extern lv_obj_t *label_tool_temp_active;
extern lv_obj_t *label_tool_temp_standby;

extern lv_obj_t *button_tool_filament;
extern lv_obj_t *label_sig_strength;
extern lv_obj_t *label_connection_status;

extern double reprap_extruder_amounts[NUM_TEMPS_BUFF];
extern double reprap_extruder_feedrates[NUM_TEMPS_BUFF];
extern char *reprap_macro_names[MAX_NUM_MACROS];    // pointers to macro file names. Use malloc & free!

typedef struct {
    double temps_standby[NUM_TEMPS_BUFF];
    double temps_active[NUM_TEMPS_BUFF];
} reprap_tool_poss_temps_t;

typedef struct {
    double temps_standby[NUM_TEMPS_BUFF];
    double temps_active[NUM_TEMPS_BUFF];
} reprap_bed_poss_temps_t;

typedef struct {
    double x;
    double y;
    double z;
    bool x_homed;
    bool y_homed;
    bool z_homed;
} reprap_axes_t;

typedef struct {
    int number;
    char name[MAX_TOOL_NAME_LEN];
    int fans;
    char filament[MAX_FILA_NAME_LEN];
    int heater_indx;                    // only support one heater per tool for now
    double temp_buff[NUM_TEMPS_BUFF];   // Temp buffer contains temperature history of heaters
    int temp_hist_curr_pos;             // Pointers to current position within the temp buffer
    double active_temp;
    double standby_temp;
} reprap_tool_t;

typedef struct {
    double temp_buff[NUM_TEMPS_BUFF];   // Temp buffer contains temperature history of heater
    int temp_hist_curr_pos;             // Pointers to current position within the temp buffer
    int heater_indx;
    double active_temp;
    double standby_temp;
} reprap_bed_t;

typedef struct {
    char *name;
    char *last_mod;
    char *dir;
    char *generator;        // slicer engine
    int size;
    double height;
    double layer_height;
    int print_time;
    int sim_print_time;
} reprap_job_t;

typedef struct {
    char *name;
    char *last_mod;
    char *dir;
    int size;
} reprap_macro_t;

typedef struct {
    void *element;  // reprap_macro_t or reprap_job_t
    int type;       // TREE_FOLDER_ELEM, TREE_FILE_ELEM
} file_tree_elem_t;

extern file_tree_elem_t reprap_jobs[MAX_NUM_JOBS];
extern file_tree_elem_t reprap_macros[MAX_NUM_MACROS];

extern reprap_axes_t reprap_axes;
extern reprap_tool_t reprap_tools[MAX_NUM_TOOLS];
extern reprap_bed_t reprap_bed;
extern reprap_tool_poss_temps_t reprap_tool_poss_temps;
extern reprap_bed_poss_temps_t reprap_bed_poss_temps;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //LVGL_REPPANEL_H
