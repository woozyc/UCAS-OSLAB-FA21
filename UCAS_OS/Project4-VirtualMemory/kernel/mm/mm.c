#include <os/mm.h>
#include <pgtable.h>

ptr_t memCurr = FREEMEM;

ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(memCurr, PAGE_SIZE);
    memCurr = ret + numPage * PAGE_SIZE;
    return memCurr;
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
    // TODO:
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
