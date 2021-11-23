
#ifndef SYS_OSTYPE_H_
#define SYS_OSTYPE_H_

typedef enum {
    ENTER_ZOMBIE_ON_EXIT,
    AUTO_CLEANUP_ON_EXIT,
} spawn_mode_t;

typedef enum {
    KERNEL_PROCESS,
    KERNEL_THREAD,
    USER_PROCESS,
    USER_THREAD,
} task_type_t;

typedef enum {
    P_0,
    P_1,
    P_2,
    P_3,
    P_4,
    P_5,
    P_6,
    P_7,
    P_8,
    P_9,
} task_priority_t;
typedef struct task_info
{
    ptr_t entry_point;
    task_type_t type;
    task_priority_t priority;
} task_info_t;
#endif
