//
// Created by cyber on 01.03.20.
//

#include <stdio.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include "custom_themes/lv_theme_rep_panel_dark.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <stdlib.h>
#include "reppanel_process.h"
#include "reppanel.h"
#include "reppanel_request.h"
#include "reppanel_helper.h"

lv_obj_t *label_bed_temp;
lv_obj_t *btn_bed_temp_active;
lv_obj_t *btn_bed_temp_standby;
lv_obj_t *btn_tool_temp_active;
lv_obj_t *btn_tool_temp_standby;

lv_obj_t *label_bed_temp_active;
lv_obj_t *label_bed_temp_standby;
lv_obj_t *label_tool_temp_active;
lv_obj_t *label_tool_temp_standby;

lv_obj_t *label_tool_temp;
lv_obj_t *label_extruder_name;
lv_obj_t *button_tool_filament;
lv_obj_t *process_container;
lv_obj_t *cont_fila_overlay;

lv_obj_t *popup_page;

static lv_obj_t *ddlist_selected_filament;
static lv_obj_t *cont_filament;

int current_visible_tool_indx = 0;     // heater/tool/extruder that is currently visible within the UI

/**
 * Send command to RepRap
 * @param new_tmp
 * @param id BTN_BED_TMP_ACTIVE, BTN_TOOL_TMP_STANDBY etc.
 */
void set_heater_status(char *new_tmp, int id) {
    char gcode_buff[20];
    switch (id) {
        case BTN_BED_TMP_ACTIVE:
            sprintf(gcode_buff, "M140 P%i S%s", reprap_bed.heater_indx, new_tmp);
            reprap_send_gcode(gcode_buff);
            break;
        case BTN_BED_TMP_STANDBY:
            sprintf(gcode_buff, "M144 P%i", reprap_bed.heater_indx);
            reprap_send_gcode(gcode_buff);
            break;
        case BTN_TOOL_TMP_ACTIVE:
            sprintf(gcode_buff, "G10 P%i S%s", reprap_tools[current_visible_tool_indx].number, new_tmp);
            reprap_send_gcode(gcode_buff);

            sprintf(gcode_buff, "T%i", reprap_tools[current_visible_tool_indx].number);
            reprap_send_gcode(gcode_buff);
            break;
        case BTN_TOOL_TMP_STANDBY:
            sprintf(gcode_buff, "G10 P%i R%s", reprap_tools[current_visible_tool_indx].number, new_tmp);
            reprap_send_gcode(gcode_buff);

            sprintf(gcode_buff, "T-1");
            reprap_send_gcode(gcode_buff);
            break;
        default:
            break;
    }
}

/**
 * Update UI according to RepRap status for tools/heaters
 * @param state : 0,1,2,3
 * @param btn_active Button for active value
 * @param bnt_standby Button for standby value
 */
void apply_heater_style(int state, lv_obj_t *btn_active, lv_obj_t *bnt_standby) {
    switch (state) {
        case 0: // off
            lv_btn_set_state(btn_active, LV_BTN_STATE_REL);
            lv_btn_set_state(bnt_standby, LV_BTN_STATE_REL);
            lv_obj_set_hidden(btn_active, false);
            lv_obj_set_hidden(bnt_standby, false);
            break;
        case 1: // standby
            lv_btn_set_state(btn_active, LV_BTN_STATE_REL);
            lv_btn_set_state(bnt_standby, LV_BTN_STATE_TGL_REL);
            lv_obj_set_hidden(btn_active, false);
            lv_obj_set_hidden(bnt_standby, false);
            break;
        case 2: // active
            lv_btn_set_state(btn_active, LV_BTN_STATE_TGL_REL);
            lv_btn_set_state(bnt_standby, LV_BTN_STATE_REL);
            lv_obj_set_hidden(btn_active, false);
            lv_obj_set_hidden(bnt_standby, false);
            break;
        default:
        case 3: // fault
            lv_btn_set_state(btn_active, LV_BTN_STATE_TGL_PR);
            lv_btn_set_state(bnt_standby, LV_BTN_STATE_TGL_PR);
            lv_obj_set_hidden(btn_active, true);
            lv_obj_set_hidden(bnt_standby, true);
            break;
    }
}

