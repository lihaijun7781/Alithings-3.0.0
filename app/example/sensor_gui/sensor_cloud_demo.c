/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "aos/cli.h"
#include "aos/kernel.h"
#include "sensor/sensor.h"
#include "aos/yloop.h"
#include "ulog/ulog.h"
#include "netmgr.h"
#include "linkkit/infra/infra_compat.h"
#include "linkkit/infra/infra_defs.h"
#include "linkkit/dev_model_api.h"
#include "app_entry.h"
#include "linkkit/infra/infra_cjson.h"
#include "soc_init.h"

#define SENSOR_SAMPLE_TIME 1000 /* sensor sampling period is 1000 ms*/

#define PROP_POST_FORMAT_TEMP "{\"CurrentTemperature\":%.1f}"
#define PROP_POST_FORMAT_HUMI "{\"CurrentHumidity\":%.1f}"
#define PROP_POST_FORMAT_ALARMSTATUS "{\"AlarmState\":%d}"
#define PROP_POST_FORMAT_ACC  "{\"Accelerometer\":{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f}}"

volatile char g_wifi_connect = 0; /* wifi connect flag */

extern float alarm_setting;
extern bool alarm_clear ;


#ifdef AOS_COMP_CLI

static void print_devinfo()
{
    char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = {0};
    char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = {0};
#ifdef DEMO_DEBUG
    char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = {0};
    char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = {0};
#endif
    HAL_GetProductKey(_product_key);
    HAL_GetDeviceName(_device_name);
    LOG("pk:%s", _product_key);
    LOG("dn:%s", _device_name);
#ifdef DEMO_DEBUG
    HAL_GetProductSecret(_product_secret);
    HAL_GetDeviceSecret(_device_secret);
    LOG("ps:%s", _product_secret);
    LOG("ds:%s", _device_secret);
#endif
}

static void set_devinfo(char *pk, char *ps, char *dn, char *ds)
{
    if (dn != NULL) {
        HAL_SetDeviceName(dn);
    }
    if (ds != NULL) {
        HAL_SetDeviceSecret(ds);
    }
    if (pk != NULL) {
        HAL_SetProductKey(pk);
    }
    if (ps != NULL) {
        HAL_SetProductSecret(ps);
    }
}

static void handle_devinfo_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    const char *rtype = argc > 1 ? argv[1] : "";
    if (strcmp(rtype, "get") == 0) {
        print_devinfo();
    } else if (strcmp(rtype, "set") == 0) {
        if (argc == 4) {
            set_devinfo(NULL, NULL, argv[2], argv[3]);
        } else if (argc == 5) {
            set_devinfo(argv[2], argv[3], argv[4], "");
        } else if (argc == 6) {
            set_devinfo(argv[2], argv[3], argv[4], argv[5]);
        } else {
            LOG("arg number err! usage:");
            LOG("devinfo set {pk} {ps} {dn} [ds] | devinfo set {dn} {ds}");
        }
    } else if (strcmp(rtype, "clean") == 0) {
        set_devinfo("", "", "", "");
    } else {
        LOG("usage:");
        LOG("devinfo [set pk ps dn ds | set dn ds | get | clean]");
    }
}

static struct cli_command devinfo_cmd = { .name     = "devinfo",
    .help     = "devinfo [set pk ps dn ds | set dn ds | get | clean ]",
     .function = handle_devinfo_cmd
};
#endif

/* linkkit initialize callback */
static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    LOG("Device Initialized, Devid: %d", devid);

    /* Set the linkkit initialize success flag */
    if (user_example_ctx->master_devid == devid) {
        user_example_ctx->master_initialized = 1;
    }

    return 0;
}

#define MESSAGE_ALARM_FLAG_ID          "alarm_off"
#define MESSAGE_ALARM_SETTING_ID          "alarm_setting"
static aos_mutex_t mutex_message_callback;
void usr_message_arrive2(const int fd, const unsigned char * message, const int len)
{
    char  alarm_flag;
    char *payload = message;
    LOG("Message Arrived: %s",message);

	if (aos_mutex_lock(&mutex_message_callback, AOS_WAIT_FOREVER))
		return;
	int res = 0;
            lite_cjson_t lite, lite_item_id, lite_item_code, lite_item_devid;

           if (payload == NULL) {
		 aos_mutex_unlock(&mutex_message_callback);
                return;
            }

            /* Parse JSON */
            res = lite_cjson_parse(payload, strlen(payload), &lite);
            if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite)) {
		aos_mutex_unlock(&mutex_message_callback);
                return;
            }

            /* Parse Message ID */
            res = lite_cjson_object_item(&lite, MESSAGE_ALARM_FLAG_ID, strlen(MESSAGE_ALARM_FLAG_ID),
                                         &lite_item_id);
            if (res != SUCCESS_RETURN || !lite_cjson_is_number(&lite_item_id)) {
                //return;
            }
	    else
	    {
	    	alarm_clear = lite_item_id.value_int;
           	 LOG("lite_item_id.value_int : %d ,alarm_clear : %d", lite_item_id.value_int,alarm_clear);
	    }
            res = lite_cjson_parse(payload, strlen(payload), &lite);
            if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite)) {
                //return;
            }
            res = lite_cjson_object_item(&lite, MESSAGE_ALARM_SETTING_ID, strlen(MESSAGE_ALARM_SETTING_ID),
                                         &lite_item_id);
            if (res != SUCCESS_RETURN || !lite_cjson_is_number(&lite_item_id)) {
                //return;
            }
	    else
	    {
	    	 char  buffer[128]={0};
	    	 alarm_setting = (float)lite_item_id.value_int;
	         sprintf(buffer,"%f",alarm_setting);
		  if (alarm_setting > 20 && alarm_setting < 60)
		  	kv_item_set(MESSAGE_ALARM_SETTING_ID, buffer, strlen(buffer));
           	 LOG("lite_item_id.value_int %d ,alarm_setting : %f", lite_item_id.value_int,alarm_setting);
	    }
	aos_mutex_unlock(&mutex_message_callback);
}

