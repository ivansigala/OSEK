/*
 * OSEK.h
 *
 *  Created on: Jan 24, 2026
 *      Author: diego
 */

#ifndef OSEK_H_
#define OSEK_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fsl_debug_console.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

#define MAX_TASKS 10

typedef enum{
	SUSPENDED = 0,
	READY,
	RUNNING,
	WAITING
}OSEK_TaskState_t;

typedef void (*OSEK_TaskEntry_t)(void);

typedef struct OSEK_Task_s{

	uint8_t          priority_u8;
	bool             autostart_b;
	OSEK_TaskState_t state_e;
	OSEK_TaskEntry_t task_addr_pf;

	uint32_t         *stack_ptr_pu32;

	struct OSEK_Task_s *prev_task_sp;
	struct OSEK_Task_s *next_task_sp;

}OSEK_Task_t;


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void OSEK_Init(void);
void OSEK_Scheduler(void);
OSEK_Task_t* OSEK_CreateTask(OSEK_TaskEntry_t task_entry_pf, uint8_t task_priority_u8, bool task_autostart_b);

void OSEK_ActivateTask(OSEK_Task_t *task_name_sp);
void OSEK_TerminateTask(void);
void OSEK_ChainTask(OSEK_Task_t *task_name_sp);

#endif /* OSEK_H_ */
