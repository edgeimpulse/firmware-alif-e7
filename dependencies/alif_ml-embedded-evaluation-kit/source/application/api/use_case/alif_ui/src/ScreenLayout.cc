/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "ScreenLayout.hpp"

#include "lvgl.h"
#include "lv_port.h"

#include "log_macros.h"

extern "C" {
LV_IMG_DECLARE(Alif240);
}
namespace alif {
namespace app {

namespace {
lv_obj_t *labelResult[5];
lv_obj_t *labelHeader;
lv_obj_t *imageObj;
lv_obj_t *imageHolder;
lv_obj_t *ledObj;

};

static lv_style_t confident;
static lv_style_t weak;

#define DISP_SCALE 2
void ScreenLayoutInit(const void *imgData, size_t imgSize, int imgWidth, int imgHeight, unsigned short imgZoom)
{
    lv_port_disp_init();

    uint32_t lv_lock_state = lv_port_lock();
    lv_obj_t *screen = lv_scr_act();

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_hex(0xffffff));
    lv_style_set_text_font(&style, &lv_font_montserrat_28);
    lv_obj_add_style(screen, &style, LV_PART_MAIN);

    /* Grid layout for the screen */
    lv_coord_t scrWidth = lv_obj_get_width(screen);
    lv_coord_t scrHeight = lv_obj_get_height(screen);
    bool landscape;
    if (scrWidth > scrHeight) {
        landscape = true;
        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW);
    } else {
        landscape = false;
        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    }
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Override some of the fanciness of the default theme */
    static lv_style_t noBorder;
    lv_style_init(&noBorder);
    lv_style_set_border_width(&noBorder, 0);

    static lv_style_t screenPadding;
    lv_style_init(&screenPadding);
    lv_style_set_pad_all(&screenPadding, 4 * DISP_SCALE);
    lv_style_set_pad_gap(&screenPadding, 8 * DISP_SCALE);

    static lv_style_t imageHolderPadding;
    lv_style_init(&imageHolderPadding);
    lv_style_set_pad_all(&imageHolderPadding, 4 * DISP_SCALE);

    static lv_style_t noPadding;
    lv_style_init(&noPadding);
    lv_style_set_pad_all(&noPadding, LV_PART_MAIN);

    static lv_style_t resultPadding;
    lv_style_init(&resultPadding);
    lv_style_set_pad_left(&resultPadding, LV_PART_MAIN);
    lv_style_set_pad_right(&resultPadding, LV_PART_MAIN);
    lv_style_set_pad_bottom(&resultPadding, LV_PART_MAIN);

    lv_obj_add_style(screen, &screenPadding, LV_PART_MAIN);

    /* Make a holder with a border for the camera image */
    /* (Serves to determine screen layout, as layout can get weird for zoomed image widgets) */
    imageHolder = lv_obj_create(screen);
    lv_obj_move_background(imageHolder);
    lv_obj_set_style_bg_color(imageHolder, lv_color_hex(0x666666), LV_PART_MAIN);
    lv_obj_set_style_radius(imageHolder, 4 * DISP_SCALE, LV_PART_MAIN);
    lv_obj_add_style(imageHolder, &noBorder, LV_PART_MAIN);
    lv_obj_add_style(imageHolder, &imageHolderPadding, LV_PART_MAIN);
#ifdef USE_LVGL_ZOOM
    lv_obj_set_content_width(imageHolder, imgWidth * 2 * DISP_SCALE);
    lv_obj_set_content_height(imageHolder, imgHeight * 2 * DISP_SCALE);
#else
    lv_obj_set_content_width(imageHolder, imgWidth * DISP_SCALE);
    lv_obj_set_content_height(imageHolder, imgHeight * DISP_SCALE);
#endif

    /* Make a holder for the results (allows easy re-layout for portrait/landscape) */
    lv_obj_t *resultHolder = lv_obj_create(screen);
    if (landscape) {
        lv_obj_set_height(resultHolder, LV_PCT(100));
    } else {
        lv_obj_set_width(resultHolder, LV_PCT(100));
    }
    lv_obj_set_flex_grow(resultHolder, 1);

    lv_obj_add_style(resultHolder, &noBorder, LV_PART_MAIN);
    lv_obj_add_style(resultHolder, &resultPadding, LV_PART_MAIN);

    lv_style_init(&confident);
    lv_style_set_text_color(&confident, lv_color_hex(0x0080ff));
    lv_style_set_text_font(&confident, &lv_font_montserrat_32);

    lv_style_init(&weak);
    lv_style_set_text_color(&weak, lv_color_hex(0xcccccc));
    lv_style_set_text_font(&weak, &lv_font_montserrat_24);

    /*Create a Label in the results area */
    labelHeader =  lv_label_create(resultHolder);

    lv_label_set_text_static(labelHeader, "");

    lv_obj_set_style_text_align(labelHeader, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_align(labelHeader, LV_ALIGN_TOP_MID);

    ledObj = lv_led_create(resultHolder);
    lv_obj_align(ledObj, LV_ALIGN_TOP_RIGHT, -4 * DISP_SCALE, -4 * DISP_SCALE);
    lv_led_off(ledObj);
    lv_obj_set_size(ledObj, 5 * DISP_SCALE, 5 * DISP_SCALE);

    /* And labels for results */
    int y = 25 * DISP_SCALE;
    for (auto &lbl : labelResult) {
        lbl = lv_label_create(resultHolder);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, y);
        lv_label_set_text(lbl, "");
        y += 25 * DISP_SCALE;
        // Text styles can be inherited, and that works for the
        // default style, but these state styles have to be put
        // directly onto the labels so that they check the
        // label's own state. If inherited, they check the parent's.
        lv_obj_add_style(lbl, &confident, LV_STATE_USER_1);
        lv_obj_add_style(lbl, &weak, LV_STATE_USER_2);
    }

    /* Centre the image in its holder */
    imageObj = lv_img_create(imageHolder);
    static lv_img_dsc_t imageDesc;
    imageDesc.data = (const uint8_t *)imgData;
    imageDesc.data_size = imgSize;
    imageDesc.header.always_zero = 0;
    imageDesc.header.reserved = 0;
    imageDesc.header.cf = LV_IMG_CF_TRUE_COLOR;
    imageDesc.header.w = imgWidth;
    imageDesc.header.h = imgHeight;
    lv_img_set_zoom(imageObj, imgZoom);
    lv_img_set_antialias(imageObj, false);
    lv_img_set_src(imageObj, &imageDesc);
    lv_obj_center(imageObj);

    /* Add a logo */
    lv_obj_t *alifObj = lv_img_create(resultHolder);
    lv_img_set_src(alifObj, &Alif240);
    lv_obj_set_align(alifObj, LV_ALIGN_BOTTOM_MID);
    lv_obj_move_background(alifObj);

    lv_port_unlock(lv_lock_state);
}

lv_obj_t *ScreenLayoutImageObject()
{
    return imageObj;
}

lv_obj_t *ScreenLayoutImageHolderObject()
{
    return imageHolder;
}

lv_obj_t *ScreenLayoutHeaderObject()
{
    return labelHeader;
}

lv_obj_t *ScreenLayoutLabelObject(int n)
{
    return labelResult[n];
}

lv_obj_t *ScreenLayoutLEDObject()
{
    return ledObj;
}

} /* namespace app */
} /* namespace alif */
