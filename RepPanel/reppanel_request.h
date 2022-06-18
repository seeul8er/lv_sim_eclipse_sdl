//
// Created by cyber on 18.03.20.
//

#ifndef LVGL_REPPANEL_REQUEST_H
#define LVGL_REPPANEL_REQUEST_H

void reprap_wifi_send_gcode(char *gcode);
void request_filaments();
void reprap_send_gcode(char *gcode_command);

void process_reprap_filelist(char *buffer);

#endif //LVGL_REPPANEL_REQUEST_H
