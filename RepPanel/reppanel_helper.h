//
// Created by cyber on 29.02.20.
//

#ifndef LVGL_REPPANEL_HELPER_H
#define LVGL_REPPANEL_HELPER_H

LV_FONT_DECLARE(reppanel_font_roboto_light_36)
LV_FONT_DECLARE(reppanel_font_roboto_bold_22)
LV_FONT_DECLARE(reppanel_font_roboto_regular_22)

char get_temp_unit();
void init_reprap_buffers();

lv_obj_t * create_button(lv_obj_t *parent, lv_obj_t *button_pnt, char *text, void *event_handler);

#endif //LVGL_REPPANEL_HELPER_H
