# CMSIS GPIO Button Toggle LED with OSEK Kernel

A real-time operating system kernel (OSEK) implementation for the NXP FRDM-MCXn947 board that manages multi-task execution with LED control via GPIO.

## Project Overview

This project demonstrates a lightweight OSEK implementation running on the ARM Cortex-M33 core. The system creates three tasks with different priorities that execute sequentially to control RGB LEDs on the development board.

**Hardware:** NXP FRDM-MCXN947 (ARM Cortex-M33)  
**RTOS:** Custom OSEK Kernel with priority-based scheduling  
**Build System:** CMake 

---

### Task Details

| Task | Priority | State | Function |
|------|----------|-------|----------|
| **Task A** | 1 (Lowest) | Auto-start | Turns ON red LED, activates Task B |
| **Task B** | 3 (Mid) | Suspended | Turns OFF red, turns ON green LED, chains to Task C |
| **Task C** | 5 (Highest) | Suspended | Turns OFF green, turns ON blue LED |

---

## Task State Machine

Each task transitions through different states during its lifecycle:

```mermaid
stateDiagram-v2
    [*] --> SUSPENDED: Created<br/>autostart=false
    [*] --> READY: Created<br/>autostart=true
    
    SUSPENDED --> READY: ActivateTask()
    READY --> RUNNING: Scheduler<br/>selects task
    RUNNING --> READY: ActivateTask()<br/>other task
    RUNNING --> SUSPENDED: TerminateTask()<br/>ChainTask()
    SUSPENDED --> [*]: Task exits
    
    note right of READY
        Task waiting for
        CPU time
    end note
    
    note right of RUNNING
        Task executing
        on CPU
    end note
```

---

## Priority-Based Scheduling

The scheduler implements a **non-preemptive** priority-based algorithm:

```mermaid
graph LR
    A["Ready Queue"] -->|Find highest<br/>priority task| B["Scheduler"]
    B -->|No task ready| C["Return to Main"]
    B -->|Task found| D["Save Current<br/>Context"]
    D -->|Load New<br/>Context| E["Execute Task"]
    E -->|Task yields| B
    
    style A fill:#3b82f6,color:#fff
    style B fill:#d97706,color:#fff
    style D fill:#6b7280,color:#fff
    style E fill:#10b981,color:#fff
    style C fill:#4b5563,color:#fff
```

**Scheduling Rules:**
- Higher priority number = higher priority
- Task with **highest priority** executes first
- Task executes until it calls `OSEK_TerminateTask()` or `OSEK_ChainTask()`
- Scheduler invoked after task yields

---

## Context Switching Architecture

The kernel preserves the main() context at startup and uses **ARM BLX instruction** for task invocation, allowing tasks to return control to the scheduler:

```mermaid
graph TB
    subgraph "Startup Context"
        A["OSEK_Init() saves:<br/>- Main Stack Pointer<br/>- Main Link Register"]
    end
    
    subgraph "Task Execution"
        B["BLX r3 calls task<br/>(r3 = task function ptr)"]
        C["Task code executes<br/>(using main stack)"]
    end
    
    subgraph "Task Return"
        D["Task calls<br/>TerminateTask()/<br/>ChainTask()"]
        E["Control returns to<br/>OSEK_Scheduler()"]
    end
    
    subgraph "Idle Handling"
        F["No ready tasks?<br/>Restore main context"]
        G["Return to main()"]
    end
    
    A --> B
    B --> C
    C --> D
    D --> E
    E -->|More tasks ready| B
    E -->|No tasks ready| F
    F --> G
    
    style A fill:#3b82f6,color:#fff
    style B fill:#d97706,color:#fff
    style C fill:#10b981,color:#fff
    style D fill:#f59e0b,color:#000
    style E fill:#6b7280,color:#fff
    style F fill:#dc2626,color:#fff
    style G fill:#059669,color:#fff
```

### Context Preservation Strategy

Instead of per-task stacks, the kernel uses a **main context preservation** approach:

