#include <screen.h>
#include <common.h>
#include <os/string.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/irq.h>
#include <os/stdio.h>
#include <sbi.h>
#include <os/smp.h>

//int screen_cursor_x;
//int screen_cursor_y;

/* screen buffer */
char new_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};
char old_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};

/* cursor position */
void vt100_move_cursor(int x, int y)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    // \033[y;xH
    disable_preempt();
    printk("%c[%d;%dH", 27, y, x);
    (*current_running)->cursor_x = x;
    (*current_running)->cursor_y = y;
    enable_preempt();
}

/* clear screen */
static void vt100_clear()
{
    // \033[2J
    printk("%c[2J", 27);
}

/* hidden cursor */
static void vt100_hidden_cursor()
{
    // \033[?25l
    printk("%c[?25l", 27);
}


void screen_scroll(){
	int i, j;
	for(i = SHELL_BEGIN; i < SCREEN_HEIGHT - 2; i++){
		for(j = 0; j < SCREEN_WIDTH; j++){
			new_screen[i * SCREEN_WIDTH + j] = new_screen[(i+1) * SCREEN_WIDTH + j];
		}
	}
	for(j = 0; j < SCREEN_WIDTH; j++){
		new_screen[i * SCREEN_WIDTH + j] = ' ';
	}
}

/* write a char */
static void screen_write_ch(char ch)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    if (ch == '\n')
    {
        (*current_running)->cursor_x = 1;
        (*current_running)->cursor_y++;
        if((*current_running)->cursor_y >= SCREEN_HEIGHT){
        	screen_scroll();
        	(*current_running)->cursor_y--;
        }
    }
    else if (ch == 8 || ch == 127 || ch == 27)
    {
        (*current_running)->cursor_x--;
    	new_screen[((*current_running)->cursor_y - 1) * SCREEN_WIDTH + ((*current_running)->cursor_x - 1)] = ' ';
    }
    else
    {
        new_screen[((*current_running)->cursor_y - 1) * SCREEN_WIDTH + ((*current_running)->cursor_x - 1)] = ch;
        (*current_running)->cursor_x++;
    }
}


void init_screen(void)
{
    vt100_hidden_cursor();
    vt100_clear();
    screen_write_ch('\n');
}

void screen_clear(void)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    int i, j;
    for (i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (j = 0; j < SCREEN_WIDTH; j++)
        {
            new_screen[i * SCREEN_WIDTH + j] = ' ';
        }
    }
    (*current_running)->cursor_x = 1;
    (*current_running)->cursor_y = 1;
    screen_reflush();
}

void screen_move_cursor(int x, int y)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    (*current_running)->cursor_x = x;
    (*current_running)->cursor_y = y;
}

int screen_get_cursor_x(void)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    return (*current_running)->cursor_x;
}

int screen_get_cursor_y(void)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    return (*current_running)->cursor_y;
}

void screen_write(char *buff)
{
    int i = 0;
    int l = kstrlen(buff);

    for (i = 0; i < l; i++)
    {
        screen_write_ch(buff[i]);
    }
}
//return any input, including empty(-1)
char screen_read(void){
	return sbi_console_getchar();
}
//return none empty input
char screen_getchar(void){
	char c;
	c = sbi_console_getchar();
	while(c == 255){
		c = sbi_console_getchar();
	}
	return c;
}

/*
 * This function is used to print the serial port when the clock
 * interrupt is triggered. However, we need to pay attention to
 * the fact that in order to speed up printing, we only refresh
 * the characters that have been modified since this time.
 */
void screen_reflush(void)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    int i, j;
    int temp_x, temp_y;
    temp_x = (*current_running)->cursor_x;
    temp_y = (*current_running)->cursor_y;

    /* here to reflush screen buffer to serial port */
    for (i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (j = 0; j < SCREEN_WIDTH; j++)
        {
            /* We only print the data of the modified location. */
            if (new_screen[i * SCREEN_WIDTH + j] != old_screen[i * SCREEN_WIDTH + j])
            {
                vt100_move_cursor(j + 1, i + 1);
                port_write_ch(new_screen[i * SCREEN_WIDTH + j]);
                old_screen[i * SCREEN_WIDTH + j] = new_screen[i * SCREEN_WIDTH + j];
            }
        }
    }

    /* recover cursor position */
    //vt100_move_cursor(screen_cursor_x, screen_cursor_y);
    (*current_running)->cursor_x = temp_x;
    (*current_running)->cursor_y = temp_y;
}

