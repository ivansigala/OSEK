# CMSIS GPIO Button Toggle LED with OSEK Kernel

A real-time operating system kernel (OSEK) implementation for the NXP FRDM-MCXn947 board that manages multi-task execution with LED control via GPIO.

## Project Overview

This project demonstrates a lightweight OSEK (AUTOSAR Operating System Embedded Kernel) implementation running on the ARM Cortex-M33 core. The system creates three tasks with different priorities that execute sequentially to control RGB LEDs on the development board.

**Hardware:** NXP FRDM-MCXn947 (ARM Cortex-M33)  
**RTOS:** Custom OSEK Kernel with priority-based scheduling  
**Build System:** CMake (MCUXpresso compatible)  

---

## Task Execution Flow

```mermaid
graph TD
    A["Task A<br/>(Priority 1)<br/>Auto-start"] -->|Activates| B["Task B<br/>(Priority 3)"]
    A -->|Terminates Self| SCHED1["Scheduler"]
    B -->|Chains To| C["Task C<br/>(Priority 5)"]
    C -->|Terminates Self| SCHED2["Scheduler"]
    SCHED2 -->|No Ready Tasks| IDLE["Return to Main"]
    
    style A fill:#dc2626,color:#fff
    style B fill:#059669,color:#fff
    style C fill:#0c4a6e,color:#fff
    style SCHED1 fill:#d97706,color:#fff
    style SCHED2 fill:#d97706,color:#fff
    style IDLE fill:#4b5563,color:#fff
```

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

The kernel uses **manual stack-based context switching** with per-task stacks:

```mermaid
graph TB
    subgraph "Task Structure"
        A["Task Control Block<br/>- Priority<br/>- State<br/>- Stack Pointer<br/>- Function Pointer"]
    end
    
    subgraph "Stack Management"
        B["Task Stack<br/>512 bytes each"]
        C["Stack Frame<br/>R4-R11, LR"]
    end
    
    subgraph "Context Operations"
        D["OSEK_ContextSave<br/>Push R4-R11, LR"]
        E["OSEK_ContextRestore<br/>Pop R4-R11, PC"]
    end
    
    A --> B
    B --> C
    D -.->|ARM Assembly| C
    E -.->|ARM Assembly| C
    
    style A fill:#3b82f6,color:#fff
    style B fill:#f59e0b,color:#000
    style C fill:#10b981,color:#fff
    style D fill:#6b7280,color:#fff
    style E fill:#6b7280,color:#fff
```

### Stack Frame Structure

Each task's stack frame stores the CPU context:

```c
typedef struct {
    uint32_t r4;    // Callee-saved register
    uint32_t r5;    // Callee-saved register
    uint32_t r6;    // Callee-saved register
    uint32_t r7;    // Callee-saved register
    uint32_t r8;    // Callee-saved register
    uint32_t r9;    // Callee-saved register
    uint32_t r10;   // Callee-saved register
    uint32_t r11;   // Callee-saved register
    uint32_t lr;    // Return address / Program Counter
} OSEK_StackFrame_t;
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
        INIT["OSEK_Init()"]
        SCHED["OSEK_Scheduler()"]
        CTXSAVE["OSEK_ContextSave()"]
        CTXREST["OSEK_ContextRestore()"]
        ACTIVATE["OSEK_ActivateTask()"]
        TERMINATE["OSEK_TerminateTask()"]
        CHAIN["OSEK_ChainTask()"]
    end
    
    subgraph "Hardware & Data Structures"
        GPIO["GPIO Control"]
        POOL["Task Pool<br/>Static Allocation"]
        QUEUE["Task Linked List"]
    end
    
    APP --> INIT
    TASKA --> TERMINATE
    TASKA --> ACTIVATE
    TASKB --> CHAIN
    TASKC --> TERMINATE
    
    INIT --> SCHED
    SCHED --> CTXSAVE
    SCHED --> CTXREST
    ACTIVATE --> SCHED
    TERMINATE --> SCHED
    CHAIN --> SCHED
    
    SCHED -.-> QUEUE
    QUEUE -.-> POOL
    TASKA -.-> GPIO
    TASKB -.-> GPIO
    TASKC -.-> GPIO
    
    style APP fill:#3b82f6,color:#fff
    style TASKA fill:#dc2626,color:#fff
    style TASKB fill:#059669,color:#fff
    style TASKC fill:#0c4a6e,color:#fff
    style INIT fill:#d97706,color:#fff
    style SCHED fill:#d97706,color:#fff
    style GPIO fill:#6b7280,color:#fff
    style POOL fill:#f59e0b,color:#000
    style QUEUE fill:#f59e0b,color:#000
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
- MCUXpresso IDE
- ARM GCC Compiler
- CMake 3.20+

### Build Commands

**Using CMake:**
```bash
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

**Using MCUXpresso:**
1. Open MCUXpresso IDE
2. Import project: `File` → `Import` → `Existing Projects into Workspace`
3. Select the project folder
4. Build: `Project` → `Build All` or `Ctrl+B`

---

## Key Implementation Details

### 1. Static Task Pool Allocation
Instead of using `malloc()` for each task (which exhausts heap), tasks are allocated from a pre-allocated static pool:

```c
static OSEK_Task_t g_task_pool[MAX_TASKS];  // Compile-time allocation
static uint32_t g_task_pool_idx = 0;        // Track allocation index
```

### 2. Context Switching with Naked Assembly
Callee-saved registers (R4-R11) and return address (LR) are manually saved/restored using ARM inline assembly:

```c
__attribute__((naked)) static void OSEK_ContextSave(void) {
    __asm volatile (
        "push {r4-r11, lr}\n"
        "ldr r0, =g_actual_task_sp\n"
        "ldr r0, [r0]\n"
        "str sp, [r0, #20]\n"  // Save SP to task struct
        "bx lr\n"
    );
}
```

### 3. Priority-Based Scheduler
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
- **Fixed stack size:** All tasks have 512-byte stacks

### Recommended Improvements
- [ ] Add preemptive scheduling support (SysTick interrupt)
- [ ] Implement time-slicing for fairness
- [ ] Add semaphore/mutex support for task synchronization
- [ ] Dynamic stack size configuration
- [ ] Event/interrupt handling for external devices
- [ ] Task priority inheritance mechanisms

---

## Repository Setup

### .gitignore
```gitignore
# Build directories
debug/
release/
build/

# IDE files
.cproject
.project
.settings/
.mcuxpressoide_project_cache/

# CMake artifacts
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
compile_commands.json
```

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

- [AUTOSAR OSEK Specification](https://www.autosar.org/)
- [NXP MCXn947 Data Sheet](https://www.nxp.com/)
- [ARM Cortex-M Programming Guide](https://developer.arm.com/)
- [MCUXpresso IDE Documentation](https://www.nxp.com/mcuxpresso)

---

## License

Copyright 2024 NXP  
SPDX-License-Identifier: BSD-3-Clause

---

## Author

Diego - OSEK Kernel Implementation  
**Created:** January 24, 2026  
**Last Updated:** February 6, 2026
