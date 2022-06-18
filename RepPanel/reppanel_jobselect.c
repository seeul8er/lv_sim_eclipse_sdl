//
// Copyright (c) 2020 Wolfgang Christl
// Licensed under Apache License, Version 2.0 - https://opensource.org/licenses/Apache-2.0

#include <stdio.h>
#include <lvgl/lvgl.h>
#include "custom_themes/lv_theme_rep_panel_dark.h"
#include "reppanel.h"
#include "reppanel_request.h"
#include "rrf_objects.h"
#include "rrf3_object_model_parser.h"

#define CANCEL_BTN_TXT  "#ffffff Cancel#"
#define DELETE_BTN_TXT  "#c43145 "LV_SYMBOL_TRASH" Delete#"
#define SIM_BTN_TXT     LV_SYMBOL_EYE_OPEN" Simulate"
#define PRINT_BTN_TXT   REP_PANEL_DARK_ACCENT_STR " " LV_SYMBOL_PLAY" Print#"

#define JOBS_ROOT_DIR  "0:/gcodes"
#define JOBS_EMPTY ""
#define BACK_TXT    "Back"

lv_obj_t *jobs_list;
lv_obj_t *msg_box1; // file info msg box
lv_obj_t *msg_box2; // delete confirmation message box
lv_obj_t *msg_box3; // print confirmation message box
lv_obj_t *preloader;

char parent_dir_jobs[MAX_LEN_DIRNAME + 1];
file_tree_elem_t *edit_job;

static void trigger_request_fileinfo(char txt[]);

void send_print_command() {
    printf("Printing %s", edit_job->name);
    char tmp_txt[strlen(edit_job->dir) + strlen(edit_job->name) + 10];
    sprintf(tmp_txt, "M32 \"%s/%s\"", edit_job->dir, edit_job->name);
    reprap_send_gcode(tmp_txt);
}

static void print_file_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_mbox_get_active_btn_text(msg_box3), "Yes") == 0) {
            send_print_command();
            lv_obj_del_async(msg_box3);
            display_jobstatus();
        } else {
            lv_obj_del_async(msg_box3);
        }
    }
}

static void delete_file_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_mbox_get_active_btn_text(msg_box2), "Yes") == 0) {
            printf("Deleting %s", edit_job->name);
            char tmp_txt[strlen(edit_job->dir) + strlen(edit_job->name) + 10];
            sprintf(tmp_txt, "M30 \"%s/%s\"", edit_job->dir, edit_job->name);
            reprap_send_gcode(tmp_txt);
            lv_obj_del_async(msg_box2);
            lv_obj_del_async(msg_box1);
            process_reprap_filelist(edit_job->dir);
        } else {
            lv_obj_del_async(msg_box2);
        }
    }
}

static void job_action_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_mbox_get_active_btn_text(msg_box1), CANCEL_BTN_TXT) == 0) {
            printf("Close window. No action");
            lv_obj_del_async(msg_box1);
        } else if (strcmp(lv_mbox_get_active_btn_text(msg_box1), DELETE_BTN_TXT) == 0) {
            static const char *btns[] = {"Yes", "No", ""};
            msg_box2 = lv_mbox_create(lv_layer_top(), NULL);
            lv_mbox_set_text(msg_box2, "Do you really want to delete this file?");
            lv_mbox_add_btns(msg_box2, btns);
            lv_obj_set_event_cb(msg_box2, delete_file_handler);
            lv_obj_set_width(msg_box2, lv_disp_get_hor_res(NULL) - 20);
            lv_obj_align(msg_box2, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
        } else if (strcmp(lv_mbox_get_active_btn_text(msg_box1), PRINT_BTN_TXT) == 0) {
            send_print_command();
            lv_obj_del_async(msg_box1);
            display_jobstatus();
        } else if (strcmp(lv_mbox_get_active_btn_text(msg_box1), SIM_BTN_TXT) == 0) {
            printf("Simulate %s", edit_job->name);
            char tmp_txt[strlen(edit_job->dir) + strlen(edit_job->name) + 10];
            sprintf(tmp_txt, "M37 P\"%s/%s\"", edit_job->dir, edit_job->name);
            reprap_send_gcode(tmp_txt);
            lv_obj_del_async(msg_box1);
            display_jobstatus();
        }
    }
}

