#ifndef MAILBOX_HH
#define MAILBOX_HH

#define MAX_MAILBOX 16
#define MAX_NAME_LENGTH 32
#define MAX_MBOX_LENGTH 64

#include <os/smp.h>

typedef struct mailbox
{
    char message[MAX_MBOX_LENGTH];
    char name[MAX_NAME_LENGTH];
    smp_t full;
    smp_t empty;
    mutex_lock_t lock;
    int tail;
    int head;
    int user_num;
    list_head block_queue;
} kernel_mbox_t;

typedef struct{
	int key;
	kernel_mbox_t mbox_instance;
} mailbox_array_cell;

int kernel_mbox_open(char *name);
int kernel_mbox_close(int handle);
int kernel_mbox_send(int handle, void *msg, int msg_length);
int kernel_mbox_recv(int handle, void *msg, int msg_length);
int kernel_mbox_act(int handle, void *msg, int msg_length, int act_prefer);

void do_mbox_init(kernel_mbox_t *mbox, char *name);
int do_mbox_send(kernel_mbox_t *mbox, void *msg, int msg_length);
int do_mbox_recv(kernel_mbox_t *mbox, void *msg, int msg_length);

#endif
