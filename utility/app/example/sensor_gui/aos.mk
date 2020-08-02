NAME := sensor_gui

$(NAME)_MBINS_TYPE := app
$(NAME)_VERSION := 1.0.1
$(NAME)_SUMMARY := Everylinked sensor GUI example
$(NAME)_SOURCES := developerkitgui.c sensor_display.c freetype_display.c AliOS_Things_logo.c

$(NAME)_COMPONENTS := yloop cli freetype253 fatfs sensor linkkit_sdk_c  netmgr cjson

ENABLE_IRDA_HAL := 1

ENABLE_CAMERA_HAL := 1

$(NAME)_INCLUDES    += .

$(NAME)_COMPONENTS += littlevGL
$(NAME)_SOURCES += sensor_cloud_demo.c
$(NAME)_SOURCES += linkkit/linkkit_example_solo.c

$(NAME)_INCLUDES += ./ ./linkkit
GLOBAL_DEFINES += LITTLEVGL_DEVELOPERKIT CONFIG_AOS_FATFS_SUPPORT_MMC
