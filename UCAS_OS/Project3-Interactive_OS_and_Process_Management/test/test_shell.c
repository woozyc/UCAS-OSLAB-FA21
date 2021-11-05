/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <test.h>
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <screen.h>

struct task_info task_test_shell = {
    (uintptr_t)&test_shell, USER_PROCESS, P_4};


struct task_info task_test_waitpid = {(uintptr_t)&wait_exit_task, USER_PROCESS, P_4};
struct task_info task_test_semaphore = {(uintptr_t)&semaphore_add_task1, USER_PROCESS, P_4};
struct task_info task_test_barrier = {(uintptr_t)&test_barrier, USER_PROCESS, P_4};
    
struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS, P_4};
struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS, P_4};

struct task_info task_test_multicore = {(uintptr_t)&test_multicore, USER_PROCESS, P_4};
struct task_info task_test_affinity = {(uintptr_t)&test_affinity, USER_PROCESS, P_4};

static struct task_info *test_tasks[16] = {&task_test_waitpid,
                                           &task_test_semaphore,
                                           &task_test_barrier,
                                           &task_test_multicore,
                                           &strserver_task,
                                           &strgenerator_task,
                                           &task_test_affinity};
static int num_test_tasks = 7;


static char input_buffer[SCREEN_WIDTH];
static char commandhead[] = "> root@UCAS_OS: ";

static void shell_init_display(void){
    sys_move_cursor(1, SCREEN_HEIGHT - 1);
    printf("                                                                                \n");
    sys_move_cursor(1, SCREEN_HEIGHT);
    printf("--------------------------------------------------------------------------------\n");
    sys_move_cursor(1, SHELL_BEGIN);
    printf("----------------------------------- COMMAND ------------------------------------\n");
	sys_reflush();
}

static void shell_ps(){
	sys_process_show();
}

static void shell_clear(){
	sys_screen_clear();
	shell_init_display();
}

static void shell_help(){
	printf("  [MANUAL]");
	printf("  ps            : process show\n      display all the processes and their statuses\n");
	printf("  clear         : clear screen\n      clear screen and command shell\n");
	printf("  exec [task_id]: execute task\n      start running certain test task\n");
	printf("  kill [pid]    : kill process\n      kill a process\n");
	printf("  taskset    [mask] [task_id] : start a task with specified hart mask\n");
	printf("          -p [mask] [pid]     : set process hart mask\n");
}

static void shell_exec(int id){
	if(id < 0 || id >= num_test_tasks){
		printf("  No such task to execute\n");
		return ;
	}
	pid_t pid = sys_spawn(test_tasks[id], NULL, AUTO_CLEANUP_ON_EXIT, 3);
	if(pid > 0)
		printf("  Task executed, pid = %d\n", pid);
	else
		printf("  Task execution error\n");
}

static void shell_kill(int pid){
	int status = sys_kill(pid);
	if(status)
		printf("  Process %d killed\n", pid);
	else
		printf("  Process kill error\n");
}

