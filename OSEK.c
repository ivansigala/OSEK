/*
 * OSEK.c
 *
 *  Created on: Jan 24, 2026
 *      Author: diego
 */

#include "OSEK.h"

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static OSEK_Task_t *g_actual_task_sp;
static OSEK_Task_t *g_head_task_sp; /*First task in the double linked list*/
static OSEK_TaskEntry_t g_task_to_run_pf;
static uint32_t g_main_link_register = 0;
static uint32_t g_main_stack_pointer = 0;
/*******************************************************************************
 * Functions
 ******************************************************************************/

/*
 * @brief initialize the first node of the TASKS run the scheduler
 * @param void
 * @return void
 * @note later need to be implemented arguments for configuration
*/
void OSEK_Init(void){

	__asm volatile (
        "mov r0, sp\n"
        "ldr r1, =g_main_stack_pointer\n"
        "str r0, [r1]\n"
        "mov r0, lr\n"
        "ldr r1, =g_main_link_register\n"
        "str r0, [r1]\n"
    );

	g_main_link_register = g_main_link_register; /* To avoid unused variable warning */
	g_main_stack_pointer = g_main_stack_pointer; /* To avoid unused variable warning */

	OSEK_Scheduler(); /* Call the scheduler */

}


/*
 * @brief Execute the task with the highest priority
 * @param void
 * @return void
 * @note
*/
void OSEK_Scheduler(void){


	OSEK_Task_t *search_iterator_sp       = g_head_task_sp;
	OSEK_Task_t *highest_priority_task_sp = NULL;
	uint8_t      max_priority_found_u8    = 0U;

	while (search_iterator_sp != NULL) {
		if (search_iterator_sp->state_e == READY) {
			if (search_iterator_sp->priority_u8 > max_priority_found_u8) {
				max_priority_found_u8 = search_iterator_sp->priority_u8;
				highest_priority_task_sp = search_iterator_sp;
			}
		}
		search_iterator_sp = search_iterator_sp->next_task_sp;
	}

	if (highest_priority_task_sp != NULL) {
		/* Check if no task is running OR if the new task is higher priority */
		if ((g_actual_task_sp == NULL) ||
			(highest_priority_task_sp->priority_u8 > g_actual_task_sp->priority_u8)) {

			g_actual_task_sp = highest_priority_task_sp;

			highest_priority_task_sp->state_e = RUNNING;

			g_task_to_run_pf = highest_priority_task_sp->task_addr_pf;

			if(!g_task_to_run_pf){
				PRINTF("Task function pointer is NULL\r\n");
				return;
			}

			// Load the function pointer and jump to it
			__asm volatile (
                "ldr r3, =g_task_to_run_pf\n"
                "ldr r3, [r3]\n"
                "blx r3\n"              // Use BLX to allow the task to return here
            );
		}

	}else {
		__asm volatile (
            "ldr r0, =g_main_stack_pointer\n"
            "ldr r0, [r0]\n"
            "mov sp, r0\n"
            "ldr r0, =g_main_link_register\n"
            "ldr r0, [r0]\n"
            "bx r0\n"
        );
	}
}


/*
 * @param task_fp is the pointer of the function to be executed by the task
 * @param task_priority_u8 is the priority of the task the higher the number the higher the priority
 * @param task_autostart_b either the task is auto started or not
 * @return pointer to the created task structure
 * @note
*/
OSEK_Task_t* OSEK_CreateTask(OSEK_TaskEntry_t task_name_fp, uint8_t task_priority_u8, bool task_autostart_b){

	OSEK_Task_t *new_task_sp = (OSEK_Task_t*)malloc(sizeof(OSEK_Task_t));
	OSEK_Task_t *iterator_task_sp;

	new_task_sp->autostart_b  = task_autostart_b;
	new_task_sp->priority_u8  = task_priority_u8;
	new_task_sp->task_addr_pf = task_name_fp;
	new_task_sp->state_e      = (task_autostart_b)? READY : SUSPENDED;
	new_task_sp->next_task_sp = NULL;

	if(g_head_task_sp == NULL){

		g_head_task_sp = new_task_sp;
		g_head_task_sp->prev_task_sp = NULL;
		g_head_task_sp->next_task_sp = NULL;

	} else {

		iterator_task_sp = g_head_task_sp;

		for(int i = 1; i < MAX_TASKS; i++){
			if(iterator_task_sp->next_task_sp == NULL){
				iterator_task_sp->next_task_sp = new_task_sp;
				new_task_sp->prev_task_sp      = iterator_task_sp;
				break;
			}
			iterator_task_sp = iterator_task_sp->next_task_sp;
		}

		if(iterator_task_sp->next_task_sp == NULL){
			PRINTF("Max. number of Tasks reached!, task couldn't be created. Tasks: %d\r\n", MAX_TASKS);
		}

	}

	return new_task_sp;

}

/*
 * @brief Change the state of the task to ready
 * @param task_name_sp pointer to the Task's struct
 * @return void
 * @note
*/
void OSEK_ActivateTask(OSEK_Task_t *task_name_sp){

	if(task_name_sp == NULL){
		PRINTF("Task empty\r\n");
		return;
	}

	g_actual_task_sp->state_e = READY;
	task_name_sp->state_e = READY;

	OSEK_Scheduler();

}

/*
 * @brief Change the state of the task to suspended
 * @param task_name_sp pointer to the Task's struct
 * @return void
 * @note
*/
void OSEK_TerminateTask(void){

	if(g_actual_task_sp == NULL){
		PRINTF("Task empty\r\n");
		return;
	}

	g_actual_task_sp->state_e = SUSPENDED;

	OSEK_Scheduler();

}

/*
 * @brief Change the state of the current task to suspended and the new task to ready
 * @param task_name_sp pointer to the Task's struct
 * @return void
 * @note Calls the scheduler after changing the states
*/
void OSEK_ChainTask(OSEK_Task_t *task_name_sp){

	if(task_name_sp == NULL){
		PRINTF("Task empty\r\n");
		return;
	}

	g_actual_task_sp->state_e = SUSPENDED;
	task_name_sp->state_e = READY;

	OSEK_Scheduler();

}

/*******************************************************************************
 * EOF
 ******************************************************************************/
