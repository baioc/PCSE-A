#include "gdb_serial_support.h"
#include "string.h"

#define INTEL_OFFMASK	0xfff	/* offset within page */
#define PDESHIFT	22	/* page descriptor shift */
#define PDEMASK		0x3ff	/* mask for page descriptor index */
#define PTESHIFT	12	/* page table shift */
#define PTEMASK		0x3ff	/* mask for page table index */

/*
 *	Convert linear offset to page descriptor/page table index
 */
#define lin2pdenum(a)	(((a) >> PDESHIFT) & PDEMASK)
#define lin2ptenum(a)	(((a) >> PTESHIFT) & PTEMASK)

/*
 *	Convert page descriptor/page table index to linear address
 */
#define pdenum2lin(a)	((oskit_addr_t)(a) << PDESHIFT)
#define ptenum2lin(a)	((oskit_addr_t)(a) << PTESHIFT)

/*
 *	Number of ptes/pdes in a page table/directory.
 */
#define NPTES	(ptoa(1)/sizeof(pt_entry_t))
#define NPDES	(ptoa(1)/sizeof(pt_entry_t))

/*
 *	Hardware pte bit definitions (to be used directly on the ptes
 *	without using the bit fields).
 */
#define INTEL_PTE_VALID		0x00000001
#define INTEL_PTE_WRITE		0x00000002
#define INTEL_PTE_USER		0x00000004
#define INTEL_PTE_WTHRU		0x00000008
#define INTEL_PTE_NCACHE 	0x00000010
#define INTEL_PTE_REF		0x00000020
#define INTEL_PTE_MOD		0x00000040
#define INTEL_PTE_GLOBAL	0x00000100
#define INTEL_PTE_AVAIL		0x00000e00
#define INTEL_PTE_PFN		0xfffff000

#define INTEL_PDE_VALID		0x00000001
#define INTEL_PDE_WRITE		0x00000002
#define INTEL_PDE_USER		0x00000004
#define INTEL_PDE_WTHRU		0x00000008
#define INTEL_PDE_NCACHE 	0x00000010
#define INTEL_PDE_REF		0x00000020
#define INTEL_PDE_MOD		0x00000040	/* only for superpages */
#define INTEL_PDE_SUPERPAGE	0x00000080
#define INTEL_PDE_GLOBAL	0x00000100	/* only for superpages */
#define INTEL_PDE_AVAIL		0x00000e00
#define INTEL_PDE_PFN		0xfffff000

/*
 *	Macros to translate between page table entry values
 *	and physical addresses.
 */
#define	pa_to_pte(a)		((a) & INTEL_PTE_PFN)
#define	pte_to_pa(p)		((p) & INTEL_PTE_PFN)
#define	pte_increment_pa(p)	((p) += INTEL_OFFMASK+1)

#define	pa_to_pde(a)		((a) & INTEL_PDE_PFN)
#define	pde_to_pa(p)		((p) & INTEL_PDE_PFN)
#define	pde_increment_pa(p)	((p) += INTEL_OFFMASK+1)

/*
 *	Superpage-related macros.
 */
#define SUPERPAGE_SHIFT		PDESHIFT
#define SUPERPAGE_SIZE		(1 << SUPERPAGE_SHIFT)
#define SUPERPAGE_MASK		(SUPERPAGE_SIZE - 1)

#define round_superpage(x)	((oskit_addr_t)((((oskit_addr_t)(x))	\
				+ SUPERPAGE_MASK) & ~SUPERPAGE_MASK))
#define trunc_superpage(x)	((oskit_addr_t)(((oskit_addr_t)(x))	\
				& ~SUPERPAGE_MASK))

#define	superpage_aligned(x)	((((oskit_addr_t)(x)) & SUPERPAGE_MASK) == 0)


/* Must be initialized before the call to the gdb stub. */
static unsigned long client_cr0;
static unsigned long client_cr3;

/* Find the physical address of data available at VADDR in the client's virtual
space. The physical address is in *PADDR and the size of the rest of the page in
*LEN. If the virtual address has no physical mapping, *LEN is set to 0. */
static void
phys_addr_lookup(unsigned long vaddr, unsigned long *paddr, unsigned long *len)
{
	unsigned long *pd = (unsigned long *) pde_to_pa(client_cr3);
	unsigned long pde = pd[lin2pdenum(vaddr)];

	if ((client_cr0 & 0x80000000) == 0) {
		/* No virtual memory. */
		*paddr = vaddr;
		*len = 0xffffffff;
		return;
	}

	*len = 0;
	if ((pde & INTEL_PDE_VALID) == 0) return;
	if ((pde & INTEL_PDE_SUPERPAGE) == 0) {
		/* A 4K page table. */
		unsigned long *pt = (unsigned long *) pde_to_pa(pde);
		unsigned long pte = pt[lin2ptenum(vaddr)];
		unsigned long offs = vaddr & INTEL_OFFMASK;

		if ((pte & INTEL_PTE_VALID) == 0) return;
		*paddr = pte_to_pa(pte) + offs;
		*len = (INTEL_OFFMASK + 1) - offs;
	} else {
		/* A 4M page. */
		unsigned long offs = vaddr & SUPERPAGE_MASK;

		*paddr = trunc_superpage(pde) + offs;
		*len = SUPERPAGE_SIZE - offs;
	}
	return;
}

void copy_client_paging_registers(unsigned long cr0, unsigned long cr3, unsigned long cr4)
{
        (void)cr4;
	client_cr0 = cr0;
	client_cr3 = cr3;
}

static unsigned long disable_paging(void)
{
	int pcr0, ncr0;
	__asm__ __volatile__ ("movl %%cr0,%%eax; movl %%eax, %%ecx; andl $~0x80000000,%%eax; movl %%eax,%%cr0": "=c" (pcr0), "=a" (ncr0));
	return pcr0;
}

static void restore_paging(unsigned long cr0)
{
	__asm__ __volatile__ ("movl %%eax,%%cr0":: "a" (cr0));
}

int gdb_copyin(unsigned long src_va, void *dest_buf, unsigned long size)
{
	unsigned long cr0 = disable_paging();
	while (size) {
		unsigned long sz;
		unsigned long pa;

		phys_addr_lookup(src_va, &pa, &sz);
		if (sz == 0) {
			restore_paging(cr0);
			return -1;
		}
		if (sz > size) sz = size;
		memcpy(dest_buf, (void *)pa, sz);
		size -= sz;
		src_va += sz;
		dest_buf = ((char *)dest_buf) + sz;
	}
	restore_paging(cr0);
	return 0;
}

int gdb_copyout(const void *src_buf, unsigned long dest_va, unsigned long size)
{
	unsigned long cr0 = disable_paging();
	while (size) {
		unsigned long sz;
		unsigned long pa;

		phys_addr_lookup(dest_va, &pa, &sz);
		if (sz == 0) {
			restore_paging(cr0);
			return -1;
		}
		if (sz > size) sz = size;
		memcpy((void *)pa, src_buf, sz);
		size -= sz;
		dest_va += sz;
		src_buf = ((char *)src_buf) + sz;
	}
	restore_paging(cr0);
	return 0;
}


#define EFL_TF 0x100
/*
 * The GDB stub calls this architecture-specific function
 * to modify the trace flag in the processor state.
 */
void gdb_set_trace_flag(int trace_enable, struct gdb_state *state)
{
	if (trace_enable) {
		state->eflags |= EFL_TF;
	} else {
		state->eflags &= ~EFL_TF;
	}
}
