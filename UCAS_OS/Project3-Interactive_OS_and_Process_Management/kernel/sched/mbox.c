#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/mbox.h>
#include <os/string.h>
#include <os/stdio.h>

//only visible to kernel
mailbox_array_cell mailbox_array[MAX_MAILBOX];

int mbox_try_down(smp_t *smp, int value)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    /* TO DO */
	if(smp->value < value){
		return 0;
    }else{
    	return 1;
    }
}

int mbox_smp_down(smp_t *smp, int value)
{
    current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
    /* TO DO */
    int i = 0;
	while(smp->value < value){
    	i++;
    	do_block(&((*current_running)->list), &smp->block_queue);
    }
    smp->value -= value;
    return i;
}

int mbox_smp_up(smp_t *smp, int value)
{
    /* TO DO */
    int i = 0;
    while(smp->block_queue.prev != &smp->block_queue){
    	i++;
    	//inverse order
        do_unblock(smp->block_queue.next);
    }
    smp->value += value;
    return i;
}

void do_mbox_init(kernel_mbox_t *mbox, char *name){
	do_smp_init(&mbox->full, 0);
	do_smp_init(&mbox->empty, MAX_MBOX_LENGTH);
	do_mutex_lock_init(&mbox->lock);
	mbox->head = 0;
	mbox->tail = 0;
	mbox->user_num = 1;
	kstrcpy((char *)&mbox->name, name);
}
int do_mbox_send(kernel_mbox_t *mbox, void *msg, int msg_length){
	int count = 0;
	count += mbox_smp_down(&mbox->empty, msg_length);
	do_mutex_lock_acquire(&mbox->lock);
	
	int len_1, len_2;
	len_1 = MAX_MBOX_LENGTH - mbox->tail;
	len_2 = msg_length - len_1;
    if(len_1 < msg_length){
        kmemcpy((unsigned char *)(mbox->message + mbox->tail), (unsigned char *)msg, len_1);
        kmemcpy((unsigned char *)mbox->message, (unsigned char *)(msg + len_1), len_2);
        mbox->tail = len_2;
    }else{
        kmemcpy((unsigned char *)(mbox->message + mbox->tail), (unsigned char *)msg, msg_length);
        mbox->tail += msg_length;
    }
	
	do_mutex_lock_release(&mbox->lock);
	mbox_smp_up(&mbox->full, msg_length);
	return count;
}
int do_mbox_recv(kernel_mbox_t *mbox, void *msg, int msg_length){
	int count = 0;
	count += mbox_smp_down(&mbox->full, msg_length);
	do_mutex_lock_acquire(&mbox->lock);
	
	int len_1, len_2;
	len_1 = MAX_MBOX_LENGTH - mbox->head;
	len_2 = msg_length - len_1;
    if(len_1 < msg_length){
        kmemcpy((unsigned char *)msg, (unsigned char *)(mbox->message + mbox->head), len_1);
        kmemcpy((unsigned char *)(msg + len_1), (unsigned char *)mbox->message, len_2);
        mbox->head = len_2;
    }else{
        kmemcpy((unsigned char *)msg, (unsigned char *)(mbox->message + mbox->head), msg_length);
        mbox->head += msg_length;
    }
    
	do_mutex_lock_release(&mbox->lock);
	mbox_smp_up(&mbox->empty, msg_length);
	
	return count;
}

//syscalls for user

