#include <lvgl/src/lv_objx/lv_page.h>
#include <lvgl/src/lv_objx/lv_ddlist.h>
#include <lvgl/lvgl.h>
#include "custom_themes/lv_theme_rep_panel_dark.h"
#include <stdio.h>
#include "reppanel_machine.h"
#include "reppanel_helper.h"
#include "reppanel_request.h"
#include "reppanel.h"

#define TAG     "Machine"

reprap_axes_t reprap_axes;
reprap_params_t reprap_params;

static lv_style_t not_homed_style, homed_style;
static const char * yes_no_btns[] ={"Yes", "No", ""};
lv_obj_t *mbox_mbl, *mbox_tbl;

#define AWAY_BTN    0
#define CLOSER_BTN  1

#ifdef CONFIG_REPPANEL_ENABLE_LIGHT_CONTROL
#define LIGHTNING_CMD_ON "M42 P2 S1"
#define LIGHTNING_CMD_HALF "M42 P2 S0.5"
#define LIGHTNING_CMD_OFF "M42 P2 S0"
#endif

lv_obj_t *machine_page;
lv_obj_t *btnm_height;
lv_obj_t *btn_closer;
lv_obj_t *btn_away;
lv_obj_t *cont_heigh_adj_diag, *label_z_pos_cali;
lv_obj_t *btn_home_all, *btn_home_y, *btn_home_x, *btn_home_z;
lv_obj_t *btn_baby_closer, *btn_baby_away, *label_babystep;
lv_obj_t *btn_power, *label_power;
lv_obj_t *btn_fan_off, *label_fan, *slider;

#ifdef CONFIG_REPPANEL_ENABLE_LIGHT_CONTROL
lv_obj_t *btn_light_off, *btn_light_half, *btn_light_on;
#endif


static void home_all_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("G28");
    }
}

static void home_x_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("G28 X");
    }
}

static void home_y_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("G28 Y");
    }
}

static void home_z_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("G28 Z");
    }
}

#ifdef CONFIG_REPPANEL_ENABLE_LIGHT_CONTROL
static void _light_off_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode(LIGHTNING_CMD_OFF);
    }
}

static void _light_half_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode(LIGHTNING_CMD_HALF);
    }
}

static void _light_on_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode(LIGHTNING_CMD_ON);
    }
}
#endif

#ifdef ENABLE_POWER_CONTROL
static void power_toggle_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        if (reprap_params.power) {
            // TODO: ask first before powering off
        }
        reprap_send_gcode(reprap_params.power ? "M81" : "M80");
    }
}
#endif

static void fan_off_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("M106 S0");
    }
}

static void slider_event_cb(lv_obj_t *slider, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        static char buf[11]; /* max 10 bytes for number plus 1 null terminating byte */
        snprintf(buf, 11, "M106 S%.2f", (lv_slider_get_value(slider) / 100.));
        reprap_send_gcode(buf);
    }
}

static void babystep_away(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("M290 S0.01");
    }
}

static void babystep_closer(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        reprap_send_gcode("M290 S-0.01");
    }
}

static void tbl_event_handler(lv_obj_t * obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED && obj == mbox_tbl) {
        if (strcmp(lv_mbox_get_active_btn_text(obj), "Yes") == 0) {
            printf("True Bed Leveling");
            reprap_send_gcode("G32");
        }
        lv_mbox_start_auto_close(mbox_tbl, 0);
    } else if (event == LV_EVENT_DELETE && obj == mbox_tbl) {
        /* Delete the parent modal background */
        lv_obj_del_async(mbox_tbl);
        mbox_tbl = NULL; /* happens before object is actually deleted! */
    }
}

static void true_bed_leveling_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        mbox_tbl = lv_mbox_create(lv_layer_top(), NULL);
        lv_mbox_set_text(mbox_tbl, "Do you really want to start true bed leveling now?");
        lv_mbox_add_btns(mbox_tbl, yes_no_btns);
        lv_obj_set_width(mbox_tbl, 350);
        lv_obj_set_event_cb(mbox_tbl, tbl_event_handler);
        lv_obj_align(mbox_tbl, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
    }
}

static void mbl_event_handler(lv_obj_t * obj, lv_event_t event) {
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_mbox_get_active_btn_text(obj), "Yes") == 0) {
            printf("Mesh Bed Leveling");
            reprap_send_gcode("G29");
        }
        lv_mbox_start_auto_close(mbox_mbl, 0);
    } else if (event == LV_EVENT_DELETE && obj == mbox_mbl) {
        /* Delete the parent modal background */
        lv_obj_del_async(mbox_mbl);
        mbox_mbl = NULL; /* happens before object is actually deleted! */
    }
}

