#include <os/mm.h>
#include <os/string.h>
#include <os/sched.h>
#include <os/stdio.h>
#include <pgtable.h>

ptr_t memCurr = FREEMEM;

//free physical page pool, array-based linked-list
freemem_node freemem_pool[(FREEMEM_END - FREEMEM) / PAGE_SIZE] = {{-1}};//index is linear mapped to pa
int freemem_head = -1;

//swap vpage pool
swapmem_node swapmem_pool[4096] = {{0, 0, 0, 0}};//index is NOT linear mapped to pa
int swapmem_head = -1;

//alloc fifo pool, only store swappable pgs
allocmem_node allocmem_pool[4096] = {{0, 0, 0}};//index is NOT linear mapped to pa
int allocmem_head = -1;

void swap_error(){
	prints("> [MEM] No swapable pages\n");
	do_exit();
}

ptr_t swap_page(){
	//TO DO:
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	//prints("\n> [MEM] physical mem full\n\r");
	int allocmem_node = allocmem_head - 1;
	for( ; allocmem_pool[(allocmem_node % 4096)].valid; allocmem_node--)
		if((allocmem_node % 4096) == allocmem_head){
			swap_error();
			return (ptr_t)NULL;
		}
	allocmem_node = (allocmem_node + 1) % 4096;
	if(allocmem_pool[allocmem_node].valid == 1){
		if(swapmem_pool[(swapmem_head + 1) % 4096].valid == 0){
			swapmem_head = (swapmem_head + 1) % 4096;
			swapmem_pool[swapmem_head].valid = 1;
			if((*current_running)->parent_id)
				swapmem_pool[swapmem_head].pid = (*current_running)->parent_id;
			else
				swapmem_pool[swapmem_head].pid = (*current_running)->pid;
			swapmem_pool[swapmem_head].vaddr = allocmem_pool[allocmem_node].vaddr;
			swapmem_pool[swapmem_head].ppte = allocmem_pool[allocmem_node].ppte;
			allocmem_pool[allocmem_node].valid = 0;
			//find pte and set invalid
			*(allocmem_pool[allocmem_node].ppte) = (PTE) 0;
			local_flush_tlb_all();
			//copy to disk
			sbi_sd_write(allocmem_pool[allocmem_node].pa, 8, swapmem_head * 8);
			//reuse swapped pa
			return pa2kva(allocmem_pool[allocmem_node].pa);
		}else{
			swap_error();
		}
	}else{
		swap_error();
	}
	return (ptr_t)NULL;
}
ptr_t allocPage(int numPage)
{
    ptr_t ret;
    int temp;
    if(freemem_head >= 0){
    	temp = freemem_head;
    	ret = freemem_head * PAGE_SIZE + FREEMEM;
    	freemem_head = freemem_pool[freemem_head].next;
    	freemem_pool[temp].next = -1; 
    	return ret;
    }
    // align PAGE_SIZE
    ret = ROUND(memCurr, PAGE_SIZE);
    if(ret < FREEMEM_END){//still have space for alloc
	    memCurr = ret + PAGE_SIZE;
    	return memCurr;
    }
    //physical mem full
    //prints("> [MEM] physical mem full\n");
    //while(1)
    	//;
    ret = swap_page();
    return ret;
}

void* kmalloc(size_t size)
{
    ptr_t ret = ROUND(memCurr, 4);
    memCurr = ret + size;
    return (void*)ret;
}
uintptr_t shm_page_get(int key)
{
    // TODO(c-core):
}

void shm_page_dt(uintptr_t addr)
{
    // TODO(c-core):
}