/**
 * Update UI based on RepRap response
 */
void update_heater_status(const int states[MAX_NUM_TOOLS], int num_heaters) {
    for (int i = 0; i < num_heaters; i++) {
        if (i == 0) {
            apply_heater_style(states[i], btn_bed_temp_active, btn_bed_temp_standby);
        } else if (i == (current_visible_tool_indx + 1)) {
            apply_heater_style(states[i], btn_tool_temp_active, btn_tool_temp_standby);
        }
    }
    memcpy(toolstates, states, MAX_NUM_TOOLS);
}

static void _choose_prev_tool_event_handler(lv_obj_t *obj, lv_event_t event) {

}

static void _choose_next_tool_event_handler(lv_obj_t *obj, lv_event_t event) {

}

static void redraw_process_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        lv_obj_del(cont_fila_overlay);
    }
}

static void load_filament_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char val_txt_buff[100];
        lv_ddlist_get_selected_str(ddlist_selected_filament, val_txt_buff, 100);
        printf("Loading %s", val_txt_buff);
    }
}

static void unload_filament_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char val_txt_buff[100];
        lv_ddlist_get_selected_str(ddlist_selected_filament, val_txt_buff, 100);
        printf("Unloading %s", val_txt_buff);
    }
}

static void change_tmp_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        const char *val_txt_buff = lv_btnm_get_active_btn_text(obj);
        if (val_txt_buff != NULL) {
            switch ((int) (lv_obj_user_data_t) obj->user_data) {
                case BTN_BED_TMP_ACTIVE:
                    lv_label_set_text(label_bed_temp_active, val_txt_buff);
                    break;
                case BTN_BED_TMP_STANDBY:
                    lv_label_set_text(label_bed_temp_standby, val_txt_buff);
                    break;
                case BTN_TOOL_TMP_ACTIVE:
                    lv_label_set_text(label_tool_temp_active, val_txt_buff);
                    break;
                case BTN_TOOL_TMP_STANDBY:
                    lv_label_set_text(label_tool_temp_standby, val_txt_buff);
                    break;
                default:
                    break;
            }
        }
        lv_obj_del_async(popup_page);
    }
}

static void close_popup_page(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        lv_obj_del_async(popup_page);
    }
}