static void mesh_bed_leveling_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        mbox_mbl = lv_mbox_create(lv_layer_top(), NULL);
        lv_mbox_set_text(mbox_mbl, "Do you really want to start mesh bed leveling now?");
        lv_mbox_add_btns(mbox_mbl, yes_no_btns);
        lv_obj_set_width(mbox_mbl, 350);
        lv_obj_set_event_cb(mbox_mbl, mbl_event_handler);
        lv_obj_align(mbox_mbl, NULL, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
    }
}

void update_ui_machine() {
    if (label_z_pos_cali) lv_label_set_text_fmt(label_z_pos_cali, "%.02f mm", reprap_axes.axes[2]);
    if (visible_screen != REPPANEL_MACHINE_SCREEN) return;

    if (btn_home_x && machine_page) {
        if (reprap_axes.homed[0])
            lv_btn_set_style(btn_home_x, LV_BTN_STYLE_REL, &homed_style);
        else
            lv_btn_set_style(btn_home_x, LV_BTN_STYLE_REL, &not_homed_style);

        if (reprap_axes.homed[1])
            lv_btn_set_style(btn_home_y, LV_BTN_STYLE_REL, &homed_style);
        else
            lv_btn_set_style(btn_home_y, LV_BTN_STYLE_REL, &not_homed_style);

        if (reprap_axes.homed[2])
            lv_btn_set_style(btn_home_z, LV_BTN_STYLE_REL, &homed_style);
        else
            lv_btn_set_style(btn_home_z, LV_BTN_STYLE_REL, &not_homed_style);

        if (reprap_axes.homed[0] && reprap_axes.homed[1] && reprap_axes.homed[2])
            lv_btn_set_style(btn_home_all, LV_BTN_STYLE_REL, &homed_style);
        else
            lv_btn_set_style(btn_home_all, LV_BTN_STYLE_REL, &not_homed_style);
    }
    if (label_babystep && machine_page) {
        lv_label_set_text_fmt(label_babystep, "Babystep:\n%.2fmm", reprap_axes.babystep[2]);
    }
#ifdef ENABLE_POWER_CONTROL
    if (btn_power && machine_page) {
        if (reprap_params.power) {
            lv_btn_set_style(btn_power, LV_BTN_STYLE_REL, &homed_style);
            lv_label_set_text(label_power, "On");
        } else {
            lv_btn_set_style(btn_power, LV_BTN_STYLE_REL, &not_homed_style);
            lv_label_set_text(label_power, "Off");
        }
    }
#endif
    if (label_fan && machine_page) {
        lv_label_set_text_fmt(label_fan, " %u%% ", reprap_params.fan);
        lv_slider_set_value(slider, reprap_params.fan, LV_ANIM_ON);
    }
}

