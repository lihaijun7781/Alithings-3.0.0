/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef __HAL_GPIO_STM32L4_H
#define __HAL_GPIO_STM32L4_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32l4xx_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "aos/kernel.h"

#define PINS_IN_GROUP  16
#define GROUP_GPIOA    0
#define GROUP_GPIOB    1	 
#define GROUP_GPIOC    2
#define GROUP_GPIOD    3	 
#define GROUP_GPIOE    4
	 
#define HAL_GPIO_0                 ((uint8_t)0)        /* represent GPIOA pin 0 */
#define HAL_GPIO_1                 ((uint8_t)1)        /* represent GPIOA pin 1 */
#define HAL_GPIO_2                 ((uint8_t)2)        /* represent GPIOA pin 2 */
#define HAL_GPIO_3                 ((uint8_t)3)        /* represent GPIOA pin 3 */
#define HAL_GPIO_4                 ((uint8_t)4)        /* represent GPIOA pin 4 */
#define HAL_GPIO_5                 ((uint8_t)5)        /* represent GPIOA pin 5 */
#define HAL_GPIO_6                 ((uint8_t)6)        /* represent GPIOA pin 6 */
#define HAL_GPIO_7                 ((uint8_t)7)        /* represent GPIOA pin 7 */
#define HAL_GPIO_8                 ((uint8_t)8)        /* represent GPIOA pin 8 */
#define HAL_GPIO_9                 ((uint8_t)9)        /* represent GPIOA pin 9 */
#define HAL_GPIO_10                ((uint8_t)10)       /* represent GPIOA pin 10 */
#define HAL_GPIO_11                ((uint8_t)11)       /* represent GPIOA pin 11 */
#define HAL_GPIO_12                ((uint8_t)12)       /* represent GPIOA pin 12 */
#define HAL_GPIO_13                ((uint8_t)13)       /* represent GPIOA pin 13 */
#define HAL_GPIO_14                ((uint8_t)14)       /* represent GPIOA pin 14 */
#define HAL_GPIO_15                ((uint8_t)15)       /* represent GPIOA pin 15 */
	 
#define HAL_GPIO_16                ((uint8_t)16)       /* represent GPIOB pin 0 */
#define HAL_GPIO_17                ((uint8_t)17)       /* represent GPIOB pin 1 */
#define HAL_GPIO_18                ((uint8_t)18)       /* represent GPIOB pin 2 */
#define HAL_GPIO_19                ((uint8_t)19)       /* represent GPIOB pin 3 */
#define HAL_GPIO_20                ((uint8_t)20)       /* represent GPIOB pin 4 */
#define HAL_GPIO_21                ((uint8_t)21)       /* represent GPIOB pin 5 */
#define HAL_GPIO_22                ((uint8_t)22)       /* represent GPIOB pin 6 */
#define HAL_GPIO_23                ((uint8_t)23)       /* represent GPIOB pin 7 */
#define HAL_GPIO_24                ((uint8_t)24)       /* represent GPIOB pin 8 */
#define HAL_GPIO_25                ((uint8_t)25)       /* represent GPIOB pin 9 */
#define HAL_GPIO_26                ((uint8_t)26)       /* represent GPIOB pin 10 */
#define HAL_GPIO_27                ((uint8_t)27)       /* represent GPIOB pin 11 */
#define HAL_GPIO_28                ((uint8_t)28)       /* represent GPIOB pin 12 */
#define HAL_GPIO_29                ((uint8_t)29)       /* represent GPIOB pin 13 */
#define HAL_GPIO_30                ((uint8_t)30)       /* represent GPIOB pin 14 */
#define HAL_GPIO_31                ((uint8_t)31)       /* represent GPIOB pin 15 */

#define HAL_GPIO_32                ((uint8_t)32)       /* represent GPIOC pin 0 */
#define HAL_GPIO_33                ((uint8_t)33)       /* represent GPIOC pin 1 */
#define HAL_GPIO_34                ((uint8_t)34)       /* represent GPIOC pin 2 */
#define HAL_GPIO_35                ((uint8_t)35)       /* represent GPIOC pin 3 */
#define HAL_GPIO_36                ((uint8_t)36)       /* represent GPIOC pin 4 */
#define HAL_GPIO_37                ((uint8_t)37)       /* represent GPIOC pin 5 */
#define HAL_GPIO_38                ((uint8_t)38)       /* represent GPIOC pin 6 */
#define HAL_GPIO_39                ((uint8_t)39)       /* represent GPIOC pin 7 */
#define HAL_GPIO_40                ((uint8_t)40)       /* represent GPIOC pin 8 */
#define HAL_GPIO_41                ((uint8_t)41)       /* represent GPIOC pin 9 */
#define HAL_GPIO_42                ((uint8_t)42)       /* represent GPIOC pin 10 */
#define HAL_GPIO_43                ((uint8_t)43)       /* represent GPIOC pin 11 */
#define HAL_GPIO_44                ((uint8_t)44)       /* represent GPIOC pin 12 */
#define HAL_GPIO_45                ((uint8_t)45)       /* represent GPIOC pin 13 */
#define HAL_GPIO_46                ((uint8_t)46)       /* represent GPIOC pin 14 */
#define HAL_GPIO_47                ((uint8_t)47)       /* represent GPIOC pin 15 */	
#define ALS_INT                    HAL_GPIO_15     /*PA15*/
#define LED_1                      HAL_GPIO_45     /*PC13*/ //R led
#define AUDIO_CTL                  HAL_GPIO_19     /*PD5*/
#define AUDIO_RST                  HAL_GPIO_20     /*PD6*/
#define AUDIO_WU                   HAL_GPIO_21     /*PD4*/
#define CAM_PD                     HAL_GPIO_19     /*PE13*/
#define CAM_RST                    HAL_GPIO_19     /*PB8*/
#define LED_3                      HAL_GPIO_43     /*PC11*/ //B led
#define LED_2                      HAL_GPIO_42     /*PC10*/ //G led
#define HTS_LED                    HAL_GPIO_22     /*PD11*/
#define IR_IN                      HAL_GPIO_22     /*PE9*/
#define IR_OUT                     HAL_GPIO_25     /*PB9*/
#define KEY_1                      HAL_GPIO_41     /*PC9*/
#define KEY_2                      HAL_GPIO_11     /*PE14*/
#define KEY_3                      HAL_GPIO_41     /*PE10*/
#define LCD_DCX                    HAL_GPIO_16      /*PB0*/
#define LCD_PWR                    HAL_GPIO_18     /*PB2*/
#define LCD_RST                    HAL_GPIO_17     /*PB1*/
#define PCIE_RST                   HAL_GPIO_45     /*PC13*/
#define SECURE_CLK                 HAL_GPIO_22     /*PE15*/
#define SECURE_IO                  HAL_GPIO_0      /*PA0*/
#define SECURE_RST                 HAL_GPIO_1      /*PA1*/
#define SIM_DET                    HAL_GPIO_22     /*PE2*/
#define USB_PCIE_SW                HAL_GPIO_21     /*PB5*/
#define WIFI_RST                   HAL_GPIO_16     /*PB0*/
#define WIFI_WU                    HAL_GPIO_17     /*PB1*/
#define ZIGBEE_INT                 HAL_GPIO_22     /*PE8*/
#define ZIGBEE_RST                 HAL_GPIO_22     /*PD7*/
#define BEEP_PIN                   HAL_GPIO_12
#define GPIOA_SPEED	 GPIO_SPEED_FREQ_VERY_HIGH

#ifdef __cplusplus
}
#endif

#endif /* __HAL_GPIO_STM32L4_H */