static void set_bed_temp_status_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_SHORT_CLICKED) {
        printf("Change heater temperature\n");
        // keep current selection. Toggle buttons will otherwise switch
        if (lv_btn_get_state(obj) == LV_BTN_STATE_TGL_REL) {
            lv_btn_set_state(obj, LV_BTN_STATE_REL);
        } else if (lv_btn_get_state(obj) == LV_BTN_STATE_REL) {
            lv_btn_set_state(obj, LV_BTN_STATE_TGL_REL);
        }
        popup_page = lv_page_create(lv_layer_top(), NULL);
        static lv_style_t style_bg;
        lv_style_copy(&style_bg, lv_page_get_style(popup_page, LV_PAGE_STYLE_BG));
        style_bg.body.border.width = 1;
        style_bg.body.border.color = REP_PANEL_DARK_ACCENT;
        lv_page_set_style(popup_page, LV_PAGE_STYLE_BG, &style_bg);
        lv_page_set_scrl_layout(popup_page, LV_LAYOUT_COL_L);
        lv_obj_set_size(popup_page, LV_HOR_RES - 20, LV_VER_RES - 20);
        lv_obj_align(popup_page, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_t *label = lv_label_create(popup_page, NULL);
        lv_label_set_text(label, "Select temperature");
        lv_obj_t *cont_temp_buttons = lv_cont_create(popup_page, NULL);
        lv_cont_set_layout(cont_temp_buttons, LV_LAYOUT_GRID);
        lv_cont_set_fit(cont_temp_buttons, LV_FIT_TIGHT);

        lv_obj_t *btn_matrix = lv_btnm_create(cont_temp_buttons, NULL);
        lv_obj_set_width(btn_matrix, LV_HOR_RES - 80);
        static char *temp_map_tmp[NUM_TEMPS_BUFF + 4];
        static char txt[NUM_TEMPS_BUFF][7];
        int map_indx = 0;
        double *temps;
        switch ((int) (lv_obj_user_data_t) obj->user_data) {
            case BTN_BED_TMP_ACTIVE:
                temps = reprap_bed_poss_temps.temps_active;
                break;
            case BTN_BED_TMP_STANDBY:
                temps = reprap_bed_poss_temps.temps_standby;
                break;
            case BTN_TOOL_TMP_ACTIVE:
                temps = reprap_tool_poss_temps.temps_active;
                break;
            case BTN_TOOL_TMP_STANDBY:
                temps = reprap_tool_poss_temps.temps_standby;
                break;
            default:
                temps = reprap_bed_poss_temps.temps_active;
                break;
        }
        for (int i = 0; i < NUM_TEMPS_BUFF + 1; i++) {
            if (i != 0 && i % 6 == 0) {
                temp_map_tmp[map_indx] = "\n";
                map_indx++;
            }
            if (temps[i] > 0) {
                sprintf(txt[i], "%.0f°%c", temps[i], 'C');
                temp_map_tmp[map_indx] = txt[i];
            } else {
                temp_map_tmp[map_indx] = "";
                break;
            }
            map_indx++;
        }
        lv_btnm_set_map(btn_matrix, (const char **) temp_map_tmp);
        lv_obj_set_event_cb(btn_matrix, change_tmp_event_handler);
        lv_obj_align(btn_matrix, NULL, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_user_data(btn_matrix, obj->user_data);       // pass on user data to know which temp is adjusted
        static lv_obj_t *cancel_btn;
        create_button(popup_page, cancel_btn, "Cancel", close_popup_page);
    } else if (event == LV_EVENT_LONG_PRESSED) {
        printf("Change heater status\n");
        char gcode_buff[20];
        if (lv_btn_get_state(obj) == LV_BTN_STATE_PR) {     // this kind of detection works great on linux. Not so much with a touch panel
            printf("\tActivating\n");
            char *temp_txt;
            if ((int) (lv_obj_user_data_t) obj->user_data == BTN_BED_TMP_ACTIVE) {
                lv_btn_set_state(btn_bed_temp_standby, LV_BTN_STATE_REL);
                temp_txt = lv_label_get_text(label_bed_temp_active);
                char char_temp_only[strlen(temp_txt)];
                strcpy(char_temp_only, temp_txt);
                char_temp_only[strlen(temp_txt) - 3] = '\0';     // cut off °C/F
                sprintf(gcode_buff, "M140 P%i S%s", reprap_bed.heater_indx, char_temp_only);
                reprap_send_gcode(gcode_buff);
            } else if ((int) (lv_obj_user_data_t) obj->user_data == BTN_BED_TMP_STANDBY) {
                lv_btn_set_state(btn_bed_temp_active, LV_BTN_STATE_REL);
                sprintf(gcode_buff, "M144 P%i", reprap_bed.heater_indx);
                reprap_send_gcode(gcode_buff);
            } else if ((int) (lv_obj_user_data_t) obj->user_data == BTN_TOOL_TMP_ACTIVE) {
                lv_btn_set_state(btn_tool_temp_standby, LV_BTN_STATE_REL);
                temp_txt = lv_label_get_text(label_tool_temp_active);
                char char_temp_only[strlen(temp_txt)];
                strcpy(char_temp_only, temp_txt);
                char_temp_only[strlen(temp_txt) - 3] = '\0';     // cut off °C/F
                sprintf(gcode_buff, "G10 P%i S%s", reprap_tools[current_visible_tool_indx].number, char_temp_only);
                reprap_send_gcode(gcode_buff);

                sprintf(gcode_buff, "T%i", reprap_tools[current_visible_tool_indx].number);
                reprap_send_gcode(gcode_buff);
            } else {
                lv_btn_set_state(btn_tool_temp_active, LV_BTN_STATE_REL);
                temp_txt = lv_label_get_text(label_tool_temp_standby);
                char char_temp_only[strlen(temp_txt)];
                strcpy(char_temp_only, temp_txt);
                char_temp_only[strlen(temp_txt) - 3] = '\0';     // cut off °C/F
                sprintf(gcode_buff, "G10 P%i R%s", reprap_tools[current_visible_tool_indx].number, char_temp_only);
                reprap_send_gcode(gcode_buff);

                sprintf(gcode_buff, "T-1");
                reprap_send_gcode(gcode_buff);
            }
        } else {
            switch ((int) (lv_obj_user_data_t) obj->user_data) {
                case BTN_BED_TMP_ACTIVE:
                case BTN_BED_TMP_STANDBY:
                    printf("\tDeactivating bed heater\n");
                    break;
                case BTN_TOOL_TMP_ACTIVE:
                case BTN_TOOL_TMP_STANDBY:
                default:
                    printf("\tDeactivating tool %i heater\n", current_visible_tool_indx);
                    break;
            }
        }
    }
}

static void filament_change_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        cont_fila_overlay = lv_cont_create(lv_layer_top(), NULL);
        static lv_style_t somestyle;
        lv_style_copy(&somestyle, lv_cont_get_style(cont_fila_overlay, LV_CONT_STYLE_MAIN));
        somestyle.body.border.width = 1;
        somestyle.body.border.color = REP_PANEL_DARK_ACCENT;
        somestyle.body.padding.left = LV_DPI / 6;
        somestyle.body.padding.right = LV_DPI / 6;
        somestyle.body.padding.top = LV_DPI / 12;
        somestyle.body.padding.bottom = LV_DPI / 12;
        somestyle.body.padding.inner = LV_DPI / 9;
        lv_cont_set_style(cont_fila_overlay, LV_CONT_STYLE_MAIN, &somestyle);

        lv_cont_set_fit2(cont_fila_overlay, LV_FIT_FILL, LV_FIT_FLOOD);
        lv_cont_set_layout(cont_fila_overlay, LV_LAYOUT_PRETTY);
        lv_obj_align_origo(cont_fila_overlay, NULL, LV_ALIGN_CENTER, 0, -120);

        cont_filament = lv_cont_create(cont_fila_overlay, NULL);
        lv_cont_set_layout(cont_filament, LV_LAYOUT_PRETTY);
        lv_cont_set_fit2(cont_filament, LV_FIT_FILL, LV_FIT_TIGHT);

        lv_obj_t *l = lv_label_create(cont_filament, NULL);
        lv_label_set_text(l, "Filaments");

        ddlist_selected_filament = lv_ddlist_create(cont_filament, NULL);
        lv_ddlist_set_options(ddlist_selected_filament, filament_names);
        lv_ddlist_set_draw_arrow(ddlist_selected_filament, true);
        lv_ddlist_set_fix_height(ddlist_selected_filament, 150);
        lv_ddlist_set_fix_width(ddlist_selected_filament, 250);
        lv_ddlist_set_sb_mode(ddlist_selected_filament, LV_SB_MODE_AUTO);

        lv_obj_t *cont2 = lv_cont_create(cont_filament, NULL);
        lv_cont_set_layout(cont2, LV_LAYOUT_ROW_M);
        lv_cont_set_fit2(cont2, LV_FIT_FILL, LV_FIT_TIGHT);

        static lv_obj_t *button_load_filament;
        create_button(cont2, button_load_filament, "Load", load_filament_event_handler);
        static lv_obj_t *button_unload_filament;
        create_button(cont2, button_unload_filament, "Unload", unload_filament_event_handler);

        static lv_obj_t *button_exit;
        create_button(cont_fila_overlay, button_exit, "Ok", redraw_process_event_handler);

        lv_obj_align(cont_filament, cont_fila_overlay, LV_ALIGN_CENTER, 0, 0); /*Align to the corner*/
    }
}

