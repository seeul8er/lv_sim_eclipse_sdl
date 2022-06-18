//
// Created by cyber on 01.03.20.
//

#include <stdio.h>
#include <lvgl/src/lv_objx/lv_page.h>
#include <lvgl/lvgl.h>
#include "custom_themes/lv_theme_rep_panel_dark.h"
#include "reppanel_info.h"
#include "reppanel_settings.h"
#include "reppanel.h"
#include "reppanel_helper.h"

#define TAG "RepPanelInfo"

lv_obj_t *info_page;
lv_obj_t *ta_wifi_pass;
lv_obj_t *ta_ssid;
lv_obj_t *ta_printer_addr;
lv_obj_t *ta_reprap_pass;
static lv_obj_t *cont_overlay, *cont_overlay_duet_addr;
static lv_obj_t *ddl_ssid, *ddl_duets;

static lv_obj_t *kb;

static void save_reppanel_settings_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        printf("Not implemented\n");
    }
}

static void kb_event_cb(lv_obj_t *event_kb, lv_event_t event) {
    /* Just call the regular event handler */
    lv_kb_def_event_cb(event_kb, event);
    if (event == LV_EVENT_CANCEL) {
        lv_obj_del(kb);
        kb = NULL;
    }
}

static void ta_event_cb(lv_obj_t *ta, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if (kb == NULL) {
            kb = lv_kb_create(info_page, NULL);
            lv_obj_set_pos(kb, 0, 0);
            lv_obj_set_size(kb, LV_HOR_RES - 30, 120);
            lv_kb_set_cursor_manage(kb, true); /* Automatically show/hide cursors on text areas */
            lv_kb_set_ta(kb, ta);
            lv_obj_set_event_cb(kb, kb_event_cb);
        } else {
            lv_kb_set_ta(kb, ta);
        }
    } else if (event == LV_EVENT_INSERT) {
        const char *str = lv_event_get_data();
        if (str[0] == '\n') {
            printf("Ready\n");
        }
    }
}

static void set_ssid_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char buf[33];
        lv_ddlist_get_selected_str(ddl_ssid, buf, sizeof(buf));
        lv_ta_set_text(ta_ssid, buf);
        lv_obj_del(cont_overlay);
    }
}

static void cancel_ssid_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        lv_obj_del(cont_overlay);
    }
}

static void scan_wifi_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char ssids[] = "SSID1\nSSID02\nSSID3\n";

        cont_overlay = lv_cont_create(lv_layer_top(), NULL);
        static lv_style_t somestyle;
        lv_style_copy(&somestyle, lv_cont_get_style(cont_overlay, LV_CONT_STYLE_MAIN));
        somestyle.body.border.width = 1;
        somestyle.body.border.color = REP_PANEL_DARK_ACCENT;
        somestyle.body.padding.left = LV_DPI / 6;
        somestyle.body.padding.right = LV_DPI / 6;
        somestyle.body.padding.top = LV_DPI / 12;
        somestyle.body.padding.bottom = LV_DPI / 12;
        somestyle.body.padding.inner = LV_DPI / 9;
        lv_cont_set_style(cont_overlay, LV_CONT_STYLE_MAIN, &somestyle);

        lv_cont_set_fit(cont_overlay, LV_FIT_TIGHT);
        lv_cont_set_layout(cont_overlay, LV_LAYOUT_PRETTY);
        lv_obj_align_origo(cont_overlay, NULL, LV_ALIGN_CENTER, 0, -100);

        lv_obj_t *l = lv_label_create(cont_overlay, NULL);
        lv_label_set_text(l, "Select SSID");
        ddl_ssid = lv_ddlist_create(cont_overlay, NULL);
        lv_ddlist_set_sb_mode(ddl_ssid, LV_SB_MODE_AUTO);
        lv_ddlist_set_fix_height(ddl_ssid, LV_HOR_RES - 350);
        lv_ddlist_set_align(ddl_ssid, LV_LABEL_ALIGN_LEFT);
        lv_ddlist_set_draw_arrow(ddl_ssid, true);
        lv_ddlist_set_fix_width(ddl_ssid, 300);
        lv_ddlist_set_options(ddl_ssid, ssids);
        static lv_obj_t *ok_buttn;
        create_button(cont_overlay, ok_buttn, "OK", set_ssid_event);
        static lv_obj_t *cancel_buttn;
        create_button(cont_overlay, cancel_buttn, "Cancel", cancel_ssid_event);
    }
}