int kernel_mbox_open(char *name){
	//return 0 when failed
	int i;
	if(kstrlen(name) > MAX_NAME_LENGTH){
		printk("> [MBOX] Name is too long (name length limit: %d)\n", MAX_NAME_LENGTH);
	}
	for(i = 1; i < MAX_MAILBOX; i++){
		if(mailbox_array[i].key && !kstrcmp(mailbox_array[i].mbox_instance.name, name)){
			mailbox_array[i].mbox_instance.user_num++;
			return i;
		}
	}
	for(i = 1; i < MAX_MAILBOX; i++){
		if(!mailbox_array[i].key){
			mailbox_array[i].key = (int)(long)name;
			do_mbox_init(&(mailbox_array[i].mbox_instance), name);
			return i;
		}
	}
	printk("> [MBOX] Mailbox open failed\n");
	return 0;
}
int kernel_mbox_send(int handle, void *msg, int msg_length){
	if(handle < 1 || handle >= MAX_MAILBOX || !mailbox_array[handle].key){
		printk("> [MBOX] Invalid mailbox, open one before use\n");
		return 0;
	}
	if(msg_length > MAX_MBOX_LENGTH){
		printk("> [MBOX] Send message is too long\n");
		return 0;
	}
	return do_mbox_send(&(mailbox_array[handle].mbox_instance), msg, msg_length);
}
int kernel_mbox_recv(int handle, void *msg, int msg_length){
	if(handle < 1 || handle >= MAX_MAILBOX || !mailbox_array[handle].key){
		printk("> [MBOX] Invalid mailbox, open one before use\n");
		return 0;
	}
	if(msg_length > MAX_MBOX_LENGTH){
		printk("> [MBOX] Receive message is too long\n");
		return 0;
	}
	return do_mbox_recv(&(mailbox_array[handle].mbox_instance), msg, msg_length);
}
int kernel_mbox_close(int handle){
	if(handle < 1 || handle >= MAX_MAILBOX || !mailbox_array[handle].key){
		printk("> [MBOX] Invalid mailbox, open one before use\n");
		return 0;
	}
	mailbox_array[handle].mbox_instance.user_num--;
	if(!mailbox_array[handle].mbox_instance.user_num)
		mailbox_array[handle].key = 0;//destory
	return 1;
}

//unified mailbox operation
int kernel_mbox_act(int handle, void *msg, int msg_length, int act_prefer){
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	if(handle < 1 || handle >= MAX_MAILBOX || !mailbox_array[handle].key){
		printk("> [MBOX] Invalid mailbox, open one before use\n");
		return 0;
	}
	if(msg_length > MAX_MBOX_LENGTH){
		printk("> [MBOX] Operating message is too long\n");
		return 0;
	}
	kernel_mbox_t *mbox = &(mailbox_array[handle].mbox_instance);
	//act_prefer = 0: try send first, 1: try read first
	int send_succeed = 0;
	int recv_succeed = 0;
	if(!act_prefer){
		while(!send_succeed && !recv_succeed){
			send_succeed = mbox_try_down(&mbox->empty, msg_length);
			if(!send_succeed){
				recv_succeed = mbox_try_down(&mbox->full, msg_length);
			}
			if(!send_succeed && !recv_succeed){
				do_block(&((*current_running)->list), &mbox->empty.block_queue);
			}
		}
		if(send_succeed){
			mbox->empty.value -= msg_length;
		}else{
			mbox->full.value -= msg_length;
		}
	}else{
		while(!send_succeed && !recv_succeed){
			recv_succeed = mbox_try_down(&mbox->full, msg_length);
			if(!send_succeed){
				send_succeed = mbox_try_down(&mbox->empty, msg_length);
			}
			if(!send_succeed && !recv_succeed){
				do_block(&((*current_running)->list), &mbox->full.block_queue);
			}
		}
		if(send_succeed){
			mbox->empty.value -= msg_length;
		}else{
			mbox->full.value -= msg_length;
		}
	}
	//entering the critical section
	do_mutex_lock_acquire(&mbox->lock);
	
	int len_1, len_2;
	if(send_succeed){
		len_1 = MAX_MBOX_LENGTH - mbox->tail;
		len_2 = msg_length - len_1;
	    if(len_1 < msg_length){
    	    kmemcpy((unsigned char *)(mbox->message + mbox->tail), (unsigned char *)msg, len_1);
    	    kmemcpy((unsigned char *)mbox->message, (unsigned char *)(msg + len_1), len_2);
    	    mbox->tail = len_2;
    	}else{
    	    kmemcpy((unsigned char *)(mbox->message + mbox->tail), (unsigned char *)msg, msg_length);
    	    mbox->tail += msg_length;
    	}
		do_mutex_lock_release(&mbox->lock);
		mbox_smp_up(&mbox->full, msg_length);
    	return 0;
    }else{//recv_succeed
		len_1 = MAX_MBOX_LENGTH - mbox->head;
		len_2 = msg_length - len_1;
    	if(len_1 < msg_length){
      	  kmemcpy((unsigned char *)msg, (unsigned char *)(mbox->message + mbox->head), len_1);
        	kmemcpy((unsigned char *)(msg + len_1), (unsigned char *)mbox->message, len_2);
        	mbox->head = len_2;
    	}else{
        	kmemcpy((unsigned char *)msg, (unsigned char *)(mbox->message + mbox->head), msg_length);
        	mbox->head += msg_length;
    	}
		do_mutex_lock_release(&mbox->lock);
		mbox_smp_up(&mbox->empty, msg_length);
    	return 1;
    }
}
