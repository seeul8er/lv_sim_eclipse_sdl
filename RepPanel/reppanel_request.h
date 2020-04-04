//
// Created by cyber on 18.03.20.
//

#ifndef LVGL_REPPANEL_REQUEST_H
#define LVGL_REPPANEL_REQUEST_H

void reprap_wifi_send_gcode(char *gcode);
void request_filaments();
void request_macros();
void request_jobs_async(char *folder_path);
void request_macros_async(char *folder_path);
void reprap_send_gcode(char *gcode_command);

#endif //LVGL_REPPANEL_REQUEST_H
