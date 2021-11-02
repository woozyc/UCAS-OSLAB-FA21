/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                              A Mini PThread-like library
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#ifndef MTHREAD_H_
#define MTHREAD_H_

#include <stdint.h>
#include <stdatomic.h>
#include <os/list.h>
#include <os/lock.h>

#define MAX_MUTEX_LOCK 64

typedef struct{
	int key;
	mutex_lock_t lock_instance;
} mutex_array_cell;

typedef int mthread_mutex_t;

int mthread_mutex_init(mthread_mutex_t* handle);
int mthread_mutex_lock(mthread_mutex_t* handle);
int mthread_mutex_unlock(mthread_mutex_t* handle);

//int mthread_mutex_init(void *handle);
int mthread_mutex_destroy(void *handle);
int mthread_mutex_trylock(void *handle);
//int mthread_mutex_lock(void *handle);
//int mthread_mutex_unlock(void *handle);

typedef struct mthread_barrier
{
    // TODO:
} mthread_barrier_t;

int mthread_barrier_init(void *handle, unsigned count);
int mthread_barrier_wait(void *handle);
int mthread_barrier_destroy(void *handle);

typedef struct mthread_semaphore
{
    // TODO:
} mthread_semaphore_t;

int mthread_semaphore_init(void *handle, int val);
int mthread_semaphore_up(void *handle);
int mthread_semaphore_down(void *handle);
int mthread_semaphore_destroy(void *handle);

#endif