/**
 * Draw process screen showing temperatures with possibility to change them. Stuff to edit FFF process related parameters
 * @param parent_screen Parent screen to draw elements on
 */
void draw_process(lv_obj_t *parent_screen) {
    process_container = lv_cont_create(parent_screen, NULL);
    lv_cont_set_layout(process_container, LV_LAYOUT_ROW_T);
    lv_cont_set_fit2(process_container, LV_FIT_TIGHT, LV_FIT_TIGHT);

    lv_obj_t *holder1 = lv_cont_create(process_container, NULL);
    lv_cont_set_fit(holder1, LV_FIT_TIGHT);
    lv_cont_set_layout(holder1, LV_LAYOUT_COL_M);
    lv_obj_t *holder2 = lv_cont_create(process_container, NULL);
    lv_cont_set_fit(holder2, true);
    lv_cont_set_layout(holder2, LV_LAYOUT_COL_M);
    lv_obj_t *holder3 = lv_cont_create(process_container, NULL);
    lv_cont_set_fit(holder3, true);
    lv_cont_set_layout(holder3, LV_LAYOUT_COL_M);

    lv_obj_t *holder_empty = lv_cont_create(holder1, NULL);
    lv_cont_set_fit(holder_empty, true);
    lv_cont_set_layout(holder_empty, LV_LAYOUT_ROW_T);

    lv_obj_t *label_empty0 = lv_label_create(holder_empty, NULL);
    lv_label_set_text(label_empty0, "");
    lv_obj_t *label_empty1 = lv_label_create(holder1, NULL);
    lv_label_set_text(label_empty1, "");
    // move label down a bit
    static lv_style_t new_cont_empty_style;
    lv_style_copy(&new_cont_empty_style, lv_cont_get_style(holder_empty, LV_CONT_STYLE_MAIN));
    new_cont_empty_style.body.padding.top = LV_DPI / 18;
    lv_obj_set_style(holder_empty, &new_cont_empty_style);

    lv_obj_t *label_active = lv_label_create(holder1, NULL);
    lv_label_set_text(label_active, "Active");
    lv_obj_t *cont_empty = lv_cont_create(holder1, holder_empty);
    lv_obj_set_height(cont_empty, 5);
    lv_obj_t *label_standby = lv_label_create(holder1, NULL);
    lv_label_set_text(label_standby, "Standby");

    lv_obj_t *holder_bed = lv_cont_create(holder2, NULL);
    lv_cont_set_fit(holder_bed, true);
    lv_cont_set_layout(holder_bed, LV_LAYOUT_ROW_T);

    lv_obj_t *label_bed = lv_label_create(holder_bed, NULL);
    lv_label_set_text(label_bed, "Bed");
    label_bed_temp = lv_label_create(holder2, NULL);
    lv_label_set_text(label_bed_temp, "100.6°C");

    const lv_style_t *panel_style = lv_cont_get_style(holder_empty, LV_CONT_STYLE_MAIN);

    btn_bed_temp_active = lv_btn_create(holder2, NULL);
    lv_obj_set_event_cb(btn_bed_temp_active, set_bed_temp_status_event_handler);
    static lv_style_t new_released_style;
    lv_style_copy(&new_released_style, lv_btn_get_style(btn_bed_temp_active, LV_BTN_STYLE_REL));
    new_released_style.body.main_color = panel_style->body.main_color;
    new_released_style.body.grad_color = panel_style->body.grad_color;
    new_released_style.body.border.width = 0;
    new_released_style.body.padding.left = LV_DPI / 3;
    new_released_style.body.padding.right = LV_DPI / 3;
    new_released_style.body.padding.top = LV_DPI / 12;
    new_released_style.body.padding.bottom = LV_DPI / 12;
    new_released_style.body.padding.inner = LV_DPI / 10;
    lv_btn_set_fit(btn_bed_temp_active, LV_FIT_TIGHT);
    lv_btn_set_style(btn_bed_temp_active, LV_BTN_STYLE_REL, &new_released_style);
    lv_btn_set_toggle(btn_bed_temp_active, true);
    label_bed_temp_active = lv_label_create(btn_bed_temp_active, NULL);
    lv_label_set_text(label_bed_temp_active, "100°C");
    lv_obj_set_user_data(btn_bed_temp_active, BTN_BED_TMP_ACTIVE);

    btn_bed_temp_standby = lv_btn_create(holder2, btn_bed_temp_active);
    lv_obj_set_user_data(btn_bed_temp_standby, (lv_obj_user_data_t) BTN_BED_TMP_STANDBY);
    label_bed_temp_standby = lv_label_create(btn_bed_temp_standby, NULL);
    lv_label_set_text(label_bed_temp_standby, "0°C");

    lv_obj_t *holder_extruder = lv_cont_create(holder3, NULL);
    lv_cont_set_fit(holder_extruder, true);
    lv_cont_set_layout(holder_extruder, LV_LAYOUT_ROW_M);

    lv_obj_t *prev_extruder_label = lv_label_create(holder_extruder, NULL);
    lv_label_set_text(prev_extruder_label, LV_SYMBOL_LEFT);

    label_extruder_name = lv_label_create(holder_extruder, NULL);
    lv_label_set_text(label_extruder_name, tool_names_map[0]);

    lv_obj_t *next_extruder_label = lv_label_create(holder_extruder, NULL);
    lv_label_set_text(next_extruder_label, LV_SYMBOL_RIGHT);

    label_tool_temp = lv_label_create(holder3, NULL);
    lv_label_set_text(label_tool_temp, "258.7°C");

    btn_tool_temp_active = lv_btn_create(holder3, btn_bed_temp_active);
    lv_obj_set_user_data(btn_tool_temp_active, (lv_obj_user_data_t) BTN_TOOL_TMP_ACTIVE);
    label_tool_temp_active = lv_label_create(btn_tool_temp_active, NULL);
    lv_label_set_text(label_tool_temp_active, "110°C");

    btn_tool_temp_standby = lv_btn_create(holder3, btn_bed_temp_active);
    lv_obj_set_user_data(btn_tool_temp_standby, (lv_obj_user_data_t) BTN_TOOL_TMP_STANDBY);
    label_tool_temp_standby = lv_label_create(btn_tool_temp_standby, NULL);
    lv_label_set_text(label_tool_temp_standby, "10°C");

    create_button(holder3, button_tool_filament, "Filament", filament_change_event);

    // light big font for current temps
    static lv_style_t style_label;
    lv_style_copy(&style_label, &lv_style_plain);
    style_label.text.color = REP_PANEL_DARK_HIGHLIGHT;
    style_label.text.font = &reppanel_font_roboto_light_36;
    lv_obj_set_style(label_empty1, &style_label);
    lv_obj_set_style(label_bed_temp, &style_label);
    lv_obj_set_style(label_tool_temp, &style_label);

    // Bold font for headings
    static lv_style_t style_label_heading;
    lv_style_copy(&style_label_heading, &lv_style_plain);
    style_label_heading.text.color = REP_PANEL_DARK_TEXT;
    style_label_heading.text.font = &reppanel_font_roboto_bold_22;
    lv_obj_set_style(label_extruder_name, &style_label_heading);
    lv_obj_set_style(label_bed, &style_label_heading);

    // built in icon font to display arrows
    static lv_style_t style_label_icon;
    lv_style_copy(&style_label_icon, &lv_style_plain);
    style_label_icon.text.color = REP_PANEL_DARK_ACCENT;
    lv_obj_set_style(prev_extruder_label, &style_label_icon);
    lv_obj_set_style(next_extruder_label, &style_label_icon);

    reprap_bed_poss_temps.temps_standby[0] = 100;
    reprap_bed_poss_temps.temps_standby[1] = 101;
    reprap_bed_poss_temps.temps_standby[2] = 102;
    reprap_bed_poss_temps.temps_standby[3] = 103;
    reprap_bed_poss_temps.temps_standby[4] = 104;
}