static void set_duet_addr_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char buf[64];
        lv_ddlist_get_selected_str(ddl_duets, buf, sizeof(buf));
        lv_ta_set_text(ta_printer_addr, buf);
        lv_obj_del(cont_overlay_duet_addr);
    }
}

static void cancel_duet_addr_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        lv_obj_del(cont_overlay_duet_addr);
    }
}

static void scan_duet_event(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        char duets[] = "DUET1\nDUET2\nDUET3\n";

        cont_overlay_duet_addr = lv_cont_create(lv_layer_top(), NULL);
        static lv_style_t somestyle;
        lv_style_copy(&somestyle, lv_cont_get_style(cont_overlay_duet_addr, LV_CONT_STYLE_MAIN));
        somestyle.body.border.width = 1;
        somestyle.body.border.color = REP_PANEL_DARK_ACCENT;
        somestyle.body.padding.left = LV_DPI / 6;
        somestyle.body.padding.right = LV_DPI / 6;
        somestyle.body.padding.top = LV_DPI / 12;
        somestyle.body.padding.bottom = LV_DPI / 12;
        somestyle.body.padding.inner = LV_DPI / 9;
        lv_cont_set_style(cont_overlay_duet_addr, LV_CONT_STYLE_MAIN, &somestyle);

        lv_cont_set_fit(cont_overlay_duet_addr, LV_FIT_TIGHT);
        lv_cont_set_layout(cont_overlay_duet_addr, LV_LAYOUT_PRETTY);
        lv_obj_align_origo(cont_overlay_duet_addr, NULL, LV_ALIGN_CENTER, 0, -100);

        lv_obj_t *l = lv_label_create(cont_overlay_duet_addr, NULL);
        lv_label_set_text(l, "Select Duet");
        ddl_duets = lv_ddlist_create(cont_overlay_duet_addr, NULL);
        lv_ddlist_set_sb_mode(ddl_duets, LV_SB_MODE_AUTO);
        lv_ddlist_set_fix_height(ddl_duets, LV_HOR_RES - 350);
        lv_ddlist_set_align(ddl_duets, LV_LABEL_ALIGN_LEFT);
        lv_ddlist_set_draw_arrow(ddl_duets, true);
        lv_ddlist_set_fix_width(ddl_duets, 300);
        lv_ddlist_set_options(ddl_duets, duets);
        static lv_obj_t *ok_buttn;
        create_button(cont_overlay_duet_addr, ok_buttn, "OK", set_duet_addr_event);
        static lv_obj_t *cancel_buttn;
        create_button(cont_overlay_duet_addr, cancel_buttn, "Cancel", cancel_duet_addr_event);
    }
}

