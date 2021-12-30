#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

#include <os.h>

static char buff[64];

int main(int argc, char **argv)
{
    int i, j;
    int mode = (argc > 1);
    sys_move_cursor(1,1);
    printf("starting test fs...mode: %s\n", mode ? "large" : "normal");
    int fd;
    if(!mode){
    	fd = sys_fopen("1.txt", O_RDWR);
    }else{
    	fd = sys_fopen("big.txt", O_RDWR);
    }
    if(mode){
    	printf("Start writing large file...");
    }

    // write 'hello world!' * 10
    for (i = 1; i < 11; i++)
    {
    	if(mode){
    		printf("%d/10 ", i);
	    	sys_lseek(fd, i * 839000, SEEK_SET);//over 8MB
	    }
        sys_fwrite(fd, "hello world!\n", 13);
    }
    if(mode)
    	printf("\n");
    
    // read
    for (i = 1; i < 11; i++)
    {
    	if(mode){
    		printf(" %d: ", i);
	    	sys_lseek(fd, i * 839000, SEEK_SET);
	    }
        sys_fread(fd, buff, 13);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);
}
