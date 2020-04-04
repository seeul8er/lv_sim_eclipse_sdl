//
// Created by cyber on 15.03.20.
//

#include <stdio.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <lvgl/src/lv_objx/lv_cont.h>
#include <lvgl/lvgl.h>
#include <stdlib.h>

#include "reppanel_macros.h"
#include "reppanel.h"
#include "reppanel_request.h"

#define MACRO_ROOT_DIR  "0:/macros"
#define MACRO_EMPTY ""
#define BACK_TXT    "Back"
#define MAX_LEN_DIR 64

file_tree_elem_t reprap_macros[MAX_NUM_MACROS];

lv_obj_t *macro_container;
lv_obj_t *macro_list;
lv_obj_t *preloader;
lv_obj_t *msg_box3;
char parent_dir[MAX_LEN_DIR];

reprap_macro_t *edit_macro;

static void _exe_macro_file_handler(lv_obj_t * obj, lv_event_t event) {
    if(event == LV_EVENT_RELEASED) {
        if (strcmp(lv_mbox_get_active_btn_text(msg_box3), "Yes") == 0) {
            printf("Macro: Running file %s\n", edit_macro->name);
            char tmp_txt[strlen(edit_macro->dir) + strlen(edit_macro->name) + 10];
            sprintf(tmp_txt, "M98 P\"%s/%s\"", edit_macro->dir, edit_macro->name);
            reprap_send_gcode(tmp_txt);
            lv_obj_del_async(msg_box3);
        } else {
            lv_obj_del_async(msg_box3);
        }
    }
}

static void _macro_clicked_event_handler(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        int selected_indx = lv_list_get_btn_index(macro_list, obj);
        // check if back button exists
        if (strcmp(((reprap_macro_t*) reprap_macros[selected_indx].element)->dir, MACRO_ROOT_DIR) != 0) {
            if (selected_indx == 0) {
                // back button was pressed
                printf("Going back to parent %s\n", parent_dir);
                if (!preloader)
                    preloader = lv_preload_create(lv_layer_top(), NULL);
                lv_obj_set_size(preloader, 75, 75);
                lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
                request_macros_async(parent_dir);
                return;
            }
            // no back button pressed
            // decrease index to match with reprap_macros array indexing
            selected_indx--;
        }
        edit_macro = (reprap_macro_t*) reprap_macros[selected_indx].element;
        if (edit_macro == NULL)
            return;
        if (reprap_macros[selected_indx].type == TREE_FILE_ELEM) {
            static const char * btns[] ={"Yes", "No", ""};
            msg_box3 = lv_mbox_create(lv_layer_top(), NULL);
            char msg[100];
            sprintf(msg, "Do you want to execute %s?", edit_macro->name);
            lv_mbox_set_text(msg_box3, msg);
            lv_mbox_add_btns(msg_box3, btns);
            lv_obj_set_event_cb(msg_box3, _exe_macro_file_handler);
            lv_obj_set_width(msg_box3, lv_disp_get_hor_res(NULL) - 20);
            lv_obj_align(msg_box3, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
        } else if (reprap_macros[selected_indx].type == TREE_FOLDER_ELEM) {
            printf("Clicked folder %s\n", edit_macro->name);
            if (!preloader)
                preloader = lv_preload_create(lv_layer_top(), NULL);
            lv_obj_set_size(preloader, 75, 75);
            lv_obj_align_origo(preloader, lv_layer_top(), LV_ALIGN_CENTER, 0, 0);
            static char tmp_txt[128];
            sprintf(tmp_txt, "%s/%s&first=0", edit_macro->dir, edit_macro->name);
            request_macros_async(tmp_txt);
        }
    }
}

void _nav_back_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_RELEASED) {
        static char tmp_txt[128];
        sprintf(tmp_txt, "%s&first=0", parent_dir);
        request_macros_async(tmp_txt);
    }
}

void update_macro_list_ui() {
    lv_obj_del(preloader);
    if (macro_list) {
        lv_list_clean(macro_list);
    } else {
        return;
    }
    // Add back button in case we are not in root directory
    if (strcmp(((reprap_macro_t *) reprap_macros[0].element)->dir, MACRO_ROOT_DIR) != 0) {
        lv_obj_t *back_btn;
        back_btn = lv_list_add_btn(macro_list, LV_SYMBOL_LEFT, BACK_TXT);
        lv_obj_set_event_cb(back_btn, _macro_clicked_event_handler);
        // update parent dir
        strcpy(parent_dir, ((reprap_macro_t *) reprap_macros[0].element)->dir);
        char *pch;
        pch = strrchr(parent_dir, '/');
        parent_dir[pch - parent_dir] = '\0';
    } else {
        strcpy(parent_dir, MACRO_EMPTY);
    }
    for (int i = 0; reprap_macros[i].element != NULL; i++) {
        lv_obj_t *list_btn;
        if (reprap_macros[i].type == TREE_FOLDER_ELEM)
            list_btn = lv_list_add_btn(macro_list, LV_SYMBOL_DIRECTORY,
                                       ((reprap_macro_t *) reprap_macros[i].element)->name);
        else
            list_btn = lv_list_add_btn(macro_list, LV_SYMBOL_FILE, ((reprap_macro_t *) reprap_macros[i].element)->name);
        lv_obj_set_event_cb(list_btn, _macro_clicked_event_handler);
    }
}

void draw_macro(lv_obj_t *parent_screen) {
    strcpy(parent_dir, MACRO_EMPTY);
    macro_container = lv_cont_create(parent_screen, NULL);
    lv_cont_set_layout(macro_container, LV_LAYOUT_CENTER);
    lv_cont_set_fit2(macro_container, LV_FIT_FILL, LV_FIT_TIGHT);

    preloader = lv_preload_create(macro_container, NULL);
    lv_obj_set_size(preloader, 75, 75);

    macro_list = lv_list_create(macro_container, NULL);
    lv_obj_set_size(macro_list, LV_HOR_RES-10, lv_disp_get_ver_res(NULL) - (lv_obj_get_height(cont_header) + 5));

    for (int i = 0; i < 10; i++) {
        if (reprap_macros[i].element == NULL) {
            reprap_macros[i].element = (reprap_macro_t *) malloc(sizeof(reprap_macro_t));
        }
        char txt[32];
        char txt_dir[] = "0:/macros";
        sprintf(txt, "macro %i", i);
        ((reprap_macro_t *) reprap_macros[i].element)->name = malloc(strlen(txt) + 1);
        ((reprap_macro_t *) reprap_macros[i].element)->dir = malloc(strlen(txt_dir) + 1);
        ((reprap_macro_t *) reprap_macros[i].element)->last_mod = malloc(1 + 1);
        strcpy(((reprap_macro_t *) reprap_macros[i].element)->name,txt);
        strcpy(((reprap_macro_t *) reprap_macros[i].element)->dir, txt_dir);
        if (i%2 == 0) {
            reprap_macros[i].type = TREE_FILE_ELEM;
        } else {
            reprap_macros[i].type = TREE_FOLDER_ELEM;
        }
    }
    update_macro_list_ui();
}