void draw_info(lv_obj_t *parent_screen) {
    info_page = lv_page_create(parent_screen, NULL);
    lv_obj_set_size(info_page, lv_disp_get_hor_res(NULL),
                    lv_disp_get_ver_res(NULL) - (lv_obj_get_height(cont_header) + 5));
    lv_page_set_scrl_fit2(info_page, LV_FIT_TIGHT, LV_FIT_FILL);
    lv_page_set_scrl_layout(info_page, LV_LAYOUT_COL_L);

    lv_obj_t *label_about = lv_label_create(info_page, NULL);
    lv_label_set_recolor(label_about, true);
    lv_label_set_text_fmt(label_about, "%s RepPanel for ESP32# - v%s", REP_PANEL_DARK_ACCENT_ALT1_STR, "1.3.4");

    lv_obj_t *ssid_cnt = lv_cont_create(info_page, NULL);
    lv_cont_set_layout(ssid_cnt, LV_LAYOUT_ROW_M);
    lv_cont_set_fit(ssid_cnt, LV_FIT_TIGHT);
    lv_obj_t *label_ssid = lv_label_create(ssid_cnt, NULL);
    lv_label_set_text(label_ssid, "WiFi SSID:");

    ta_ssid = lv_ta_create(ssid_cnt, NULL);
    lv_ta_set_pwd_mode(ta_ssid, false);
    lv_ta_set_cursor_type(ta_ssid, LV_CURSOR_LINE | LV_CURSOR_HIDDEN);
    lv_obj_set_width(ta_ssid, LV_HOR_RES - 250);
    lv_obj_set_height(ta_ssid, 25);
    lv_ta_set_text(ta_ssid, (const char *) wifi_ssid);
    lv_obj_set_event_cb(ta_ssid, ta_event_cb);
    lv_ta_set_one_line(ta_ssid, true);

    static lv_obj_t *scan_wifi_btn;
    create_button(ssid_cnt, scan_wifi_btn, LV_SYMBOL_REFRESH, scan_wifi_event);

    lv_obj_t *wifi_pass_cnt = lv_cont_create(info_page, ssid_cnt);
    lv_obj_t *label_wifi_pass = lv_label_create(wifi_pass_cnt, NULL);
    lv_label_set_text(label_wifi_pass, "WiFi password:");

    ta_wifi_pass = lv_ta_create(wifi_pass_cnt, NULL);
    lv_ta_set_text(ta_wifi_pass, (const char *) wifi_pass);
    lv_ta_set_pwd_mode(ta_wifi_pass, true);
    lv_ta_set_one_line(ta_wifi_pass, true);
    lv_obj_set_width(ta_wifi_pass, LV_HOR_RES - 250);
    lv_obj_set_pos(ta_wifi_pass, 0, 0);
    lv_obj_set_event_cb(ta_wifi_pass, ta_event_cb);
    lv_ta_set_one_line(ta_wifi_pass, true);

    lv_obj_t *printer_addr_cnt = lv_cont_create(info_page, ssid_cnt);
    lv_obj_t *label_printer_addr = lv_label_create(printer_addr_cnt, NULL);
    lv_label_set_text(label_printer_addr, "Printer address:");

    ta_printer_addr = lv_ta_create(printer_addr_cnt, NULL);
    lv_ta_set_pwd_mode(ta_printer_addr, false);
    lv_ta_set_cursor_type(ta_printer_addr, LV_CURSOR_LINE | LV_CURSOR_HIDDEN);
    lv_obj_set_width(ta_printer_addr, LV_HOR_RES - 300);
    lv_obj_set_height(ta_printer_addr, 25);
    lv_ta_set_text(ta_printer_addr, "192.168.10.1");
    lv_obj_set_event_cb(ta_printer_addr, ta_event_cb);
    lv_ta_set_one_line(ta_ssid, true);

    static lv_obj_t *scan_duet_btn;
    create_button(printer_addr_cnt, scan_duet_btn, LV_SYMBOL_REFRESH, scan_duet_event);

    lv_obj_t *reprap_pass_cnt = lv_cont_create(info_page, ssid_cnt);
    lv_obj_t *label_reprap_pass = lv_label_create(reprap_pass_cnt, NULL);
    lv_label_set_text(label_reprap_pass, "RepRap password:");

    ta_reprap_pass = lv_ta_create(reprap_pass_cnt, NULL);
    lv_ta_set_text(ta_reprap_pass, "FuckOff!");
    lv_ta_set_pwd_mode(ta_reprap_pass, true);
    lv_ta_set_one_line(ta_reprap_pass, true);
    lv_obj_set_width(ta_reprap_pass, LV_HOR_RES - 260);
    lv_obj_set_pos(ta_reprap_pass, 0, 0);
    lv_obj_set_event_cb(ta_reprap_pass, ta_event_cb);
    lv_ta_set_one_line(ta_reprap_pass, true);

//    label_sig_strength = lv_label_create(info_page, NULL);
//    lv_label_set_recolor(label_sig_strength, true);
//    lv_label_set_long_mode(label_sig_strength, LV_LABEL_LONG_BREAK);
//    lv_label_set_text_fmt(label_sig_strength, "RepRap signal: %s-124dBm# RepPanel signal: %s-125dBm#",
//            REP_PANEL_DARK_ACCENT_ALT2_STR, REP_PANEL_DARK_ACCENT_ALT2_STR);
    static lv_obj_t *save_bnt;
    create_button(info_page, save_bnt, "Save", save_reppanel_settings_event);


}