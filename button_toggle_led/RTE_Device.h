/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

/* Driver name mapping. */
/* User needs to provide the implementation of GPIO set up pins for the enabled GPI instance. */
#define RTE_GPIO_PORT0 1
#define RTE_GPIO_PORT1 1

#define RTE_GPIO0_IRQ_HANDLER GPIO00_IRQHandler
#define RTE_GPIO0_IRQ         GPIO00_IRQn

#define RTE_GPIO1_IRQ_HANDLER GPIO01_IRQHandler
#define RTE_GPIO1_IRQ         GPIO01_IRQn

#endif /* _RTE_DEVICE_H */
