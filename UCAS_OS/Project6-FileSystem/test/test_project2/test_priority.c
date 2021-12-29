#include <stdio.h>
#include <sys/syscall.h>

void priority_task0(void)
{
    unsigned long i;
    int print_location = 7;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        sys_priority(P_0);
        printf("> [TASK] This task is to test priority P_0. (%lu)", i);
    }
}

void priority_task1(void)
{
    unsigned long i;
    int print_location = 8;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        sys_priority(P_1);
        printf("> [TASK] This task is to test priority P_1. (%lu)", i);
    }
}

void priority_task2(void)
{
    unsigned long i;
    int print_location = 9;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        sys_priority(P_4);
        printf("> [TASK] This task is to test priority P_4. (%lu)", i);
    }
}

void priority_task3(void)
{
    unsigned long i;
    int print_location = 10;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        sys_priority(P_9);
        printf("> [TASK] This task is to test priority P_9. (%lu)", i);
    }
}