void draw_machine(lv_obj_t *parent_screen) {
    machine_page = lv_page_create(parent_screen, NULL);
    lv_obj_set_size(machine_page, lv_disp_get_hor_res(NULL),
                    lv_disp_get_ver_res(NULL) - (lv_obj_get_height(cont_header) + 5));
    lv_page_set_scrl_fit2(machine_page, LV_FIT_FILL, LV_FIT_FILL);
    lv_page_set_scrl_layout(machine_page, LV_LAYOUT_COL_M);

    lv_obj_t *home_cont = lv_cont_create(machine_page, NULL);
    lv_cont_set_layout(home_cont, LV_LAYOUT_ROW_M);
    lv_cont_set_fit(home_cont, LV_FIT_TIGHT);
    lv_obj_t *label_home = lv_label_create(home_cont, NULL);
    lv_label_set_text(label_home, "Home:");
    btn_home_all = create_button(home_cont, btn_home_all, "  All Axis  ", home_all_event);
    btn_home_x = create_button(home_cont, btn_home_x, " X ", home_x_event);
    btn_home_y = create_button(home_cont, btn_home_y, " Y ", home_y_event);
    btn_home_z = create_button(home_cont, btn_home_z, " Z ", home_z_event);

    lv_style_copy(&not_homed_style, lv_btn_get_style(btn_home_x, LV_BTN_STYLE_REL));
    lv_style_copy(&homed_style, lv_btn_get_style(btn_home_x, LV_BTN_STYLE_REL));
    homed_style.body.main_color = REP_PANEL_DARK_ACCENT;
    homed_style.body.grad_color = REP_PANEL_DARK_ACCENT;
    homed_style.text.color = REP_PANEL_DARK;

    lv_obj_t *cont_cali = lv_cont_create(machine_page, NULL);
    lv_cont_set_fit(cont_cali, LV_FIT_TIGHT);
    lv_cont_set_layout(cont_cali, LV_LAYOUT_ROW_M);

    static lv_obj_t *true_bed_leveling_butn;
    create_button(cont_cali, true_bed_leveling_butn, "True Bed Leveling", true_bed_leveling_event);

    static lv_obj_t *mesh_bed_leveling_butn;
    create_button(cont_cali, mesh_bed_leveling_butn, "Mesh Bed Leveling", mesh_bed_leveling_event);

#ifdef ENABLE_POWER_CONTROL
    lv_obj_t *power_cont = lv_cont_create(machine_page, NULL);
    lv_cont_set_layout(power_cont, LV_LAYOUT_ROW_M);
    lv_cont_set_fit(power_cont, LV_FIT_TIGHT);
    lv_obj_t *label_power_header = lv_label_create(power_cont, NULL);
    lv_label_set_text(label_power_header, "Power:");

    btn_power = lv_btn_create(power_cont, NULL);
    lv_btn_set_fit(btn_power, LV_FIT_TIGHT);
    lv_obj_set_event_cb(btn_power, power_toggle_event);
    lv_obj_align(btn_power, power_cont, LV_ALIGN_CENTER, 0, 0);
    label_power = lv_label_create(btn_power, NULL);
    lv_label_set_text(label_power, reprap_params.power ? "On" : "Off");
#endif

#ifdef CONFIG_REPPANEL_ENABLE_LIGHT_CONTROL
    lv_obj_t *label_light = lv_label_create(power_cont, NULL);
    lv_label_set_text(label_light, "Light:");
    btn_light_on = create_button(power_cont, btn_light_on, "On", _light_on_event);
    btn_light_half = create_button(power_cont, btn_light_half, "50%", _light_half_event);
    btn_light_off = create_button(power_cont, btn_light_off, "Off", _light_off_event);
#endif

    // Baby step buttons
    lv_obj_t *baby_cont = lv_cont_create(machine_page, NULL);
    lv_cont_set_layout(baby_cont, LV_LAYOUT_ROW_M);
    lv_cont_set_fit(baby_cont, LV_FIT_TIGHT);

    label_babystep = lv_label_create(baby_cont, NULL);
    lv_label_set_text_fmt(label_babystep, "Babystep:\n%.2fmm", reprap_axes.babystep[2]);

    btn_baby_closer = lv_btn_create(baby_cont, NULL);
    lv_btn_set_fit(btn_baby_closer, LV_FIT_TIGHT);
    lv_cont_set_layout(btn_baby_closer, LV_LAYOUT_ROW_T);
    lv_cont_set_fit2(btn_baby_closer, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_align(btn_baby_closer, NULL, LV_ALIGN_CENTER, 0,0);
    lv_obj_set_auto_realign(btn_baby_closer, true);
    lv_obj_set_event_cb(btn_baby_closer, babystep_closer);
    LV_IMG_DECLARE(closer_icon);
    lv_obj_t *img1 = lv_img_create(btn_baby_closer, NULL);
    lv_img_set_src(img1, &closer_icon);
    lv_obj_t *label_closer_title = lv_label_create(btn_baby_closer, NULL);
    lv_label_set_text(label_closer_title, "-0.01mm");

    btn_baby_away = lv_btn_create(baby_cont, NULL);
    lv_btn_set_fit(btn_baby_away, LV_FIT_TIGHT);
    lv_cont_set_layout(btn_baby_away, LV_LAYOUT_ROW_T);
    lv_cont_set_fit2(btn_baby_away, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_align(btn_baby_away, NULL, LV_ALIGN_CENTER, 0,0);
    lv_obj_set_auto_realign(btn_baby_away, true);
    lv_obj_set_event_cb(btn_baby_away, babystep_away);
    LV_IMG_DECLARE(away_icon);
    lv_obj_t *img2 = lv_img_create(btn_baby_away, NULL);
    lv_img_set_src(img2, &away_icon);
    lv_obj_t *label_away_title = lv_label_create(btn_baby_away, NULL);
    lv_label_set_text(label_away_title, "0.01mm");

    // Fan control
    lv_obj_t *fan_cont = lv_cont_create(machine_page, NULL);
    lv_cont_set_layout(fan_cont, LV_LAYOUT_ROW_M);
    lv_cont_set_fit(fan_cont, LV_FIT_TIGHT);
    lv_obj_t *label_fan_title = lv_label_create(fan_cont, NULL);
    lv_label_set_text(label_fan_title, "Fan:  ");

    slider = lv_slider_create(fan_cont, NULL);
    lv_obj_set_width(slider, LV_DPI * 2);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(slider, slider_event_cb);
    lv_slider_set_range(slider, 0, 100);
    label_fan = lv_label_create(fan_cont, NULL);
    lv_label_set_text_fmt(label_fan, " %u%% ", reprap_params.fan);
    btn_fan_off = create_button(fan_cont, btn_fan_off, " Off ", fan_off_event);

    update_ui_machine();
}