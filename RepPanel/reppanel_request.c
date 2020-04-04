//
// Created by cyber on 18.03.20.
//

#include <stdio.h>
#include <stdlib.h>
#include "reppanel_request.h"
#include "reppanel_console.h"
#include "reppanel.h"
#include "reppanel_macros.h"
#include "reppanel_jobselect.h"

void reprap_wifi_download(char *file) {
    printf("reprap_wifi_download - not implemented\n");
}

void reprap_send_gcode(char *gcode_command) {
    printf("reprap_send_gcode - not implemented\n");
    add_console_hist_entry(gcode_command, NULL, CONSOLE_TYPE_INFO);
    update_entries_ui();
}

void request_filaments() {
    printf("request_filaments - not implemented\n");
}

void request_macros_async(char *folder_path) {
    printf("Requesting: %s\n", folder_path);
    request_macros();
}

void request_jobs_async(char *folder_path) {
    printf("Requesting: %s\n", folder_path);
    printf("Requesting Jobs not implemented\n");
    for (int i = 0; i < 10; i++) {
        if (reprap_jobs[i].element == NULL) {
            reprap_jobs[i].element = (reprap_job_t *) malloc(sizeof(reprap_job_t));
        }
        char txt[32];
        char txt_dir[] = "0:/gcodes";
        sprintf(txt, "Job %i", i);
        ((reprap_job_t *) reprap_jobs[i].element)->name = malloc(strlen(txt) + 1);
        ((reprap_job_t *) reprap_jobs[i].element)->dir = malloc(strlen(txt_dir) + 1);
        ((reprap_job_t *) reprap_jobs[i].element)->last_mod = malloc(1 + 1);
        strcpy(((reprap_job_t *) reprap_jobs[i].element)->name,txt);
        strcpy(((reprap_job_t *) reprap_jobs[i].element)->dir, txt_dir);
        if (i%2 == 0) {
            reprap_jobs[i].type = TREE_FILE_ELEM;
        } else {
            reprap_jobs[i].type = TREE_FOLDER_ELEM;
        }
    }
    update_job_list_ui();
}

void request_macros() {
    printf("request_macros - not implemented\n");
    for (int i = 0; i < 10; i++) {
        if (reprap_macros[i].element == NULL) {
            reprap_macros[i].element = (reprap_macro_t *) malloc(sizeof(reprap_macro_t));
        }
        char txt[32];
        char txt_dir[] = "0:/macros/Some other Dir";
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

void request_reprap_status_updates(int *task) {
    printf("request_reprap_status_updates - not implemented\n");
}

/**
 * Get additional status info from printer. No not call too often.
 * @param task
 */
void request_reprap_ext_status_updates(int *task) {
    printf("request_reprap_ext_status_updates - not implemented\n");
}