```c
// Saved at startup in OSEK_Init()
static uint32_t g_main_stack_pointer = 0;  // SP of main()
static uint32_t g_main_link_register = 0;  // Return address to main()

// Task invocation via BLX (Branch with Link and Exchange)
// Automatically: LR = return address, jumps to task
// Task return: BX LR returns to OSEK_Scheduler()
```

---

## Kernel Architecture

```mermaid
graph TD
    subgraph "Application Layer"
        APP["main()"]
        TASKA["Task A"]
        TASKB["Task B"]
        TASKC["Task C"]
    end
    
    subgraph "OSEK Kernel"
        INIT["OSEK_Init()<br/>Saves main context"]
        SCHED["OSEK_Scheduler()<br/>Find & execute task"]
        ACTIVATE["OSEK_ActivateTask()"]
        TERMINATE["OSEK_TerminateTask()"]
        CHAIN["OSEK_ChainTask()"]
    end
    
    subgraph "Hardware & Data Structures"
        GPIO["GPIO Control"]
        QUEUE["Task Linked List"]
        BLX["Assembly calls"]
    end
    
    style APP fill:#3b82f6,color:#fff
    style TASKA fill:#dc2626,color:#fff
    style TASKB fill:#059669,color:#fff
    style TASKC fill:#0c4a6e,color:#fff
    style INIT fill:#d97706,color:#fff
    style SCHED fill:#d97706,color:#fff
    style GPIO fill:#6b7280,color:#fff
    style QUEUE fill:#f59e0b,color:#000
    style BLX fill:#10b981,color:#fff
```

---

## Task Execution Sequence

Detailed execution timeline showing how tasks flow through the system:

```mermaid
sequenceDiagram
    participant Main as main()
    participant Init as OSEK_Init()
    participant Sched as Scheduler
    participant TaskA as Task A
    participant TaskB as Task B
    participant TaskC as Task C
    
    Main->>Init: Call OSEK_Init()
    Init->>Sched: Call OSEK_Scheduler()
    
    Sched->>TaskA: Execute (Priority 1, Ready)
    TaskA->>TaskA: GPIO_0_LED.SetOutput(RED, ON)
    TaskA->>Sched: OSEK_ActivateTask(TaskB)
    TaskA->>Sched: OSEK_TerminateTask()
    
    Sched->>Sched: Find highest priority ready task
    Sched->>TaskB: Execute (Priority 3, Ready)
    TaskB->>TaskB: GPIO_0_LED.SetOutput(RED, OFF)
    TaskB->>TaskB: GPIO_0_LED.SetOutput(GREEN, ON)
    TaskB->>Sched: OSEK_ChainTask(TaskC)
    
    Sched->>TaskC: Execute (Priority 5, Ready)
    TaskC->>TaskC: GPIO_0_LED.SetOutput(GREEN, OFF)
    TaskC->>TaskC: GPIO_1_LED.SetOutput(BLUE, ON)
    TaskC->>Sched: OSEK_TerminateTask()
    
    Sched->>Sched: No ready tasks
    Sched->>Main: Return to main()
```

---

## LED Output Timeline

```mermaid
timeline
    title LED State Changes During Task Execution
    
    Task A : Red LED ON
             
    Task B : Red LED OFF
           : Green LED ON
           
    Task C : Green LED OFF
           : Blue LED ON
           
    Idle   : All LEDs OFF
           : Waiting for next trigger
```

---

## File Structure

```
.
├── cmsis_button_toggle_led.c    # Application entry point & task definitions
├── OSEK.c                        # Kernel implementation (scheduler, context switching)
├── OSEK.h                        # Kernel headers & data structures
├── CMakeLists.txt               # Build configuration
├── CMakePresets.json            # CMake presets for MCUXpresso
├── button_toggle_led/           # Board configuration
├── frdmmcxn947_cm33_core0/       # Device-specific configuration
└── debug/                       # Build output (excluded from git)
```

---

## Building the Project

### Prerequisites
- MCUXpresso extension for VSCode
- ARM GCC Compiler
- CMake 3.20+
- (All installed with the VSCode extension)


---

## Key Implementation Details

### 1. Main Context Preservation at Startup
The kernel saves the main() execution context during initialization:

