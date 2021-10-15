#include "test.h"

/* [TASK1] [TASK3] task group to test do_scheduler() */
// do_scheduler() annotations are required for non-robbed scheduling
struct task_info task2_1 = {(ptr_t)&print_task1, KERNEL_THREAD, P_0};
struct task_info task2_2 = {(ptr_t)&print_task2, KERNEL_THREAD, P_0};
struct task_info task2_3 = {(ptr_t)&drawing_task, KERNEL_THREAD, P_0};
struct task_info *sched1_tasks[16] = {&task2_1, &task2_2, &task2_3};
const int num_sched1_tasks = 3;

/* [TASK2] task group to test lock */
// test_lock1.c : Kernel space lock test
// test_lock2.c : User space lock test
struct task_info task2_4 = {(ptr_t)&lock_task1, KERNEL_THREAD, P_0};
struct task_info task2_5 = {(ptr_t)&lock_task2, KERNEL_THREAD, P_0};
struct task_info *lock_tasks[16] = {&task2_4, &task2_5};
const int num_lock_tasks = 2;

struct task_info *sched1_lock_tasks[16] = {&task2_1, &task2_2, &task2_3, &task2_4, &task2_5};
const int num_sched1_lock_tasks = num_sched1_tasks + num_lock_tasks;

/* [TASK4] task group to test interrupt */
// When the task is running, please implement the following system call :
// (1) sys_sleep()
// (2) sys_move_cursor()
// (3) sys_write()
// timer interrupt included!
struct task_info task2_6 = {(ptr_t)&sleep_task, USER_PROCESS, P_0};
struct task_info task2_7 = {(ptr_t)&timer_task, USER_PROCESS, P_0};
struct task_info *timer_tasks[16] = {&task2_6, &task2_7};
const int num_timer_tasks = 2;

struct task_info task2_8 = {(ptr_t)&print_task1, USER_PROCESS, P_0};
struct task_info task2_9 = {(ptr_t)&print_task2, USER_PROCESS, P_0};
struct task_info task2_10 = {(ptr_t)&drawing_task, USER_PROCESS, P_7};
struct task_info *sched2_tasks[16] = {&task2_8, &task2_9, &task2_10};
const int num_sched2_tasks = 3;

struct task_info task2_11 = {(ptr_t)&lock_task1, USER_THREAD, P_0};
struct task_info task2_12 = {(ptr_t)&lock_task2, USER_THREAD, P_0};
struct task_info *lock2_tasks[16] = {&task2_11, &task2_12};
const int num_lock2_tasks = 2;

struct task_info *sched2_lock_tasks[16] = {&task2_8, &task2_9, &task2_10, &task2_11, &task2_12};
const int num_sched2_lock_tasks = num_sched2_tasks + num_lock2_tasks;

struct task_info *timer_sched2_lock_tasks[16] = {&task2_6, &task2_7, &task2_8, &task2_9, &task2_10, &task2_11, &task2_12};
const int num_timer_sched2_lock_tasks = num_timer_tasks + num_sched2_tasks + num_lock2_tasks;

/* [TASK5] task group to test priority scheduling and fork */
// added the following syscalls:
// (1) sys_priority()
// (2) sys_fork()
struct task_info task2_13 = {(ptr_t)&priority_task0, USER_THREAD, P_0};
struct task_info task2_14 = {(ptr_t)&priority_task1, USER_THREAD, P_0};
struct task_info task2_15 = {(ptr_t)&priority_task2, USER_THREAD, P_0};
struct task_info task2_16 = {(ptr_t)&priority_task3, USER_THREAD, P_0};
struct task_info *priority_tasks[16] = {&task2_13, &task2_14, &task2_15, &task2_16};
const int num_priority_tasks = 4;

struct task_info task2_17 = {(ptr_t)&fork_task, USER_THREAD, P_0};
struct task_info *fork_tasks[16] = {&task2_17};
const int num_fork_tasks = 1;

struct task_info *priority_fork_tasks[16] = {&task2_13, &task2_14, &task2_15, &task2_16, &task2_17};
const int num_priority_fork_tasks = num_priority_tasks + num_fork_tasks;

struct task_info *all_tasks[16] = {&task2_6, &task2_7, &task2_8, &task2_9, &task2_10, &task2_11, &task2_12, &task2_13,
								   &task2_14, &task2_15, &task2_16, &task2_17};
const int num_all_tasks = num_timer_tasks + num_sched2_tasks + num_lock2_tasks + num_priority_tasks + num_fork_tasks;
