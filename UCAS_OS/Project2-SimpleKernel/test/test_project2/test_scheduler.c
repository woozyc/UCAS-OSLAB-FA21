#include <stdio.h>
#include <sys/syscall.h>

static char blank[] = {"                    "};
static char plane1[] = {"    ___         _   "};
static char plane2[] = {"| __\\_\\______/_|  "};
static char plane3[] = {"<[___\\_\\_______|  "};
static char plane4[] = {"|  o'o              "};

void print_task1(void)
{
    int i;
    int print_location = 5;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test scheduler. (%d)", i);
        //for part I print
        screen_reflush();
        sys_yield();
    }
}

void print_task2(void)
{
    int i;
    int print_location = 6;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test scheduler. (%d)", i);
        //for part I print
        screen_reflush();
        sys_yield();
    }
}

void drawing_task(void)
{
    int i, j = 22;

    while (1)
    {
        for (i = 55; i > 0; i--)
        {
            sys_move_cursor(i, j + 0);
            printf("%s", plane1);
        //for part I print
        screen_reflush();

            sys_move_cursor(i, j + 1);
            printf("%s", plane2);
        //for part I print
        screen_reflush();

            sys_move_cursor(i, j + 2);
            printf("%s", plane3);
        //for part I print
        screen_reflush();

            sys_move_cursor(i, j + 3);
            printf("%s", plane4);
        //for part I print
        screen_reflush();
        }
        sys_yield();

        sys_move_cursor(1, j + 0);
        printf("%s", blank);
        //for part I print
        screen_reflush();

        sys_move_cursor(1, j + 1);
        printf("%s", blank);
        //for part I print
        screen_reflush();

        sys_move_cursor(1, j + 2);
        printf("%s", blank);
        //for part I print
        screen_reflush();

        sys_move_cursor(1, j + 3);
        printf("%s", blank);
        //for print
        screen_reflush();
    }
}
