/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_gpio_cmsis.h"
#include "app.h"
#include "OSEK.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void task_a(void);
static void task_b(void);
static void task_c(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Whether the SW button is pressed */
volatile bool g_ButtonPress = false;
uint32_t LEDLevel = LOGIC_LED_OFF;

/* Task pointers */
OSEK_Task_t *task_a_ptr = NULL;
OSEK_Task_t *task_b_ptr = NULL;
OSEK_Task_t *task_c_ptr = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BUTTON_EventCallback(uint32_t pin, uint32_t event)
{
    if (pin == EXAMPLE_BUTTON_PIN && event == ARM_GPIO_TRIGGER_FALLING_EDGE)
    {
        g_ButtonPress = true;
        PRINTF("\r\nBUTTON Pressed! \r\n");
    }
}

/*!
 * @brief Task A - Turns on the red led, activates task B and terminates itself
 */
static void task_a(void)
{
    PRINTF("\r\nTask A is running!\r\n");

    GPIO_0_LED.SetOutput(RED_LED_PIN, LOGIC_LED_ON);

    OSEK_ActivateTask(task_b_ptr);
    OSEK_TerminateTask();
}

/*!
 * @brief Task B - Turns on the green led and chains to Task C
 */
static void task_b(void)
{
    PRINTF("\r\nTask B is running!\r\n");
    
    GPIO_0_LED.SetOutput(RED_LED_PIN, LOGIC_LED_OFF);
    GPIO_0_LED.SetOutput(GREEN_LED_PIN, LOGIC_LED_ON);

    OSEK_ChainTask(task_c_ptr);
}

/*!
 * @brief Task C - Turns on the blue led and terminates itself
 */
static void task_c(void)
{
    PRINTF("\r\nTask C is running!\r\n");

    GPIO_0_LED.SetOutput(GREEN_LED_PIN, LOGIC_LED_OFF);
    GPIO_1_LED.SetOutput(BLUE_LED_PIN, LOGIC_LED_ON);

    OSEK_TerminateTask();
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitHardware();

    PRINTF("\r\nCMSIS GPIO Example! \r\n");
    PRINTF("\r\nUse Button to toggle LED! \r\n");

    /* BUTTON pin set up */
    EXAMPLE_BUTTON_GPIO_INTERFACE.Setup(EXAMPLE_BUTTON_PIN, BUTTON_EventCallback);
    EXAMPLE_BUTTON_GPIO_INTERFACE.SetEventTrigger(EXAMPLE_BUTTON_PIN, ARM_GPIO_TRIGGER_FALLING_EDGE);
   
    /* LED pin set up */
    /* RED */
    GPIO_0_LED.Setup(RED_LED_PIN, NULL);
    GPIO_0_LED.SetDirection(RED_LED_PIN, ARM_GPIO_OUTPUT);
    GPIO_0_LED.SetOutput(RED_LED_PIN, LOGIC_LED_OFF);

    /*  */
    GPIO_0_LED.Setup(GREEN_LED_PIN, NULL);
    GPIO_0_LED.SetDirection(GREEN_LED_PIN, ARM_GPIO_OUTPUT);
    GPIO_0_LED.SetOutput(GREEN_LED_PIN, LOGIC_LED_OFF);

    /* RED */
    GPIO_1_LED.Setup(BLUE_LED_PIN, NULL);
    GPIO_1_LED.SetDirection(BLUE_LED_PIN, ARM_GPIO_OUTPUT);
    GPIO_1_LED.SetOutput(BLUE_LED_PIN, LOGIC_LED_OFF);

    /* Create and start Task A with priority 5, auto-start enabled */
    task_a_ptr = OSEK_CreateTask(task_a, 1, true);
    task_b_ptr = OSEK_CreateTask(task_b, 3, false);
    task_c_ptr = OSEK_CreateTask(task_c, 5, false);

    OSEK_Init();

    while (1)
    {
 
    }
}
