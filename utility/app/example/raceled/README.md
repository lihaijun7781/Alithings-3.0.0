# raceled sample

## Contents

```sh
raceled
├── raceled.c         # raceled source code
├── Config.in       # kconfig file
├── raceled.mk        # aos build system file
└── k_app_config.h  # aos app config file
```

## Introduction

The **raceled** example shows how to drive LEDs and use of GPIO input with interrupts on the [supported boards](../../../board) in AliOS-Things, the example will work like this:
* RGB led loop every 1s.
* push button will turn raceled on/off.

### Requirements

in `raceled.c` need to redefine the following macro:
* `GPIO_RED  `(LED red)
* `GPIO_BLUE`(LED blue)
* `GPIO_GREEN`(LED green)

### Features

* RGB LED run one loop every 1s.
* push button will turn raceled on/off.

### Dependencies

* yloop
* cli

### Supported Boards

- all

### Build

```sh
# generate raceled@everylinked default config
aos make raceled@everylinked -c config

# or customize config manually
aos make menuconfig

# build
aos make
```

> if you want to see AliOS-Things supports boards, click [board](../../../board).

### Install

```sh
aos upload raceled@yourboard
```

> if you are not sure is the`aos upload` command supports your board, check [aos upload](../../../build/site_scons/upload).

### Reference

* https://yq.aliyun.com/articles/669088

### support for st nucleo board
* verified on stm32l476rg everylink board, other nucleo also can be supported, please refer to the hardware guide for the gpio pin number
