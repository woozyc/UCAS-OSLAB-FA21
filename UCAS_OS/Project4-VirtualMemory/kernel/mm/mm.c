#include <os/mm.h>
#include <os/string.h>
#include <os/stdio.h>
#include <pgtable.h>

ptr_t memCurr = FREEMEM;

//free physical page pool, array-based linked-list
typedef struct freemem_node{
	int swap_status;
	int next;
}freemem_node;
freemem_node freemem_pool[(FREEMEM_END - FREEMEM) / PAGE_SIZE] = {0, -1};
int freemem_head = -1;

ptr_t allocPage(int numPage)
{
    ptr_t ret;
    int temp;
    if(freemem_head >= 0){
    	temp = freemem_head;
    	ret = freemem_head * PAGE_SIZE + FREEMEM;
    	freemem_head = freemem_pool[freemem_head].next;
    	freemem_pool[temp].swap_status = 0; 
    	freemem_pool[temp].next = -1; 
    	return ret;
    }
    // align PAGE_SIZE
    ret = ROUND(memCurr, PAGE_SIZE);
    if(ret < FREEMEM_END){//still have space for alloc
	    memCurr = ret + numPage * PAGE_SIZE;
    	return memCurr;
    }
    //physical mem full
    //TODO:
    prints("> [MEM] physical mem full\n");
    while(1)
    	;
    return (ptr_t)NULL;
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
uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir_t)
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
    return pa2kva(pa);
}

void free_mem(uintptr_t pgdir_t){
	int i, k = freemem_head, node_index;
	PTE *pgdir = (PTE *)pgdir_t;
	PTE *pmd;
	for(i = 0; i < 512; i++){
		if(pgdir[i] % 2){//pgdir exist
			pmd = (PTE *)pa2kva(get_pa(pgdir[i]));
			//free level-2 pgtable
			//pay attention! DO NOT free kernel level-2 pages!!!
			if(pmd > FREEMEM_END)
				continue ;
			node_index = ((unsigned long)pmd - FREEMEM) / PAGE_SIZE;
			freemem_pool[node_index].next = -1;//end of freemem list
			freemem_pool[node_index].swap_status = 0;
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
	freemem_pool[node_index].swap_status = 0;
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
