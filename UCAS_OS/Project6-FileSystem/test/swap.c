#include <stdio.h>
#include <stdint.h>
#include <sys/syscall.h>

#define START 0x100000lu

int main(){
	uint64_t *array = (uint64_t *)START;//4k page, one page has 512 uint64
	int i, j;
	sys_move_cursor(1, 9);
	printf("writing...\n");
	for(i = 0; i < 4096; i++){//exceed 4K free pages, from 0x00100000 to 0x01100000... 0000 0001 0001 0000 0000 0000 0000 0000
		for(j = 0; j < 512; j++){//fill each page             vpn2 = 0, vpn1 = 0-8, vpn0 = 256-511(vpn1 = 0), vpn0 = 0-511(vpn1 = 1-7), vpn0 = 0-511(vpn1 = 1-7)
			array[i * 512 + j] = i;                          //                     vpn0 = 0-256(vpn1 = 8)
		}
		if((i >= 0 && i < 5) || (i >= 4080 && i < 4085))
			printf("%d:%lu ", i, array[i * 512]);
	}
	//first few pages should have been swapped
	printf("\nreading...\n");
	for(i = 0; i < 5; i++){
		printf("%d:%lu ", i, array[i * 512]);
	}
	for(i = 4080; i < 4085; i++){
		printf("%d:%lu ", i, array[i * 512]);
	}
	printf("\nDone\n");
	return 0;
}
