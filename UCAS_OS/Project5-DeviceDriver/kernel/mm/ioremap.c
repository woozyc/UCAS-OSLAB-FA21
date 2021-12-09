#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

void *ioremap(unsigned long phys_addr, unsigned long size)
{
    // map phys_addr to a virtual address
    // then return the virtual address
    int i = 0;
    uint64_t va_start;
    while(size > 0){
	    uint64_t va = io_base;
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
    	
    	uint64_t pa = phys_addr;
    	set_pfn(&ptes[vpn0], pa >> NORMAL_PAGE_SHIFT);
    	set_attribute(
    	    &ptes[vpn0], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
    	                    _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
    	io_base += PAGE_SIZE;
    	phys_addr += PAGE_SIZE;
    	size -= PAGE_SIZE;
    	
    	if(!i){
    		va_start = va;
    	}
    	i++;
    }
    local_flush_tlb_all();
    return va_start;
}

void iounmap(void *io_addr)
{
    // TODO: a very naive iounmap() is OK
    // maybe no one would call this function?
    // *pte = 0;
}
