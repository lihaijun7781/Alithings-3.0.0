/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include "aos/kernel.h"
#include "ulog/ulog.h"

#include "aos/hal/gpio.h"



#define GPIO_LED_IO     45
#define GPIO_TRIGGER_IO 11
#define GPIO_INPUT_IO   41


#define GPIO_RED       45
#define GPIO_BLUE      43
#define GPIO_GREEN     42
static void app_trigger_low_action(void *arg);
static void app_trigger_high_action(void *arg);

gpio_dev_t led;
gpio_dev_t trigger;
gpio_dev_t input;
//add for st nucleo board
gpio_dev_t led_red;
gpio_dev_t led_green;
gpio_dev_t led_blue;
static void gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    uint32_t value = 0;
    hal_gpio_input_get(&input, &value);
    hal_gpio_output_toggle(&led);
    LOG("GPIO[%u] intr, val: %u\n", gpio_num, value);
} 

static void app_trigger_low_action(void *arg)
{
    hal_gpio_output_low(&trigger);
    aos_post_delayed_action(1000, app_trigger_high_action, NULL);
}

static void app_trigger_high_action(void *arg)
{
    hal_gpio_output_high(&trigger);
    aos_post_delayed_action(1000, app_trigger_low_action, NULL);
}

int application_start(int argc, char *argv[])
{
#ifdef STM32L496xx
    // developerkit 	
    /* gpio port config */
    led.port = GPIO_LED_IO;
    /* set as output mode */
    led.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&led);

    /* gpio port config */
    trigger.port = GPIO_TRIGGER_IO;
    /* set as output mode */
    trigger.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&trigger);

    /* input pin config */
    input.port = GPIO_INPUT_IO;
    /* set as interrupt mode */
    input.config = IRQ_MODE;
    /* configure GPIO with the given settings */
    hal_gpio_init(&input);

    /* gpio interrupt config */
    hal_gpio_enable_irq(&input, IRQ_TRIGGER_BOTH_EDGES, gpio_isr_handler, (void *) GPIO_INPUT_IO);

    aos_post_delayed_action(1000, app_trigger_low_action, NULL);
#else


    led_red.port = GPIO_RED;
    /* set as output mode */
    led_red.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&led_red);

    led_blue.port = GPIO_BLUE;
    /* set as output mode */
    led_blue.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&led_blue);

    led_green.port = GPIO_GREEN;
    /* set as output mode */
    led_green.config = OUTPUT_PUSH_PULL;
    /* configure GPIO with the given settings */
    hal_gpio_init(&led_green);
    int cnt = 0;
    while (1)
    {
      /* Insert delay 1000 ms */
        printf(" race led loop %d \n",cnt++);
        aos_msleep(333);
        hal_gpio_output_toggle(&led_green);
        aos_msleep(333);
        hal_gpio_output_toggle(&led_blue);
        aos_msleep(333);
        hal_gpio_output_toggle(&led_red);
    }
#endif
    aos_loop_run();
    return 0;
}
