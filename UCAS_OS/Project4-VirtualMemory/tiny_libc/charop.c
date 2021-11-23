#include <stdio.h>
#include <sys/syscall.h>

char getchar(void){
	//	return sys_getchar();
	char c;
	while((c = sys_serial_read()) == 255)
		;
	return c;
}
/*
void putchar(char c){
	char c_str[2] = {0};
	c_str[0] = c;
	sys_serial_write(c_str);
}
*/
