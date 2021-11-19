#include <stdio.h>
#include <sys/syscall.h>
#include <assert.h>

#define PARENT_PRI P_3

void fork_task(){
	int print_location = 11;
	unsigned long i = 0;
	int is_parent = 1;
	char c;
	int child_num = 0;
	//assert_supervisor_mode();
	task_priority_t priority = PARENT_PRI;
	sys_priority(priority);
	while(1){
		i++;
		if(is_parent){
			c = sys_read();
			if(c >= '0' && c <= '9'){
					child_num++;
					is_parent = sys_fork();
					if(!is_parent){
						priority = c - '0';
						sys_priority(priority);
					}
			}
			sys_move_cursor(1, print_location);
			printf("> [TASK] Testing fork. This is a parent. Priority P_%d. (%lu)", PARENT_PRI, i);
		}else{
			sys_move_cursor(1, print_location + child_num);
			printf("> [TASK] Testing fork. This is a child. Priority P_%d. (%lu)", priority, i);
		}
	}
}
