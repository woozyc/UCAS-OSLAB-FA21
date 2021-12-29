#ifndef MM_H
#define MM_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Memory Management
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

#include <type.h>
#include <pgtable.h>

#define MEM_SIZE 32
#define PAGE_SIZE 4096 // 4K
#define INIT_KERNEL_STACK 0xffffffc051000000lu
#define FREEMEM (INIT_KERNEL_STACK+4*PAGE_SIZE)//0xffffffc051004000lu
#define FREEMEM_END 0xffffffc05e000000lu//0xffffffc052004000lu
//#define FREEMEM_END 0xffffffc052000000lu//4K free pages
#define USER_STACK_ADDR 0xf00010000lu

/* Rounding; only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))

typedef struct freemem_node{
	int next;
}freemem_node;
typedef struct swapmem_node{
	int pid;
	ptr_t vaddr;
	PTE * ppte;
	char valid;
}swapmem_node;
typedef struct allocmem_node{
	int pid;
	ptr_t pa;
	ptr_t vaddr;
	PTE * ppte;
	char valid;
}allocmem_node;

extern freemem_node freemem_pool[];
extern int freemem_head;
extern swapmem_node swapmem_pool[];
extern int swapmem_head;
extern allocmem_node allocmem_pool[];
extern int allocmem_head;

extern ptr_t memCurr;

extern ptr_t allocPage(int numPage);
extern void freePage(ptr_t baseAddr, int numPage);
extern void* kmalloc(size_t size);
extern void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir);
extern uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir, int swapable);
uintptr_t shm_page_get(int key);
void shm_page_dt(uintptr_t addr);
void free_mem(uintptr_t pgdir_t);

#endif /* MM_H */
