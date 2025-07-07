#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "styles.h"
#include "ui.h"


#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;

void create_screen_main_scr() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main_scr = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 480, 480);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    {
        lv_obj_t *parent_obj = obj;
        {
            // speedArc
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.speed_arc = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 475, 475);
            lv_arc_set_range(obj, 0, 240);
            lv_arc_set_value(obj, 25);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_rounded(obj, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_color(obj, lv_color_hex(0xffffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 64, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 15, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff12020), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // speedScale
            lv_obj_t *obj = lv_scale_create(parent_obj);
            objects.speed_scale = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 475, 475);
            lv_scale_set_mode(obj, LV_SCALE_MODE_ROUND_INNER);
            lv_scale_set_range(obj, 0, 240);
            lv_scale_set_total_tick_count(obj, 49);
            lv_scale_set_major_tick_every(obj, 4);
            lv_scale_set_label_show(obj, true);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_length(obj, 20, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_24px, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_length(obj, 10, LV_PART_ITEMS | LV_STATE_DEFAULT);
            lv_obj_set_style_arc_width(obj, 3, LV_PART_ITEMS | LV_STATE_DEFAULT);
        }
        {
            // speed
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.speed = obj;
            lv_obj_set_pos(obj, 0, -23);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_128px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "000");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 348, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_16px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "KPH");
        }
        {
            // fuelBar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.fuel_bar = obj;
            lv_obj_set_pos(obj, -30, 75);
            lv_obj_set_size(obj, 90, 10);
            lv_bar_set_value(obj, 25, LV_ANIM_OFF);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_pad(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
        {
            // coolantBar
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.coolant_bar = obj;
            lv_obj_set_pos(obj, -30, 100);
            lv_obj_set_size(obj, 90, 10);
            lv_bar_set_range(obj, 70, 120);
            lv_bar_set_value(obj, 88, LV_ANIM_OFF);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_color(obj, lv_color_hex(0xff00ffd1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_pad(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_outline_opa(obj, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0xffff0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
        }
        {
            // fuelLevel
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.fuel_level = obj;
            lv_obj_set_pos(obj, 48, 75);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "100");
        }
        {
            // coolant
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.coolant = obj;
            lv_obj_set_pos(obj, 48, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_24px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "123");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 309, 312);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_14px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "%");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 309, 337);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(obj, &ui_font_white_rabbit_14px, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "C");
        }
        {
            // lowFuel_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.low_fuel_tt = obj;
            lv_obj_set_pos(obj, -115, 45);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_low_fuel);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xfffffb00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // overTemperature_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.over_temperature_tt = obj;
            lv_obj_set_pos(obj, 115, 45);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_over_temperature);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // brakes_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.brakes_tt = obj;
            lv_obj_set_pos(obj, -100, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_brakes);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // abs_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.abs_tt = obj;
            lv_obj_set_pos(obj, -58, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_abs);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // lowCoolant_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.low_coolant_tt = obj;
            lv_obj_set_pos(obj, 68, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_low_coolant);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // battery_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.battery_tt = obj;
            lv_obj_set_pos(obj, 110, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_battery);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // lowOil_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.low_oil_tt = obj;
            lv_obj_set_pos(obj, 26, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_low_oil);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // mil_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.mil_tt = obj;
            lv_obj_set_pos(obj, 1, 46);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_mil);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xfffffb00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // hiBeam_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.hi_beam_tt = obj;
            lv_obj_set_pos(obj, 0, 211);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_hi_beams);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xff8675ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // indicators_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.indicators_tt = obj;
            lv_obj_set_pos(obj, 0, -115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_turn_indicators);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xff00ff3d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // airbag_tt
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.airbag_tt = obj;
            lv_obj_set_pos(obj, -16, 170);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_airbag_srs);
            lv_image_set_pivot(obj, 0, 0);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_image_recolor(obj, lv_color_hex(0xffff3a3a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_image_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_main_scr();
}

void tick_screen_main_scr() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main_scr,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main_scr();
}
