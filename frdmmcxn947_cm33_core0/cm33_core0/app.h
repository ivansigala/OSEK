/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_BUTTON_GPIO_INTERFACE  Driver_GPIO_PORT0
#define GPIO_0_LED                     Driver_GPIO_PORT0
#define GPIO_1_LED                     Driver_GPIO_PORT1
#define EXAMPLE_BUTTON_PIN            BOARD_SW3_GPIO_PIN
#define RED_LED_PIN               BOARD_LED_RED_GPIO_PIN
#define GREEN_LED_PIN           BOARD_LED_GREEN_GPIO_PIN
#define BLUE_LED_PIN             BOARD_LED_BLUE_GPIO_PIN
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