void update_file_info_dialog_ui(reprap_model_t *_reprap_model) {
    if (!msg_box1) return;
    static char mbox_txt[128];
    sprintf(mbox_txt, "Print time: %ih %imin\n"
                      "Simulated time: %ih %imin\n"
                      "Height: %.2fmm\n"
                      "Filament usage: %.2fmm",
            2, 10, 9, 54, 102.2,
            1248.2);
    printf("Updating file info dialog");
    lv_mbox_set_text(msg_box1, mbox_txt);
}

static void job_clicked_event_handler(lv_obj_t *obj, lv_event_t event) {
    int selected_indx = lv_list_get_btn_index(jobs_list, obj);
    // check if back button exists
    if (strcmp(reprap_dir_elem[0].dir, JOBS_ROOT_DIR) != 0) {
        if (selected_indx == 0 && event == LV_EVENT_SHORT_CLICKED) {
            // back button was pressed
            printf("Going back to parent %s", parent_dir_jobs);
            if (!preloader)
                preloader = lv_preload_create(lv_layer_top(), NULL);
            lv_obj_set_size(preloader, 75, 75);
            lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
            process_reprap_filelist("{\"dir\":\"0:/gcodes/\",\"first\":0,\"files\":[{\"type\":\"d\",\"name\":\"Calibration\",\"size\":0,\"date\":\"2020-06-28T12:24:28\"},{\"type\":\"d\",\"name\":\"CLFFF\",\"size\":0,\"date\":\"2020-06-28T12:24:36\"},{\"type\":\"d\",\"name\":\"Shopping Coins\",\"size\":0,\"date\":\"2020-09-10T20:00:20\"},{\"type\":\"d\",\"name\":\"RepPanel\",\"size\":0,\"date\":\"2022-06-12T22:40:30\"},{\"type\":\"d\",\"name\":\"Drone\",\"size\":0,\"date\":\"2020-12-19T18:25:12\"},{\"type\":\"d\",\"name\":\"BusinessTime\",\"size\":0,\"date\":\"2021-03-28T12:51:16\"},{\"type\":\"d\",\"name\":\"3D Printer Upgrades\",\"size\":0,\"date\":\"2021-05-01T14:43:56\"},{\"type\":\"d\",\"name\":\"Johannes\",\"size\":0,\"date\":\"2021-06-09T22:26:04\"},{\"type\":\"d\",\"name\":\"MessingAround\",\"size\":0,\"date\":\"2020-10-10T13:20:50\"},{\"type\":\"d\",\"name\":\"PhoneClip\",\"size\":0,\"date\":\"2021-02-19T10:24:32\"}],\"next\":0}");
            return;
        } else if (selected_indx != 0) {
            // no back button pressed
            // decrease index to match with reprap_macros array indexing
            selected_indx--;
        }
    }
    edit_job = &reprap_dir_elem[selected_indx];
    if (event == LV_EVENT_SHORT_CLICKED) {
        if (reprap_dir_elem[selected_indx].type == TREE_FILE_ELEM) {
            static const char *btns[] = {"Yes", "No", ""};
            msg_box3 = lv_mbox_create(lv_layer_top(), NULL);
            char msg[strlen(edit_job->name) + 23];
            sprintf(msg, "Do you want to print %s?", edit_job->name);
            lv_mbox_set_text(msg_box3, msg);
            lv_mbox_add_btns(msg_box3, btns);
            lv_obj_set_event_cb(msg_box3, print_file_handler);
            lv_obj_set_width(msg_box3, lv_disp_get_hor_res(NULL) - 20);
            lv_obj_align(msg_box3, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
        } else if (reprap_dir_elem[selected_indx].type == TREE_FOLDER_ELEM) {
            printf("Clicked folder %s (index %i)", edit_job->name, selected_indx);
            if (!preloader)
                preloader = lv_preload_create(lv_layer_top(), NULL);
            lv_obj_set_size(preloader, 75, 75);
            lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
            static char tmp_txt_job_path[MAX_LEN_DIRNAME + MAX_LEN_FILENAME + 1];
            sprintf(tmp_txt_job_path, "%s/%s", reprap_dir_elem[selected_indx].dir, edit_job->name);
            process_reprap_filelist("{\"dir\":\"0:/gcodes/\",\"first\":0,\"files\":[{\"type\":\"d\",\"name\":\"Calibration\",\"size\":0,\"date\":\"2020-06-28T12:24:28\"},{\"type\":\"d\",\"name\":\"CLFFF\",\"size\":0,\"date\":\"2020-06-28T12:24:36\"},{\"type\":\"d\",\"name\":\"Shopping Coins\",\"size\":0,\"date\":\"2020-09-10T20:00:20\"},{\"type\":\"d\",\"name\":\"RepPanel\",\"size\":0,\"date\":\"2022-06-12T22:40:30\"},{\"type\":\"d\",\"name\":\"Drone\",\"size\":0,\"date\":\"2020-12-19T18:25:12\"},{\"type\":\"d\",\"name\":\"BusinessTime\",\"size\":0,\"date\":\"2021-03-28T12:51:16\"},{\"type\":\"d\",\"name\":\"3D Printer Upgrades\",\"size\":0,\"date\":\"2021-05-01T14:43:56\"},{\"type\":\"d\",\"name\":\"Johannes\",\"size\":0,\"date\":\"2021-06-09T22:26:04\"},{\"type\":\"d\",\"name\":\"MessingAround\",\"size\":0,\"date\":\"2020-10-10T13:20:50\"},{\"type\":\"d\",\"name\":\"PhoneClip\",\"size\":0,\"date\":\"2021-02-19T10:24:32\"}],\"next\":0}");
        } else {
            printf("Selected unknown file tree element -> Index: %i - Type: %i - Name: %s - Dir: %s", selected_indx,
                     reprap_dir_elem[selected_indx].type, reprap_dir_elem[selected_indx].name, reprap_dir_elem[selected_indx].dir);
        }
    } else if (event == LV_EVENT_LONG_PRESSED && reprap_dir_elem[selected_indx].type == TREE_FILE_ELEM) {   // file info dialog
        // request file info
        char tmp_txt[strlen(edit_job->dir) + strlen(edit_job->name) + 2];
        sprintf(tmp_txt, "%s/%s", edit_job->dir, edit_job->name);
        trigger_request_fileinfo(tmp_txt);
        // build UI
        static const char *btns[] = {SIM_BTN_TXT, PRINT_BTN_TXT, DELETE_BTN_TXT, CANCEL_BTN_TXT, ""};
        msg_box1 = lv_mbox_create(lv_layer_top(), NULL);
        lv_mbox_set_text(msg_box1, "Loading Job Information...");
        lv_mbox_add_btns(msg_box1, btns);
        lv_mbox_set_recolor(msg_box1, true);
        lv_obj_set_width(msg_box1, lv_disp_get_hor_res(NULL) - 15);
        lv_obj_set_event_cb(msg_box1, job_action_handler);
        lv_obj_align(msg_box1, lv_layer_top(), LV_ALIGN_IN_TOP_MID, 0, 50);
    }
}

static void trigger_request_fileinfo(char txt[]) {
    reppanel_parse_rr_fileinfo("{\n"
                               "  \"err\": 0,\n"
                               "  \"size\": 46357299,\n"
                               "  \"lastModified\": \"2022-01-25T20:49:30\",\n"
                               "  \"height\": 300.00,\n"
                               "  \"firstLayerHeight\": 0.20,\n"
                               "  \"layerHeight\": 0.20,\n"
                               "  \"printTime\": 76773,\n"
                               "  \"filament\": [\n"
                               "    310244.8\n"
                               "  ],\n"
                               "  \"generatedBy\": \"SuperSlicer 2.3.57 on 2022-01-25 at 19:49:05 UTC\"\n"
                               "}", &reprap_model,273);
    update_file_info_dialog_ui(&reprap_model);

}

void update_job_list_ui() {
    if (visible_screen != REPPANEL_JOBSELECT_SCREEN) return;
    lv_obj_del(preloader);
    if (jobs_list) {
        lv_list_clean(jobs_list);
    } else {
        return;
    }

    // Add back button in case we are not in root directory
    if (strcmp(reprap_dir_elem[0].dir, JOBS_ROOT_DIR) != 0) {
        lv_obj_t *back_btn;
        back_btn = lv_list_add_btn(jobs_list, LV_SYMBOL_LEFT, BACK_TXT);
        lv_obj_set_event_cb(back_btn, job_clicked_event_handler);
        // update parent dir
        strcpy(parent_dir_jobs, reprap_dir_elem[0].dir);
        char *pch;
        pch = strrchr(parent_dir_jobs, '/');
        parent_dir_jobs[pch - parent_dir_jobs] = '\0';
    } else {
        strcpy(parent_dir_jobs, JOBS_EMPTY);
    }
    // count number of elements
    uint8_t cnt = 0;
    for (int i = 0; reprap_dir_elem[i].type != TREE_EMPTY_ELEM && i < MAX_NUM_ELEM_DIR; i++) {
        cnt++;
    }
}


void draw_jobselect(lv_obj_t *parent_screen) {
    lv_obj_t *jobs_container = lv_cont_create(parent_screen, NULL);
    lv_cont_set_layout(jobs_container, LV_LAYOUT_COL_M);
    lv_cont_set_fit(jobs_container, LV_FIT_FILL);

    preloader = lv_preload_create(jobs_container, NULL);
    lv_obj_set_size(preloader, 75, 75);

    jobs_list = lv_list_create(jobs_container, NULL);
    lv_obj_set_size(jobs_list, LV_HOR_RES - 10, lv_disp_get_ver_res(NULL) - (lv_obj_get_height(cont_header) + 5));

    process_reprap_filelist("{\"dir\":\"0:/gcodes/\",\"first\":0,\"files\":[{\"type\":\"d\",\"name\":\"Calibration\",\"size\":0,\"date\":\"2020-06-28T12:24:28\"},{\"type\":\"d\",\"name\":\"CLFFF\",\"size\":0,\"date\":\"2020-06-28T12:24:36\"},{\"type\":\"d\",\"name\":\"Shopping Coins\",\"size\":0,\"date\":\"2020-09-10T20:00:20\"},{\"type\":\"d\",\"name\":\"RepPanel\",\"size\":0,\"date\":\"2022-06-12T22:40:30\"},{\"type\":\"d\",\"name\":\"Drone\",\"size\":0,\"date\":\"2020-12-19T18:25:12\"},{\"type\":\"d\",\"name\":\"BusinessTime\",\"size\":0,\"date\":\"2021-03-28T12:51:16\"},{\"type\":\"d\",\"name\":\"3D Printer Upgrades\",\"size\":0,\"date\":\"2021-05-01T14:43:56\"},{\"type\":\"d\",\"name\":\"Johannes\",\"size\":0,\"date\":\"2021-06-09T22:26:04\"},{\"type\":\"d\",\"name\":\"MessingAround\",\"size\":0,\"date\":\"2020-10-10T13:20:50\"},{\"type\":\"d\",\"name\":\"PhoneClip\",\"size\":0,\"date\":\"2021-02-19T10:24:32\"}],\"next\":0}");
}