extern float g_temp;
extern float g_humi;

/* sensor cloud test, include the following functions */
/* 1. wait wifi conect                                */
/* 2. link aliyun server                              */
/* 3. sensor start                                    */
/* 4. handle linkkit event                            */
void sensor_cloud_test(void *arg)
{
    int  ret = 0;
    static char param[128];

    iotx_linkkit_dev_meta_info_t  master_meta_info;
    user_example_ctx_t           *user_ctx = user_example_get_ctx();
#ifdef AOS_COMP_CLI
    aos_cli_register_command(&devinfo_cmd);
#endif
    (void)arg;

   aos_mutex_new(&mutex_message_callback);
    /* Wait the wifi connect flag to set */
    while (g_wifi_connect == 0) {
        static int cnt = 0;
        static int retry = 0;
        aos_msleep(100);
        if (cnt++ == 80)
        {
            printf("Connect AP failed , retry ....\n\n");
            netmgr_start(true);
            if (retry++ < 5)
            {
                cnt = 0;
            }
        }
        if (cnt > 200)
            cnt = 0;
    }

    /* Linkkit start */
    ret = linkkit_init();
    if (ret != 0){
        return;
    }

    /* Register linkkit initialize callback */
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
#if 0
    memcpy(master_meta_info.product_key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy(master_meta_info.product_secret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memcpy(master_meta_info.device_name, DEVICE_NAME, strlen(DEVICE_NAME));
    memcpy(master_meta_info.device_secret, DEVICE_SECRET, strlen(DEVICE_SECRET));

#else
    HAL_GetProductKey(master_meta_info.product_key);
    HAL_GetDeviceName(master_meta_info.device_name);
    HAL_GetDeviceSecret(master_meta_info.device_secret);
#endif

    /* Create a new device */
    user_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (user_ctx->master_devid < 0) {
        printf("IOT_Linkkit_Open Failed\n");
        return;
    }

    /* Start Connect Aliyun Server */
    ret = IOT_Linkkit_Connect(user_ctx->master_devid);
    if (ret < 0) {
        printf("IOT_Linkkit_Connect Failed\n");
        return;
    }



#if 0
    /* Open the acceleration sensor device */
    ret = sensor_hal_open(TAG_DEV_ACC, 0);
    if (ret != 0) {
        return;
    }
#endif

    {

	   static char  buff[128] =  "user/get";
	 static iotx_user_subscribe_context  context={buff,usr_message_arrive2};
        IOT_Ioctl(IOTX_IOCTL_SUB_USER_TOPIC,&context);
    }

    /* Enter loop run to handle linkkit event */
    while (1) {
        float temp;
        static bool bflag =false;
        /* Read the temperature sensor data */

        hal_gpio_output_low(&brd_gpio_table[GPIO_LED_3]);
        temp = g_temp;
        {
            /* Print the temperature sensor data */
            printf("\nTemperature value : %.1f centidegree\n", ((float)temp));

            memset(param, 0, 128);

            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_TEMP, ((float)(temp)));

            /* Report the temperature data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }
        }


         temp = g_humi;
        {
            /* Print the temperature sensor data */
            printf("\nHumi value : %.1f \n", ((float)temp));

            memset(param, 0, 128);

            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_HUMI, ((float)(temp)));

            /* Report the temperature data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }
        }


        bflag = !bflag;
        if(bflag)
        {
            /* Print the temperature sensor data */

            memset(param, 0, 128);
            char  alarmstatus = 0;
            if ((g_temp > alarm_setting) && (!alarm_clear))
                alarmstatus = 1;
            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_ALARMSTATUS, alarmstatus);

            /* Report the temperature data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }

        }
        hal_gpio_output_high(&brd_gpio_table[GPIO_LED_3]);        
        IOT_Linkkit_Yield(15000);

    }
}

