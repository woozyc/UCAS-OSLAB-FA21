#include <sbi.h>

#define VERSION_BUF 50

int version = 1; // version must between 0 and 9
char buf[VERSION_BUF];

int bss_check()
{
    for (int i = 0; i < VERSION_BUF; ++i) {
        if (buf[i] != 0) return 0;
    }
    return 1;
}

int getch()
{
	//Waits for a keyboard input and then returns.
	int c = sbi_console_getchar();
	while(c == -1)
		c = sbi_console_getchar();
    return c;
}

char bigdata[2048] = {'1'};//over a section size, 5 sections in total

int main(void)
{
    int check = bss_check();
    char output_str[] = "bss check: _ version: _\n\r";
    char output_val[2] = {0};
    int i, output_val_pos = 0;

    output_val[0] = check ? 't' : 'f';
    output_val[1] = version + '0';
    for (i = 0; i < sizeof(output_str); ++i) {
        buf[i] = output_str[i];
	if (buf[i] == '_') {
	    buf[i] = output_val[output_val_pos++];
	}
    }

	//print "Hello OS!"
	sbi_console_putstr("Hello OS!\n\r");
	sbi_console_putstr("Kernel No.1 is running well!\n\r");
	sbi_console_putstr("Size: 5 sections\n\r");
	//print array buf which is expected to be "Version: 1"
	sbi_console_putstr(buf);

    //fill function getch, and call getch here to receive keyboard input.
    int input;
    while(1){
    	input = getch();
		//print it out at the same time 
		sbi_console_putchar(input);
    }

    return 0;
}