/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TO DO:
    kmemcpy((uint8_t *)dest_pgdir, (uint8_t *)src_pgdir, (uint32_t)PAGE_SIZE);
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page.
   */
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir_t, int swapable)
{
    // TO DO:
    va &= VA_MASK;
    PTE *pgdir = (PTE *)pgdir_t;
    uint64_t vpn2 = va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
    uint64_t vpn1 = (vpn2 << PPN_BITS) ^
                    (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
    uint64_t vpn0 = (vpn2 << (PPN_BITS + PPN_BITS)) ^
    				(vpn1 <<  PPN_BITS            ) ^
                    (va   >> (NORMAL_PAGE_SHIFT  ));
    if ((pgdir[vpn2] % 2) == 0) {
        // alloc a new second-level page directory
        set_pfn(&pgdir[vpn2], kva2pa(allocPage(1)) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pgdir[vpn2], _PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED | _PAGE_DIRTY);
        clear_pgdir(pa2kva(get_pa(pgdir[vpn2])));
    }
    PTE *pmd = (PTE *)pa2kva(get_pa(pgdir[vpn2]));
    
    if ((pmd[vpn1] % 2) == 0) {
        // alloc a new third-level page directory
        set_pfn(&pmd[vpn1], kva2pa(allocPage(1)) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pmd[vpn1], _PAGE_PRESENT | _PAGE_USER | _PAGE_ACCESSED | _PAGE_DIRTY);
        clear_pgdir(pa2kva(get_pa(pmd[vpn1])));
    }
    PTE *ptes = (PTE *)pa2kva(get_pa(pmd[vpn1]));
    
    uint64_t pa = kva2pa(allocPage(1));  
    set_pfn(&ptes[vpn0], pa >> NORMAL_PAGE_SHIFT);
    set_attribute(
        &ptes[vpn0], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
                        _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
    int allocmem_head_old = allocmem_head;
    if(swapable){
    	allocmem_head++;
    	if(allocmem_head == 4096){
    		allocmem_head = 0;
    	}
    	if(allocmem_pool[allocmem_head].valid == 0){
    		allocmem_pool[allocmem_head].pid = (*current_running)->parent_id ? (*current_running)->parent_id : (*current_running)->pid;
    		allocmem_pool[allocmem_head].valid = 1;
	    	allocmem_pool[allocmem_head].pa = pa;
    		allocmem_pool[allocmem_head].vaddr = (va >> 12) << 12;
    		allocmem_pool[allocmem_head].ppte = &ptes[vpn0];
    	}else{
    		allocmem_head = allocmem_head_old;
    	}
    }
    return pa2kva(pa);
}

void free_mem(uintptr_t pgdir_t){
	int i, j, m, k = freemem_head, node_index;
	PTE *pgdir = (PTE *)pgdir_t;
	PTE *pmd, *pmd2, *ppte;
	current_running = get_current_cpu_id() ? &current_running_1 : &current_running_0;
	for(i = 0; i < 512; i++){
		if(pgdir[i] % 2){//pgdir exist. i <=> vpn2
			pmd = (PTE *)pa2kva(get_pa(pgdir[i]));
			//pay attention! DO NOT free kernel pages!!!
			if(pmd > (PTE *)FREEMEM_END || pmd < FREEMEM)
				continue ;
			
			for(j = 0; j < 512; j++){//free. j <=> vpn1
				pmd2 = (PTE *)pa2kva(get_pa(pmd[j]));
				if(pmd[j] % 2){
					for(m = 0; m < 512; m++){//free. j <=> vpn1
						ppte = (PTE *)pa2kva(get_pa(pmd2[m]));
						if(pmd2[m] % 2){
							//free page frame
							node_index = ((unsigned long)ppte - FREEMEM) / PAGE_SIZE;
							freemem_pool[node_index].next = -1;//end of freemem list
							if(freemem_head < 0){//list empty
							freemem_head = node_index;
								k = freemem_head;
							}else{
								//k keeps finding the next, there is no need to turn back
								for(; freemem_pool[k].next >= 0; k = freemem_pool[k].next);
								//k is the last list node
								freemem_pool[k].next = node_index;
							}
						}
					}
					//free level-3 pgtable
					node_index = ((unsigned long)pmd2 - FREEMEM) / PAGE_SIZE;
					freemem_pool[node_index].next = -1;//end of freemem list
					if(freemem_head < 0){//list empty
						freemem_head = node_index;
						k = freemem_head;
					}else{
						//k keeps finding the next, there is no need to turn back
						for(; freemem_pool[k].next >= 0; k = freemem_pool[k].next);
						//k is the last list node
						freemem_pool[k].next = node_index;
					}
				}
			}
			//free level-2 pgtable
			node_index = ((unsigned long)pmd - FREEMEM) / PAGE_SIZE;
			freemem_pool[node_index].next = -1;//end of freemem list
			if(freemem_head < 0){//list empty
				freemem_head = node_index;
				k = freemem_head;
			}else{
				//k keeps finding the next, there is no need to turn back
				for(; freemem_pool[k].next >= 0; k = freemem_pool[k].next);
				//k is the last list node
				freemem_pool[k].next = node_index;
			}
		}
	}
	//free level-1 pgdir
	node_index = ((unsigned long)pgdir - FREEMEM) / PAGE_SIZE;
	freemem_pool[node_index].next = -1;//end of freemem list
	if(freemem_head < 0){//list empty
		freemem_head = node_index;
		k = freemem_head;
	}else{
		//k keeps finding the next, there is no need to turn back
		for(; freemem_pool[k].next >= 0; k = freemem_pool[k].next);
		//k is the last list node
		freemem_pool[k].next = node_index;
	}
	//free swapped pages
	for(i = 0; i < 4096; i++){
		if(swapmem_pool[i].pid == (*current_running)->pid){
			swapmem_pool[i].valid = 0;
		}
	}
	int swapmem_head_old = swapmem_head;
	for(; swapmem_head >= 0 && swapmem_pool[swapmem_head].valid == 0; swapmem_head = (swapmem_head+1) % 4096){
		if(swapmem_head == (swapmem_head_old - 1) % 4096){
			swapmem_head = -1;
			break;
		}
	}
	//free swapable pages
	for(i = 0; i < 4096; i++){
		if(allocmem_pool[i].pid == (*current_running)->pid){
			allocmem_pool[i].valid = 0;
		}
	}
	int allocmem_head_old = allocmem_head;
	for(; allocmem_head >= 0 && allocmem_pool[allocmem_head].valid == 0; allocmem_head = (allocmem_head+1) % 4096){
		if(allocmem_head == (allocmem_head_old - 1) % 4096){
			allocmem_head = -1;
			break;
		}
	}
}