static int resolve_command(int len){
	int j;
	char *cmd, *args;
	int set_mask, set_pid, set_taskid;
	for(j = 0; j < len - 1 && input_buffer[j] == ' '; j++);//find first none empty char
	cmd = input_buffer + j;
	if(strlen(cmd) == 0)
		return -1;
	for( ; j < len - 1; j++){
		if(input_buffer[j] == ' '){//split command
			input_buffer[j] = 0;
			j++;
			break;
		}
	}
	args = input_buffer + j++;
	for( ; j < len - 1; j++){
		if(input_buffer[j] == ' '){//split command
			input_buffer[j] = 0;
			j++;
			break;
		}
	}
	if(!strcmp(cmd, "ps")){
		shell_ps();
		return 1;
	}else if(!strcmp(cmd, "clear")){
		shell_clear();
		return 2;
	}else if(!strcmp(cmd, "help")){
		shell_help();
		return 3;
	}else if(!strcmp(cmd, "exec")){
		if(*args < '0' || *args > '9'){
			//printf("%c\n", *args);
			printf("  Usage: exec [task_id]\n");
			printf("    0: waitpid   1: semaphore    2: barrier 3: multicore\n");
			printf("    4: strserver 5: strgenerator 6: affinity\n");
			return -1;
		}
		while(*args >= '0' && *args <= '9' && j <= len){
			shell_exec(itoa(args));
			args = input_buffer + j++;
			for( ; j < len; j++){
				if(input_buffer[j] == ' '){//split command
					input_buffer[j] = 0;
					j++;
					break;
				}
			}
		}
		return 4;
	}else if(!strcmp(cmd, "kill")){
		if(*args < '0' || *args > '9'){
			printf("  Usage: kill [pid], try \"ps\" to get pid.\n");
			return -1;
		}
		while(*args >= '0' && *args <= '9' && j <= len){
			shell_kill(itoa(args));
			args = input_buffer + j++;
			for( ; j < len - 1; j++){
				if(input_buffer[j] == ' '){//split command
					input_buffer[j] = 0;
					j++;
					break;
				}
			}
		}
		return 5;
	}else if(!strcmp(cmd, "taskset")){
		if(!strcmp(args, "-p")){
			args = input_buffer + j++;
			for( ; j < len - 1; j++){
				if(input_buffer[j] == ' '){//split command
					input_buffer[j] = 0;
					j++;
					break;
				}
			}
			if(*args < '0' || *args > '9'){
				printf("  Usage: taskset   [mask] [pid]\n  or   : taskset -p [mask] [pid]\n");
				return -1;
			}
			set_mask = itoa(args);
			args = input_buffer + j++;
			for( ; j < len - 1; j++){
				if(input_buffer[j] == ' '){//split command
					input_buffer[j] = 0;
					j++;
					break;
				}
			}
			if(*args < '0' || *args > '9'){
				printf("  Usage: taskset [mask] [pid]\n  or   : taskset -p [mask] [pid]\n");
				return -1;
			}
			set_pid = itoa(args);
			sys_setmask(set_mask, set_pid);
			printf("  Process %d mask set, hart mask = %d\n", set_pid, set_mask);
		}else{
			if(*args < '0' || *args > '9'){
				printf("  Usage: taskset [mask] [pid]\n  or   : taskset -p [mask] [pid]\n");
				return -1;
			}
			set_mask = itoa(args);
			args = input_buffer + j++;
			for( ; j < len - 1; j++){
				if(input_buffer[j] == ' '){//split command
					input_buffer[j] = 0;
					j++;
					break;
				}
			}
			if(*args < '0' || *args > '9'){
				printf("  Usage: taskset   [mask] [pid]\n  or   : taskset -p [mask] [pid]\n");
				return -1;
			}
			set_taskid = itoa(args);
			sys_spawn(test_tasks[set_taskid], NULL, AUTO_CLEANUP_ON_EXIT, set_mask);
			printf("  Task %d executed, mask = %d\n", set_taskid, set_mask);
		}
		return 6;
	}else{
		printf("  Unknown command, try \"help\"\n");
		return 0;
	}
}

void test_shell()
{
    char c;
    int i;
    // TO DO:
    sys_screen_clear();
    shell_init_display();
    while (1)
    {
    	printf("%s", commandhead);
    	sys_reflush();
        // TO DO: call syscall to read UART port
        i = 0;
        while(i < SCREEN_WIDTH - 1){//getchar and show input interactively
        	c = getchar();
        	if(c == 8 || c == 127 || c == 27){//backspace 8('\b') or delete 127
        		if(i > 0){//input buffer not empty
        			i--;//backspace
        			putchar(c);//clear one char on screen
        			sys_reflush();
        			if(c == 27){//delete is caught as 'ESC'+'['+'3'+'~'
        				getchar();
        				getchar();
        				getchar();
        			}
        		}
        	}else{//normal input
        		if(c == '\r'){//enter
        			putchar('\n');
        			sys_reflush();
        			break;//got one piece of command
        		}
        		input_buffer[i++] = c;//store in buffer
        		putchar(c);//show on screen
        		sys_reflush();
        	}
        }
        input_buffer[i] = 0;
        // TO DO: parse input
        resolve_command(i);

        // TO DO: ps, exec, kill, clear
        
    }
}
