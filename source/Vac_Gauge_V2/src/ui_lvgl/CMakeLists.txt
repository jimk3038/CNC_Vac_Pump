SET(SOURCES ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_small_gauge_scale_png.c
    ui_img_dark_led_png.c
    ui_img_red_led_png.c
    ui_img_small_gauge_needle_png.c
    ui_img_red_bug__png.c
    ui_img_grn_bug__png.c
    ui_img_45621010.c
    fonts/ui_font_courier_Bold.c)

add_library(ui ${SOURCES})
