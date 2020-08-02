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

#define SENSOR_SAMPLE_TIME 1000 /* sensor sampling period is 1000 ms*/

#define PROP_POST_FORMAT_TEMP "{\"CurrentTemperature\":%.1f}"
#define PROP_POST_FORMAT_HUMI "{\"CurrentHumidity\":%.1f}"
#define PROP_POST_FORMAT_ACC  "{\"Accelerometer\":{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f}}"

static volatile char g_wifi_connect = 0; /* wifi connect flag */


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


/* wifi connect success callback */
static void wifi_service_event(input_event_t *event, void *priv_data)
{
    netmgr_ap_config_t config;

    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    memset(&config, 0, sizeof(netmgr_ap_config_t));
    netmgr_get_ap_config(&config);
    LOG("wifi_service_event config.ssid %s", config.ssid);
    if (strcmp(config.ssid, "adha") == 0 || strcmp(config.ssid, "aha") == 0) {
        return;
    }

#ifdef EN_COMBO_NET
    if (awss_running) {
        awss_success_notify();
    }
#endif

    /* Set wifi connect flag */
    if (g_wifi_connect == 0) {
        g_wifi_connect = 1;
    }
}

/* sensor cloud test, include the following functions */
/* 1. wait wifi conect                                */
/* 2. link aliyun server                              */
/* 3. sensor start                                    */
/* 4. handle linkkit event                            */
void sensor_cloud_test(void *arg)
{
    int  ret = 0;
    char param[128];

    temperature_data_t temp;
    accel_data_t       acc;

    iotx_linkkit_dev_meta_info_t  master_meta_info;
    user_example_ctx_t           *user_ctx = user_example_get_ctx();

    (void)arg;

    /* Wait the wifi connect flag to set */
    while(g_wifi_connect == 0){
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


    /* Sensor Hal start */
    ret = sensor_hal_init();
    if (ret != 0) {
        return;
    }

#if 0
    /* Open the acceleration sensor device */
    ret = sensor_hal_open(TAG_DEV_ACC, 0);
    if (ret != 0) {
        return;
    }
#endif
    /* Open the temperature sensor device */
    ret = sensor_hal_open(TAG_DEV_TEMP, 0);
    if (ret != 0) {
        return;
    }

    /* Set the sampling period for sensors */
   // (void)sensor_hal_ioctl(TAG_DEV_ACC, 0, SENSOR_IOCTL_ODR_SET, SENSOR_SAMPLE_TIME);
    (void)sensor_hal_ioctl(TAG_DEV_TEMP, 0, SENSOR_IOCTL_ODR_SET, SENSOR_SAMPLE_TIME);


    /* Enter loop run to handle linkkit event */
    while (1) {
#if 0
        /* Read the acceleration sensor data */
        ret = sensor_hal_read(TAG_DEV_ACC, 0, &acc, sizeof(acc));
        if (ret > 0) {
            /* Print the acceleration sensor data */
            printf("\nAcceleration value: x-axis(%.3f g) y-axis(%.3f g) z-axis(%.3f g) \n", ((float)acc.data[0])/1000 , ((float)acc.data[1])/1000, ((float)acc.data[2])/1000);

            memset(param, 0, 128);

            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_ACC, ((float)acc.data[0])/1000 , ((float)acc.data[1])/1000, ((float)acc.data[2])/1000);

            /* Report the acceleration data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }

        }
#endif 
        /* Read the temperature sensor data */
        ret = sensor_hal_read(TAG_DEV_TEMP, 0, &temp, sizeof(temp));
        if (ret > 0) {
            /* Print the temperature sensor data */
            printf("\nTemperature value : %.1f centidegree\n", ((float)temp.t)/10);

            memset(param, 0, 128);

            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_TEMP, ((float)(temp.t))/10);

            /* Report the temperature data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }
        }


        ret = sensor_hal_read(TAG_DEV_HUMI, 0, &temp, sizeof(temp));
        if (ret > 0) {
            /* Print the temperature sensor data */
            printf("\nHumi value : %.1f \n", ((float)temp.t)/10);

            memset(param, 0, 128);

            /* build the report payload */
            sprintf(param, PROP_POST_FORMAT_HUMI, ((float)(temp.t))/10);

            /* Report the temperature data to cloud */
            if (user_ctx->master_initialized != 0) {
                ret = IOT_Linkkit_Report(user_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)param, strlen(param) + 1);
                if (ret == -1) {
                    LOG("%s %d fail\n", __func__,__LINE__);
                }
            }
        }
        IOT_Linkkit_Yield(2000);
    }
}

int application_start(int argc, char **argv)
{
    int ret;

#ifdef WITH_SAL
    sal_add_dev(NULL, NULL);
    /* Sal initialize if needed */
    sal_init();
#endif

#ifdef AOS_COMP_CLI
    aos_cli_register_command(&devinfo_cmd);
#endif
    /* Set debug log show */
    aos_set_log_level(AOS_LL_DEBUG);

    /* Wifi initialize                                         */
    /* User can use the following cli commands to connect wifi */
    /* Clear wifi command:    netmgr clear                     */
    /* Connect wifi command:  netmgr connect ssid passwd       */
    netmgr_init();

    /* Register wifi connect callback */
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);

    /* Creat task for sensor cloud test */
    ret = aos_task_new("sensor_cloud_test", sensor_cloud_test, NULL, 8192);
    if (ret != 0) {
        return -1;
    }

    /* Connect wifi with old ssid and passwd */
    netmgr_start(true);

    /* Enter yloop */
    aos_loop_run();

    return 0;
}


