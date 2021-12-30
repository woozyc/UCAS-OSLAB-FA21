#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

#include <os.h>

static char buff[64];

int main(void)
{
    int i, j;
    sys_move_cursor(1,1);
    printf("starting test fs...\n");
    int fd = sys_fopen("1.txt", O_RDWR);
    //printf("%d.\n", fd);

    // write 'hello world!' * 10
    for (i = 0; i < 10; i++)
    {
    	//printf("%d ", i);
        sys_fwrite(fd, "hello world!\n", 13);
    }
    //printf("\n");

    // read
    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, buff, 13);
    	//printf("%d ", i);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);
}
