//
// Copyright (c) 2020 Wolfgang Christl
// Licensed under Apache License, Version 2.0 - https://opensource.org/licenses/Apache-2.0

#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/lvgl.h>
#include <stdio.h>
#include "reppanel.h"
#include "reppanel_request.h"

#define TAG "Macros"

#define MACRO_ROOT_DIR  "0:/macros"
#define MACRO_EMPTY ""
#define BACK_TXT    "Back"

lv_obj_t *macro_list;
lv_obj_t *msg_box3;
lv_obj_t *preloader;
file_tree_elem_t *edit_macro;
char parent_dir_macros[MAX_LEN_DIRNAME + 1];

static void exe_macro_file_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_mbox_get_active_btn_text(msg_box3), "Yes") == 0) {
            printf("Running file %s", lv_list_get_btn_text(obj));
            char tmp_txt[strlen(edit_macro->dir) + strlen(edit_macro->name) + 10];
            sprintf(tmp_txt, "M98 P\"%s/%s\"", edit_macro->dir, edit_macro->name);
            reprap_send_gcode(tmp_txt);
            lv_obj_del_async(msg_box3);
        } else {
            lv_obj_del_async(msg_box3);
        }
    }
}

static void macro_clicked_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        int selected_indx = lv_list_get_btn_index(macro_list, obj);
        // check if back button exists
        if (strcmp(reprap_dir_elem[0].dir, MACRO_ROOT_DIR) != 0) {
            if (selected_indx == 0) {
                // back button was pressed
                printf("Going back to parent %s", parent_dir_macros);
                if (!preloader)
                    preloader = lv_preload_create(lv_layer_top(), NULL);
                lv_obj_set_size(preloader, 75, 75);
                lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
                process_reprap_filelist("\n"
                                        "Cache deaktivieren\n"
                                        "37 Anfragen\n"
                                        "82,73 KB / 89,01 KB übertragen\n"
                                        "Beendet: 10,92 s\n"
                                        "\n"
                                        "1\n"
                                        "\n"
                                        "{\"dir\":\"0:/macros/\",\"first\":0,\"files\":[{\"type\":\"f\",\"name\":\"Clean Nozzle\",\"size\":49,\"date\":\"2022-04-09T14:15:50\"},{\"type\":\"f\",\"name\":\"unload\",\"size\":514,\"date\":\"2021-07-18T12:48:02\"},{\"type\":\"f\",\"name\":\"CenterHead\",\"size\":74,\"date\":\"2020-11-08T13:59:50\"},{\"type\":\"f\",\"name\":\"non_lin_cali.gcode\",\"size\":931,\"date\":\"2021-01-17T12:34:46\"},{\"type\":\"d\",\"name\":\"Z Probe Switch Calibration\",\"size\":0,\"date\":\"2021-04-27T22:47:18\"},{\"type\":\"f\",\"name\":\"Determine Tool Offset\",\"size\":260,\"date\":\"2021-09-16T00:00:40\"},{\"type\":\"d\",\"name\":\"Accelleration Tuning\",\"size\":0,\"date\":\"2018-07-25T22:45:58\"},{\"type\":\"d\",\"name\":\"Jerk Tuning\",\"size\":0,\"date\":\"2018-07-25T22:39:22\"},{\"type\":\"d\",\"name\":\"Pressure Advance Tuning\",\"size\":0,\"date\":\"2018-07-26T00:44:30\"},{\"type\":\"d\",\"name\":\"Retraction Tuning\",\"size\":0,\"date\":\"2018-07-26T00:50:50\"},{\"type\":\"f\",\"name\":\"Trigger Report\",\"size\":226,\"date\":\"2022-01-01T10:48:20\"}],\"next\":0}\n"
                                        "\n"
                                        "2\n"
                                        "\n"
                                        "\u200B\n"
                                        "");
                return;
            } else {
                // no back button pressed
                // decrease index to match with reprap_macros array indexing
                selected_indx--;
            }
        }
        edit_macro = &reprap_dir_elem[selected_indx];
        if (edit_macro == NULL)
            return;
        if (reprap_dir_elem[selected_indx].type == TREE_FILE_ELEM) {
            static const char *btns[] = {"Yes", "No", ""};
            msg_box3 = lv_mbox_create(lv_layer_top(), NULL);
            char msg[152];
            sprintf(msg, "Do you want to execute %s?", edit_macro->name);
            lv_mbox_set_text(msg_box3, msg);
            lv_mbox_add_btns(msg_box3, btns);
            lv_obj_set_event_cb(msg_box3, exe_macro_file_handler);
            lv_obj_set_width(msg_box3, lv_disp_get_hor_res(NULL) - 20);
            lv_obj_align(msg_box3, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
        } else if (reprap_dir_elem[selected_indx].type == TREE_FOLDER_ELEM) {
            printf("Clicked folder %s (index %i)", edit_macro->name, selected_indx);
            if (!preloader)
                preloader = lv_preload_create(lv_layer_top(), NULL);
            lv_obj_set_size(preloader, 75, 75);
            lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
            static char tmp_txt[MAX_LEN_DIRNAME + MAX_LEN_FILENAME + 1];
            sprintf(tmp_txt, "%s/%s", reprap_dir_elem[selected_indx].dir, edit_macro->name);
            process_reprap_filelist("\n"
                                    "Cache deaktivieren\n"
                                    "37 Anfragen\n"
                                    "82,73 KB / 89,01 KB übertragen\n"
                                    "Beendet: 10,92 s\n"
                                    "\n"
                                    "1\n"
                                    "\n"
                                    "{\"dir\":\"0:/macros/\",\"first\":0,\"files\":[{\"type\":\"f\",\"name\":\"Clean Nozzle\",\"size\":49,\"date\":\"2022-04-09T14:15:50\"},{\"type\":\"f\",\"name\":\"unload\",\"size\":514,\"date\":\"2021-07-18T12:48:02\"},{\"type\":\"f\",\"name\":\"CenterHead\",\"size\":74,\"date\":\"2020-11-08T13:59:50\"},{\"type\":\"f\",\"name\":\"non_lin_cali.gcode\",\"size\":931,\"date\":\"2021-01-17T12:34:46\"},{\"type\":\"d\",\"name\":\"Z Probe Switch Calibration\",\"size\":0,\"date\":\"2021-04-27T22:47:18\"},{\"type\":\"f\",\"name\":\"Determine Tool Offset\",\"size\":260,\"date\":\"2021-09-16T00:00:40\"},{\"type\":\"d\",\"name\":\"Accelleration Tuning\",\"size\":0,\"date\":\"2018-07-25T22:45:58\"},{\"type\":\"d\",\"name\":\"Jerk Tuning\",\"size\":0,\"date\":\"2018-07-25T22:39:22\"},{\"type\":\"d\",\"name\":\"Pressure Advance Tuning\",\"size\":0,\"date\":\"2018-07-26T00:44:30\"},{\"type\":\"d\",\"name\":\"Retraction Tuning\",\"size\":0,\"date\":\"2018-07-26T00:50:50\"},{\"type\":\"f\",\"name\":\"Trigger Report\",\"size\":226,\"date\":\"2022-01-01T10:48:20\"}],\"next\":0}\n"
                                    "\n"
                                    "2\n"
                                    "\n"
                                    "\u200B\n"
                                    "");
        }
    }
}

