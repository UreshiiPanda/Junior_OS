// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.


// CODE_4
    // so the pages start out in a shared space, and after we handle a COW-based
    // page fault here, that page will then be marked W and moved to a private
    // space depending on who wrote to it. So the page that faulted will then
    // be backed up by a private, write-able copy


    // so first we check if the faulting page was a write to a COW page or not
    // REMEM: we need to check for the existence of the Pg DIR before we
    // check if the page itself exists
    if (~err & FEC_WR) {
        panic("in fork.c: the page fault was on a read");
    } else if(~uvpd[PDX(addr)] & PTE_P) {
        panic("in fork.c: the page dir does not exist");
    } else if (~uvpt[PGNUM(addr)] & PTE_COW) {
        panic("in fork.c: the page does not have COW permissions");
    } else if (~uvpt[PGNUM(addr)] & PTE_P) {
        panic("in fork.c: the page does not exist");
    }
    // CHECK
   // if (!((err & FEC_WR) && (uvpt[PGNUM(addr)] & PTE_COW) && (uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P))){
   //     panic("in fork.c: the pgfault was not a COW pgfault");
   // }

    // the page fault was a valid COW page fault
    // so now let's alloc a new page at PFTEMP and copy it over
    if (sys_page_alloc(0, (void *)PFTEMP, PTE_P | PTE_U | PTE_W) != 0) {
        panic("in fork.c: the sys_page_alloc failed");
    }
    memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
    // now let's map this new page for the new env
    if (sys_page_map(0, (void *)PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W) != 0) {
        panic("in fork.c: the sys_page_map failed");
    }
    // now we can unmap the old env's mapping
    if (sys_page_unmap(0, (void *)PFTEMP) != 0) {
        panic("in fork.c: the sys_page_unmap failed");
    }

    return;
// END_CODE_4
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");

// CODE_4
    // NOTE: we must map the Child as COW before the Parent, cuz otherwise this
    // would expose the Parent's data to the Child and this can lead to issues
    void * address = (void *)(pn * PGSIZE);
    // if the page is write-able or COW, the new mapping must be COW and marked
    // as such in both the parent and child envs
    if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)) {
        if (sys_page_map(0, address, envid, address, PTE_P | PTE_U | PTE_COW) != 0) {
            panic("in fork.c: the sys_page_map for a COW page to the child env failed");
        }
        if (sys_page_map(0, address, 0, address, PTE_P | PTE_U | PTE_COW) != 0) {
            panic("in fork.c: the sys_page_map for a COW page to the parent env failed");
        }
    } else {
        // the page is NOT write-able nor COW, so it is a Read-Only page and
        // should be marked as such in the child env. We don't need to re-mark
        // this page for the parent env cuz nothing is changing for the parent
        // in this case
        if (sys_page_map(0, address, envid, address, PTE_P | PTE_U) != 0) {
            panic("in fork.c: the sys_page_map for an RO page to the child env failed");
        }
    }
// END_CODE_4
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.

// CODE_4

    // so for fork() we will copy an env over, but for COW, we will not copy
    // pages but only page mappings, and then fork() copies the actual page
    // only when one of the envs tries to write to it

    // general steps:
    // 1. the Parent Env uses pgfault() as its pg fault handler
    // 2. the Parent Env calls sys_exofork() to create the Child Env
    // 3. the Parent will call duppage() to copy pages and mark them as COW.
    //    the exception to this is the page for the Exception Stack which
    //    always needs to be copied over and can't be marked COW
    // 4. the Parent copies its User PgFault entrypoint over to the Child
    // 5. the Child is now ready, the Parent marks it as ENV_RUNNABLE

    // so first we set the COW pgfault handler for the parent
    set_pgfault_handler(pgfault);
    // now we can create the new child env via our system fork
    envid_t envid = sys_exofork();
    if (envid < 0) {
        panic("in fork.c: the sys_exofork failed either due to no envs being available or due to mem exhaustion");
    }
    if (envid == 0) {
        // sys_exofork will return 0 for the child env
        // and now the child env needs to make sure that thisenv refers to itself
        // CHECK
        thisenv = &envs[ENVX(sys_getenvid())];
        // in the child, fork() returns 0
        return 0;
    }

    // the child env has already been created, it is still marked as ENV_NOT_RUNNABLE
    // the parent now needs to setup the child env, so it will copy COW pages over to
    // the child, so for each address below USTACKTOP, let's copy that page over
    for (uint32_t address = 0; address < USTACKTOP; address += PGSIZE) {
        // so if the pg DIR exists, if the page exists, and if the page is user-
        // write-able, then the child gets that page also
        // CHECK
        // cprintf("here is addrress-to-copy-into-child: %p\n", address);
        if ((uvpd[PDX(address)] & PTE_P) && (uvpt[PGNUM(address)] & PTE_P) && (uvpt[PGNUM(address)] & PTE_U)) {
            duppage(envid, PGNUM(address));
        }
    }
    // the UXSTACK is the only page that needs to be fully-mapped no matter what,
    // it can't be marked as COW cuz it will be the page that needs to be used
    // by the page fault handler to begin with
    // so let's copy over a UXSTACK page for the child
    if (sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W) != 0) {
        panic("in fork.c: failed to copy over the User Exception Stack into the child env via sys_page_alloc");
    }
    // now the parent needs to also copy over its user pgfault entrypoint over
    // to the child as well so that the child can have its own
    // void _pgfault_upcall();
    extern void _pgfault_upcall();
    // CHECK
    sys_env_set_pgfault_upcall(envid, _pgfault_upcall);

    // lastly, the parent marks the child as ENV_RUNNABLE
    if (sys_env_set_status(envid, ENV_RUNNABLE) != 0) {
        panic("in fork.c: failed to mark the child env as ENV_RUNNABLE");
    }

    // in the parent, fork() returns the pid of the child process/env that was created
    return envid;

// END_CODE_4
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
