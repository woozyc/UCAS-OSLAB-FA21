#ifndef SMP_H
#define SMP_H

// #define NR_CPUS 2
// extern void* cpu_stack_pointer[NR_CPUS];
// extern void* cpu_pcb_pointer[NR_CPUS];
extern uint64_t get_current_cpu_id();
extern void lock_kernel();
extern void unlock_kernel();

#include <os/list.h>

#define MAX_SMP 16

typedef struct semaphore
{
    int value;
    list_head block_queue;
} smp_t;

typedef struct{
	int key;
	smp_t smp_instance;
} smp_array_cell;

int smp_get(int key, int value);
int smp_down(int handle);
int smp_up(int handle);
int smp_destory(int handle);

void do_smp_init(smp_t *smp, int value);
int do_smp_down(smp_t *smp);
int do_smp_up(smp_t *smp);

#define MAX_BARRIER 16

typedef struct barrier
{
    int value;
    int waiting;
    list_head block_queue;
} barrier_t;

typedef struct{
	int key;
	barrier_t barrier_instance;
} barrier_array_cell;

int barrier_get(int key, int value);
int barrier_wait(int handle);
int barrier_destory(int handle);

void do_barrier_init(barrier_t *barrier, int value);
void do_barrier_wait(barrier_t *barrier);

#endif /* SMP_H */
