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

//#include <test.h>
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/screen.h>

/*struct task_info task_test_shell = {
    (uintptr_t)&test_shell, USER_PROCESS, P_4};


struct task_info task_test_waitpid = {(uintptr_t)&wait_exit_task, USER_PROCESS, P_4};
struct task_info task_test_semaphore = {(uintptr_t)&semaphore_add_task1, USER_PROCESS, P_4};
struct task_info task_test_barrier = {(uintptr_t)&test_barrier, USER_PROCESS, P_4};
    
struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS, P_4};
struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS, P_4};

struct task_info task_test_multicore = {(uintptr_t)&test_multicore, USER_PROCESS, P_4};
struct task_info task_test_affinity = {(uintptr_t)&test_affinity, USER_PROCESS, P_4};

struct task_info task_mailbox_test = {(uintptr_t)&mailbox_test, USER_PROCESS, P_4};

static struct task_info *test_tasks[16] = {&task_test_waitpid,
                                           &task_test_semaphore,
                                           &task_test_barrier,
                                           &task_test_multicore,
                                           &strserver_task,
                                           &strgenerator_task,
                                           &task_test_affinity,
                                           &task_mailbox_test};
static int num_test_tasks = 8;*/

#define MAXARG 4

static char input_buffer[SCREEN_WIDTH];
static char *arg_buff[MAXARG];
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
	printf("  [MANUAL]\n");
	printf("  ps         : process show\n      display all the processes and their statuses\n");
	printf("  ls         : exec show\n      display all the files that can be executed\n");
	printf("  clear      : clear screen\n      clear screen and command shell\n");
	printf("  exec [task_id] [arg1]...:\n      execute task, start running certain test task\n");
	printf("  kill [pid] : kill process\n      kill a process\n");
	printf("  to be continued\n");
	//printf("  taskset    [mask] [task_id] : start a task with specified hart mask\n");
	//printf("          -p [mask] [pid]     : set process hart mask\n");
}

static void shell_exec(char *name, int argc, char **argv){
	pid_t pid = sys_exec(name, argc, argv, AUTO_CLEANUP_ON_EXIT);
	//pid_t pid = sys_spawn(test_tasks[id], NULL, AUTO_CLEANUP_ON_EXIT, 3);
	if(pid > 0)
		printf("  Task executed, pid = %d\n", pid);
	else
		printf("  Task execution error\n");
}

static void shell_ls(){
	sys_exec_show();
}

static void shell_kill(int pid){
	int status = sys_kill(pid);
	if(status)
		printf("  Process %d killed\n", pid);
	else
		printf("  Process kill error\n");
}

static int resolve_command(int len){
	int argc = 0, j;
	char *cmd;
	for(j = 0; j < MAXARG; j++){
		arg_buff[j] = NULL;
	}
	//int set_mask, set_pid, set_taskid;
	for(j = 0; j < len - 1 && input_buffer[j] == ' '; j++);//find first none empty char
	cmd = input_buffer + j;
	if(strlen(cmd) == 0)
		return -1;
	for( ; ; j++){
		if(input_buffer[j] == 0)
			break;
		if(input_buffer[j] == ' '){//split argvs
			input_buffer[j] = 0;
			j++;
			if(input_buffer[j] != ' ')
				arg_buff[argc++] = input_buffer + j;
		}
	}
	if(argc > MAXARG){
		argc = 4;
		printf("  Too many arguments, max: 4\n");
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
		if(arg_buff[0])
			shell_exec(arg_buff[0], argc, arg_buff);
		return 4;
	}else if(!strcmp(cmd, "kill")){
		for(int i = 0; i < argc; i++){
			int pid = itoa(arg_buff[i]);
			if(pid < 0){
				printf("  Usage: kill [pid], try \"ps\" to get pid.\n");
				return -1;
			}
			shell_kill(pid);
		}
		return 5;
	}/*else if(!strcmp(cmd, "ls")){
		shell_ls();
		return 6;
	}else if(!strcmp(cmd, "taskset")){
		if(!strcmp(args, "-p")){
			args = input_buffer + j++;
			for( ; ; j++){
				if(input_buffer[j] == ' ' || input_buffer[j] == 0){//split command
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
			for( ; ; j++){
				if(input_buffer[j] == ' ' || input_buffer[j] == 0){//split command
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
			for( ; ; j++){
				if(input_buffer[j] == ' ' || input_buffer[j] == 0){//split command
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
	}*/
	else if(!strcmp(cmd, "mkfs")){
		sys_mkfs();
		return 7;
	}else if(!strcmp(cmd, "statfs")){
		sys_statfs();
		return 8;
	}else if(!strcmp(cmd, "cd")){
		if(arg_buff[0])
			sys_cd(arg_buff[0]);
		return 9;
	}else if(!strcmp(cmd, "mkdir")){
		if(arg_buff[0])
			sys_mkdir(arg_buff[0]);
		return 10;
	}else if(!strcmp(cmd, "rmdir")){
		if(arg_buff[0])
			sys_rmdir(arg_buff[0]);
		return 11;
	}else if(!strcmp(cmd, "ls")){
		if(arg_buff[0])
			sys_ls(arg_buff[0], arg_buff[1]);
		else
			sys_ls("", arg_buff[1]);
		return 12;
	}else if(!strcmp(cmd, "touch")){
		if(arg_buff[0])
			sys_touch(arg_buff[0]);
		return 12;
	}else if(!strcmp(cmd, "cat")){
		if(arg_buff[0])
			sys_cat(arg_buff[0]);
		return 12;
	}else if(!strcmp(cmd, "ln")){
		if(arg_buff[0] && arg_buff[1])
			sys_ln(arg_buff[0], arg_buff[1]);
		return 12;
	}else if(!strcmp(cmd, "rm")){
		if(arg_buff[0])
			sys_rm(arg_buff[0]);
		return 12;
	}else{
		printf("  Unknown command, try \"help\"\n");
		return 0;
	}
}

int main()
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
        // TO DO: ps, exec, kill, clear
        resolve_command(i);
    }
    return 0;
}
