SET(SOURCES screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_rpmlightsgrey_png.c
    images/ui_img_rpmlights_png.c
    fonts/ui_font_Montserrat_112.c
    fonts/ui_font_Montserrat_96.c)

add_library(ui ${SOURCES})