void update_macro_list_ui() {
    if (visible_screen != REPPANEL_MACROS_SCREEN) return;
    lv_obj_del(preloader);
    if (macro_list) {
        lv_list_clean(macro_list);
        printf("Cleaned Macro List");
    } else {
        return;
    }

    // Add back button in case we are not in root directory
    if (strcmp(reprap_dir_elem[0].dir, MACRO_ROOT_DIR) != 0) {
        lv_obj_t *back_btn;
        back_btn = lv_list_add_btn(macro_list, LV_SYMBOL_LEFT, BACK_TXT);
        lv_obj_set_event_cb(back_btn, macro_clicked_event_handler);
        // update parent dir
        strcpy(parent_dir_macros, reprap_dir_elem[0].dir);
        char *pch;
        pch = strrchr(parent_dir_macros, '/');
        parent_dir_macros[pch - parent_dir_macros] = '\0';
    } else {
        strcpy(parent_dir_macros, MACRO_EMPTY);
    }
    // count number of elements
    uint8_t cnt = 0;
    for (int i = 0; reprap_dir_elem[i].type != TREE_EMPTY_ELEM && i < MAX_NUM_ELEM_DIR; i++) {
        cnt++;
    }
}

void draw_macro(lv_obj_t *parent_screen) {
//    strcpy(parent_dir_macros, MACRO_EMPTY);
    lv_obj_t *macro_container = lv_cont_create(parent_screen, NULL);
    lv_cont_set_layout(macro_container, LV_LAYOUT_COL_M);
    lv_cont_set_fit(macro_container, LV_FIT_FILL);

    preloader = lv_preload_create(macro_container, NULL);
    lv_obj_set_size(preloader, 75, 75);

    macro_list = lv_list_create(macro_container, NULL);
    lv_obj_set_size(macro_list, LV_HOR_RES - 10, lv_disp_get_ver_res(NULL) - (lv_obj_get_height(cont_header) + 5));

    process_reprap_filelist("\n"
                            "Cache deaktivieren\n"
                            "37 Anfragen\n"
                            "82,73 KB / 89,01 KB übertragen\n"
                            "Beendet: 10,92 s\n"
                            "\n"
                            "1\n"
                            "\n"
                            "{\"dir\":\"0:/macros/\",\"first\":0,\"files\":[{\"type\":\"f\",\"name\":\"Clean Nozzle\",\"size\":49,\"date\":\"2022-04-09T14:15:50\"},{\"type\":\"f\",\"name\":\"unload\",\"size\":514,\"date\":\"2021-07-18T12:48:02\"},{\"type\":\"f\",\"name\":\"CenterHead\",\"size\":74,\"date\":\"2020-11-08T13:59:50\"},{\"type\":\"f\",\"name\":\"non_lin_cali.gcode\",\"size\":931,\"date\":\"2021-01-17T12:34:46\"},{\"type\":\"d\",\"name\":\"Z Probe Switch Calibration\",\"size\":0,\"date\":\"2021-04-27T22:47:18\"},{\"type\":\"f\",\"name\":\"Determine Tool Offset\",\"size\":260,\"date\":\"2021-09-16T00:00:40\"},{\"type\":\"d\",\"name\":\"Accelleration Tuning\",\"size\":0,\"date\":\"2018-07-25T22:45:58\"},{\"type\":\"d\",\"name\":\"Jerk Tuning\",\"size\":0,\"date\":\"2018-07-25T22:39:22\"},{\"type\":\"d\",\"name\":\"Pressure Advance Tuning\",\"size\":0,\"date\":\"2018-07-26T00:44:30\"},{\"type\":\"d\",\"name\":\"Retraction Tuning\",\"size\":0,\"date\":\"2018-07-26T00:50:50\"},{\"type\":\"f\",\"name\":\"Trigger Report\",\"size\":226,\"date\":\"2022-01-01T10:48:20\"}],\"next\":0}\n"
                            "\n"
                            "2\n"
                            "\n"
                            "\u200B\n"
                            "");
}