```c
void OSEK_Init(void) {
    __asm volatile (
        "mov r0, sp\n"
        "ldr r1, =g_main_stack_pointer\n"
        "str r0, [r1]\n"               // Save main's SP
        "mov r0, lr\n"
        "ldr r1, =g_main_link_register\n"
        "str r0, [r1]\n"               // Save main's LR (return address)
    );
    OSEK_Scheduler();
}
```

### 2. Task Invocation via BLX Instruction
Tasks are called using the ARM **BLX** (Branch with Link and eXchange) instruction, which automatically saves the return address:

```c
__asm volatile (
    "ldr r3, =g_task_to_run_pf\n"      // Load task function pointer
    "ldr r3, [r3]\n"
    "blx r3\n"                         // Call task, LR = return address
);
// Control returns here when task returns
```

When the task calls `OSEK_TerminateTask()` or `OSEK_ChainTask()`, execution returns to the scheduler loop.

### 3. Idle State Recovery
When no tasks are ready, the kernel restores main() context to return cleanly:

```c
if (highest_priority_task_sp == NULL) {
    __asm volatile (
        "ldr r0, =g_main_stack_pointer\n"
        "ldr r0, [r0]\n"
        "mov sp, r0\n"                 // Restore main's SP
        "ldr r0, =g_main_link_register\n"
        "ldr r0, [r0]\n"
        "bx r0\n"                      // Return to main()
    );
}
```

### 4. Priority-Based Scheduler
The scheduler scans the ready task list and executes the highest priority task:

```c
uint8_t max_priority_found = 0;
OSEK_Task_t *highest_task = NULL;

// Find highest priority ready task
for each task in ready queue:
    if task.priority > max_priority_found:
        max_priority_found = task.priority
        highest_task = task
```

---

## API Reference

### Task Management

| Function | Description |
|----------|-------------|
| `OSEK_Init()` | Initialize kernel and start scheduler |
| `OSEK_CreateTask()` | Create a new task with priority and auto-start flag |
| `OSEK_ActivateTask()` | Move suspended task to ready state |
| `OSEK_TerminateTask()` | Terminate current running task |
| `OSEK_ChainTask()` | Terminate current task and activate another |

### Example Usage

```c
// Create tasks
task_a_ptr = OSEK_CreateTask(task_a, 1, true);   // Priority 1, auto-start
task_b_ptr = OSEK_CreateTask(task_b, 3, false);  // Priority 3, suspended
task_c_ptr = OSEK_CreateTask(task_c, 5, false);  // Priority 5, suspended

// Initialize and start kernel
OSEK_Init();
```

---

## Limitations & Future Improvements

### Current Limitations
- **Non-preemptive scheduling:** Tasks cannot be interrupted (must yield)
- **No time slicing:** No round-robin scheduling
- **Limited to 10 tasks:** `MAX_TASKS` defined at compile-time
- **Single shared stack:** All tasks share the main() stack (potential overflow risk with deep nesting)
- **Dynamic allocation:** Uses `malloc()` for task creation (limited by heap size)

### Recommended Improvements
- [ ] Add preemptive scheduling support (SysTick interrupt)
- [ ] Add semaphore/mutex support for task synchronization
- [ ] Implement task stack allocation for isolation
- [ ] Event/interrupt handling for external devices

---

## Repository Setup

### Initial Commit
```bash
git init
git add .
git commit -m "Initial commit: OSEK kernel with task scheduling"
git remote add origin https://github.com/yourusername/repo.git
git push -u origin main
```

---

## References

- [NXP MCXn947 Data Sheet](https://docs.nxp.com/bundle/UM12018/page/topics/related_documentation.html)
- [ARM Cortex-M  Assembly Programming Guide](https://developer.arm.com/documentation/100235/0003/the-cortex-m33-instruction-set/cortex-m33-instructions)

---

## License

Copyright 2024 NXP  
SPDX-License-Identifier: BSD-3-Clause

---

## Author

Diego & Romulo - OSEK Kernel Implementation  
**Created:** January 24, 2026  
**Last Updated:** February 6, 2026
