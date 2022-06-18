//
// Created by cyber on 18.03.20.
//

#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "reppanel_request.h"
#include "reppanel_console.h"
#include "reppanel.h"
#include "reppanel_macros.h"
#include "reppanel_jobselect.h"

void reprap_send_gcode(char *gcode_command) {
    printf("reprap_send_gcode - not implemented\n");
    add_console_hist_entry(gcode_command, NULL, CONSOLE_TYPE_INFO);
    update_entries_ui();
}

void process_reprap_filelist(char *buffer) {
    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error before: %s", error_ptr);
        }
        cJSON_Delete(root);
        return;
    }
    cJSON *err_resp = cJSON_GetObjectItem(root, "err");
    if (err_resp) {
        printf( "reprap_filelist - Duet responded with error code %i", err_resp->valueint);
        printf( "%s", cJSON_Print(root));
        cJSON_Delete(root);
        return;
    }
    cJSON *next = cJSON_GetObjectItem(root, "next");
    if (next != NULL && next->valueint != 0) {
        // TODO: Not only get first list. Get all items. Check for next item
    }
    cJSON *dir_name = cJSON_GetObjectItem(root, "dir");
    if (dir_name && strncmp("0:/filaments", dir_name->valuestring, 12) == 0) {
        printf("Processing filament names");

        cJSON *filament_folders = cJSON_GetObjectItem(root, "files");
        if (filament_folders) {
            cJSON *iterator = NULL;
            int pos = 0;
            filament_names[0] = '\0';
            cJSON_ArrayForEach(iterator, filament_folders) {
                if (cJSON_IsObject(iterator)) {
                    if (strncmp("d", cJSON_GetObjectItem(iterator, "type")->valuestring, 1) == 0) {
                        if (pos != 0) strncat(filament_names, "\n", MAX_LEN_STR_FILAMENT_LIST - strlen(filament_names));
                        strncat(filament_names, cJSON_GetObjectItem(iterator, "name")->valuestring,
                                MAX_LEN_STR_FILAMENT_LIST - strlen(filament_names));
                        pos++;
                    }
                }
            }
            printf("Filament names\n%s", filament_names);
        } else {
            filament_names[0] = '\0';
        }
    } else if (dir_name && strncmp("0:/macros", dir_name->valuestring, 9) == 0) {
        printf("Processing macros");
        // will add a back button if dir is empty
        strncpy(reprap_dir_elem[0].dir, dir_name->valuestring, MAX_LEN_DIRNAME - 1);
        for (int i = 0; i < MAX_NUM_ELEM_DIR; i++) {
            reprap_dir_elem[i].type = TREE_EMPTY_ELEM;
        }
        cJSON *_folders = cJSON_GetObjectItem(root, "files");
        if (_folders) {
            cJSON *iterator = NULL;
            int pos = 0;
            cJSON_ArrayForEach(iterator, _folders) {
                if (cJSON_IsObject(iterator)) {
                    if (pos < MAX_NUM_ELEM_DIR) {
                        strncpy(reprap_dir_elem[pos].name, cJSON_GetObjectItem(iterator, "name")->valuestring,
                                MAX_LEN_FILENAME - 1);
                        strncpy(reprap_dir_elem[pos].dir, dir_name->valuestring, MAX_LEN_DIRNAME - 1);
                        if (strncmp("f", cJSON_GetObjectItem(iterator, "type")->valuestring, 1) == 0) {
                            reprap_dir_elem[pos].type = TREE_FILE_ELEM;
                        } else {
                            reprap_dir_elem[pos].type = TREE_FOLDER_ELEM;
                        }
                        pos++;
                    }
                }
            }
        }
        update_macro_list_ui();
    } else if (dir_name && strncmp("0:/gcodes", dir_name->valuestring, 9) == 0) {
        printf("Processing jobs");
        cJSON *iterator = NULL;
        // will add a back button if dir is empty
        strncpy(reprap_dir_elem[0].dir, dir_name->valuestring, MAX_LEN_DIRNAME - 1);
        for (int i = 0; i < MAX_NUM_ELEM_DIR; i++) {  // clear array
            reprap_dir_elem[i].type = TREE_EMPTY_ELEM;
        }
        int pos = 0;
        cJSON *_folders = cJSON_GetObjectItem(root, "files");
        if (_folders) {
            cJSON_ArrayForEach(iterator, _folders) {
                if (cJSON_IsObject(iterator)) {
                    if (pos < MAX_NUM_ELEM_DIR) {
                        strncpy(reprap_dir_elem[pos].name, cJSON_GetObjectItem(iterator, "name")->valuestring,
                                MAX_LEN_FILENAME - 1);
                        strncpy(reprap_dir_elem[pos].dir, dir_name->valuestring, MAX_LEN_DIRNAME - 1);
                        if (strncmp("f", cJSON_GetObjectItem(iterator, "type")->valuestring, 1) == 0) {
                            reprap_dir_elem[pos].type = TREE_FILE_ELEM;
                        } else {
                            reprap_dir_elem[pos].type = TREE_FOLDER_ELEM;
                        }
                        pos++;
                    }
                }
            }
        }
        update_job_list_ui();

    }
    cJSON_Delete(root);
}
