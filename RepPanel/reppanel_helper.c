//
// Created by cyber on 29.02.20.
//

#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/src/lv_objx/lv_btn.h>
#include <lvgl/src/lv_objx/lv_label.h>
#include <stdio.h>
#include <lvgl/lvgl.h>
#include <RepPanel/custom_themes/lv_theme_rep_panel_light.h>
#include <lvgl/src/lv_core/lv_style.h>
#include "reppanel_settings.h"
#include "reppanel.h"
#include "reppanel_console.h"

lv_obj_t *mbox_msg;

char get_temp_unit() {
    if (temp_unit == 0) {
        return 'C';
    } else {
        return 'F';
    }
}

void init_reprap_buffers() {
    for (int i = 0; i < MAX_NUM_TOOLS; i++) {
        reprap_extruder_amounts[i] = -1;
        reprap_extruder_feedrates[i] = -1;
    }
    for (int i = 0; i < MAX_NUM_MACROS; i++) {
        reprap_macro_names[i] = NULL;
    }
    for (int i = 0; i < NUM_TEMPS_BUFF; i++) {
        reprap_bed_poss_temps.temps_active[i] = -1;
        reprap_bed_poss_temps.temps_standby[i] = -1;

        reprap_tool_poss_temps.temps_active[i] = -1;
        reprap_tool_poss_temps.temps_standby[i] = -1;
    }
    for (int i = 0; i < MAX_CONSOLE_ENTRY_COUNT; i++) {
        console_enties[i] = (console_entry_t) {NULL, NULL, CONSOLE_TYPE_EMPTY};
    }
    for (int i = 0; i < MAX_NUM_MACROS; i++) {
        reprap_macros[i].type = TREE_EMPTY_ELEM;
        reprap_macros[i].element = NULL;
    }
    for (int i = 0; i < MAX_NUM_JOBS; i++) {
        reprap_jobs[i].type = TREE_EMPTY_ELEM;
        reprap_jobs[i].element = NULL;
    }
}

lv_obj_t *create_button(lv_obj_t *parent, lv_obj_t *button_pnt, char *text, void *event_handler) {
    button_pnt = lv_btn_create(parent, NULL);
    lv_btn_set_fit(button_pnt, LV_FIT_TIGHT);
    lv_obj_set_event_cb(button_pnt, event_handler);
    lv_obj_align(button_pnt, parent, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label = lv_label_create(button_pnt, NULL);
    lv_label_set_text(label, text);
    return button_pnt;
}

static void _close_msg_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        lv_obj_del_async(mbox_msg);
    }
}

void reppanel_disp_msg(char *msg_txt) {
    static const char *btns[] = {"Close", ""};
    mbox_msg = lv_mbox_create(lv_layer_top(), NULL);
    lv_mbox_set_text(mbox_msg, msg_txt);
    lv_mbox_add_btns(mbox_msg, btns);
    lv_obj_set_width(mbox_msg, 300);
    lv_obj_set_event_cb(mbox_msg, _close_msg_event_handler);
    lv_obj_align(mbox_msg, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
}

void RepPanelLogE(char *tag, char *msg){
    printf(tag, "%s", msg);
}
void RepPanelLogW(char *tag, char *msg){
    printf(tag, "%s", msg);
}
void RepPanelLogI(char *tag, char *msg){
    printf(tag, "%s", msg);
}
void RepPanelLogD(char *tag, char *msg){
    printf(tag, "%s", msg);
}
void RepPanelLogV(char *tag, char *msg){
    printf(tag, "%s", msg);
}