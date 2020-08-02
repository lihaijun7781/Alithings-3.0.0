NAME := device_sal_esp8266_12F

$(NAME)_MBINS_TYPE := kernel
$(NAME)_VERSION := 1.0.1
$(NAME)_SUMMARY := sal hal implementation for esp8266_12F
$(NAME)_SOURCES += wifi_atcmd.c
GLOBAL_DEFINES += DEV_SAL_ESP8266_12F

$(NAME)_COMPONENTS += yloop

ifneq (1, $(at_adapter))
$(NAME)_COMPONENTS += atparser

$(NAME)_SOURCES += esp8266_12F.c
endif

$(NAME)_INCLUDES += ./
