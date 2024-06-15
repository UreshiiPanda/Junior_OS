/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.

// CODE_3
    user_mem_assert(curenv, s, len, PTE_U | PTE_P);
// END_CODE_3

	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	if (e == curenv)
		cprintf("[%08x] exiting gracefully\n", curenv->env_id);
	else
		cprintf("[%08x] destroying %08x\n", curenv->env_id, e->env_id);
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.
// CODE_4
	//panic("sys_exofork not implemented");

    // this will be the new child env
    struct Env* e;
    // int env_alloc(env_store, envid_t parent id)
    // returns 0 on success, -E_NO_FREE_ENV if all envs busy, or
    // -E_NO_MEM if no mem left

    // if the curenv is NULL (there is no curenv) then we just set the parent
    // id to whatever the curenv's id is (so that the env that called this func
    // is the parent of the new child env)
    int res = env_alloc(&e, curenv ? curenv->env_id : 0);
    if (res == -E_NO_FREE_ENV) {
        return -E_NO_FREE_ENV;
    } else if (res == -E_NO_MEM) {
        return -E_NO_MEM;
    }
    //if (env_alloc(&e, curenv ? curenv->env_id:0) == -E_NO_FREE_ENV) {
    //    return -E_NO_FREE_ENV;
    //} else if (env_alloc(&e, curenv ? curenv->env_id:0) == -E_NO_FREE_ENV) {
    //    return -E_NO_MEM;
    //}
    // the user process is going to be running this child env, so we don't
    // allow the kernel to also run it
    e->env_status = ENV_NOT_RUNNABLE;
    // the TF holds the registers, and we copy the registers over to the child
    // so that it has the same registers as the parent
    // CHECK
    if (curenv) {
        e->env_tf = curenv->env_tf;
    }
    e->env_pgfault_upcall = curenv->env_pgfault_upcall;
    // the parent return val is already set in its TFs, but we need to set
    // the return val of the child as 0 (cuz fork() does this)
    e->env_tf.tf_regs.reg_eax = 0;
    //cprintf("envid in exofork: %d\n", e->env_id);
    return e->env_id;
// END_CODE_4
}

// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
// CODE_4
	//panic("sys_env_set_status not implemented");
    // here we are letting a user process set its own status or the status of
    // its child. This func is usually used to mark a new env as ready-to-run
    // after its address space and register state has been setup
    struct Env* e = NULL;
    // CHECK
    e = &envs[ENVX(envid)];
    //cprintf("envid in sys_env_set_status: %d\n", e->env_id);
    if (envid2env(envid, &e, 1) == -E_BAD_ENV) {
        // if this env is not valid or if this caller does not have permissions
        // to manip this env, then return -1
        return -E_BAD_ENV;
    }
    if (status != ENV_NOT_RUNNABLE && status != ENV_RUNNABLE) {
        return -E_INVAL;
    }
    e->env_status = status;
    return 0;
// END_CODE_4
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	//panic("sys_env_set_pgfault_upcall not implemented");

    // this is a kernel-level page-fault handler, but this syscall is a way for
    // a user-level program to handle its own page-faults. So this syscall tells
    // the kernel which func it needs to call for this user's/env's page-faults
    struct Env* e = NULL;
    if (envid2env(envid, &e, 1) != 0) {
            return -E_BAD_ENV;
    }
    e->env_pgfault_upcall = func;
    return 0;

}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you
	//   allocated!

	// LAB 4: Your code here.
	//panic("sys_page_alloc not implemented");
    struct PageInfo* pp;
    struct Env* e = NULL;
    e = &envs[ENVX(envid)];
    // check if the given perm is valid or not
    if ((perm & 0xfff) & (~PTE_SYSCALL)) {
        return -E_INVAL;
    }
    // check if the given va is page-aligned or not
    if ((uintptr_t) va & 0xfff) {
        return -E_INVAL;
    }
    //  check if the given va is >= UTOP
    if ((uintptr_t) va >= UTOP) {
        return -E_INVAL;
    }

    if (envid2env(envid, &e, 1) == -E_BAD_ENV) {
        // if this env is not valid or if this caller does not have permissions
        // to manip this env, then return -1
        return -E_BAD_ENV;
    }
    if ((perm & (PTE_U | PTE_P)) != (PTE_U | PTE_P)) {
        // if PTE_U and PTE_P are not set, or if any other bit IS set, then
        // this is an error
        return -E_INVAL;
    }

    pp = page_alloc(ALLOC_ZERO);
    if (pp == NULL) {
        return -E_NO_MEM;
    }
    if (page_insert(e->env_pgdir, pp, va, perm) != 0) {
        return -E_NO_MEM;
    }
    return 0;


}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	//panic("sys_page_map not implemented");
    // this is mapping a va from a source to a dest, so this func is basically
    // duplicating a pa-to-va mapping from one env to another, so we will use
    // this for our COW copying later on
    pte_t* pte_store;
    struct PageInfo* pp;
    struct Env* srcenv = NULL;
    srcenv = &envs[ENVX(srcenvid)];
    struct Env* dstenv = NULL;
    dstenv = &envs[ENVX(dstenvid)];
    // first we check if both the given srcenvid and the dstenvid are valid
    // and if the caller has permission to manip them
    if (envid2env(srcenvid, &srcenv, 1) != 0) {
        return -E_BAD_ENV;
    }
    if (envid2env(dstenvid, &dstenv, 1) != 0) {
        return -E_BAD_ENV;
    }
    // check again that the given va's are below UTOP
    if ((uintptr_t) srcva >= UTOP) {
        return -E_INVAL;
    }
    // check again that the given va's are below UTOP
    if ((uintptr_t) dstva >= UTOP) {
        return -E_INVAL;
    }
    // check if the given va's are page-aligned or not
    if ((uintptr_t) srcva & 0xfff) {
        return -E_INVAL;
    }
    // check if the given va's are page-aligned or not
    if ((uintptr_t) dstva & 0xfff) {
        return -E_INVAL;
    }
    //check if srcva is not mapped in srcenvid's address space
    if ((perm & ~PTE_SYSCALL) != 0) {
        return -E_INVAL;
    }
    if ((perm & (PTE_U | PTE_P)) != (PTE_U | PTE_P)) {
        return -E_INVAL;
    }
    pp = page_lookup(srcenv->env_pgdir, srcva, &pte_store);
    // if this page didn't exist for the src, then this is invalid
    if (!pp) {
        return -E_INVAL;
    }
    // check that the srcva isn't just read-only in srcenvid's address space
    if ((perm & PTE_W) && !(*pte_store & PTE_W)) {
        return -E_INVAL;
    }
    // if the page insert fails due to lack of mem then we return failure
    if (page_insert(dstenv->env_pgdir, pp, dstva, perm) != 0) {
        return -E_NO_MEM;
    }
    return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().

	// LAB 4: Your code here.
	//panic("sys_page_unmap not implemented");
    struct Env* e = NULL;
    e = &envs[ENVX(envid)];
    pte_t* pte_store;
    // check if the given va is page-aligned or not
    if ((uintptr_t) va & 0xfff) {
        return -E_INVAL;
    }
    //  check if the given va is >= UTOP
    if ((uintptr_t) va >= UTOP) {
        return -E_INVAL;
    }
    // return an error if the envid deosn't exist or if the caller doesn't have
    // permission to change that envid
    if (envid2env(envid, &e, 1) != 0) {
        return -E_BAD_ENV;
    }
    // if all of the checks passed, then we can safely remove the mapping via
    // page_remove()
    page_remove(e->env_pgdir, va);
    return 0;

}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	// LAB 4: Your code here.
	//panic("sys_ipc_try_send not implemented");
    struct Env* e;
    struct PageInfo* pp;
    pte_t* pte;

    if ((envid2env(envid, &e, 0)) != 0) {
        return -E_BAD_ENV;
    }
    if (e->env_ipc_recving == 0) {
        return -E_IPC_NOT_RECV;
    }
    // if the source va is valid, so under UTOP
    if ((uint32_t)srcva < UTOP) {
        if (PGOFF(srcva) != 0) {
            return -E_INVAL;
        }
        if ((perm & (PTE_P | PTE_U)) != (PTE_P | PTE_U)) {
            return -E_INVAL;
        }
        if ((perm & ~PTE_SYSCALL) != 0) {
            return -E_INVAL;
        }
        pp = page_lookup(curenv->env_pgdir, srcva, &pte);
        if (!pp) {
            return -E_INVAL;
        }
        if (page_insert(e->env_pgdir, pp, e->env_ipc_dstva, perm) != 0) {
            return -E_NO_MEM;
        }
        e->env_ipc_perm = perm;
    } else {
        e->env_ipc_perm = 0;
    }
    // our given send is valid so we set the ipc details in the env and we
    // mark the env as RUNNABLE again
    // now that env1 has set env2's status as RUNNABLE, then env2 can be
    // scheduled and run
    e->env_ipc_recving = 0;
    e->env_ipc_from = curenv->env_id;
    e->env_ipc_value = value;
    e->env_status = ENV_RUNNABLE;
    return 0;

}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	//panic("sys_ipc_recv not implemented");

    if ((uint32_t)dstva < UTOP && PGOFF(dstva) != 0) {
        return -E_INVAL;
    }
    // now we record that this env wants to receive a page, and then it marks
    // itself as NOT_RUNNABLE to give up the CPU to another env
    curenv->env_ipc_recving = 1;
    curenv->env_ipc_dstva = dstva;
    curenv->env_status = ENV_NOT_RUNNABLE;
	return 0;
}

// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.


	//panic("syscall not implemented");

	switch (syscallno) {

// CODE_3
    // we return 0 if this was a valid syscall number
    // and the cases can be found in lib/syscall.c and inc/syscall.h
    case SYS_cputs:
        {
            sys_cputs((const char *)a1, (size_t)a2);
            return 0;
        }
    case SYS_cgetc:
        {
            return sys_cgetc();
        }
    case SYS_getenvid:
        {
            envid_t env_id = sys_getenvid();
            return env_id;
        }
    case SYS_env_destroy:
        {
            // get the curr env's env_id, then feed it to the syscall func: sys_env_destroy()
            //envid_t env_id = sys_getenvid();
            return sys_env_destroy(a1);
        }

// CODE_4
    case SYS_yield:
        {
            // the scheduler has been called for a switch
            sys_yield();
            return 0;
        }
    case SYS_exofork:
        {
            return sys_exofork();
        }
    case SYS_env_set_status:
        {
            return sys_env_set_status((envid_t)a1, (int)a2);
        }
    case SYS_page_alloc:
        {
            return sys_page_alloc((envid_t)a1, (void*)a2, (int)a3);
        }
    case SYS_page_map:
        {
            return sys_page_map((envid_t)a1, (void*)a2, (envid_t)a3, (void*)a4, (int)a5);
        }
    case SYS_page_unmap:
        {
            return sys_page_unmap((envid_t)a1, (void*)a2);
        }
    case SYS_env_set_pgfault_upcall:
        {
            return sys_env_set_pgfault_upcall((envid_t)a1, (void *)a2);
        }
    case SYS_ipc_recv:
        {
            return sys_ipc_recv((void *)a1);
        }
    case SYS_ipc_try_send:
        {
            return sys_ipc_try_send(a1, a2, (void *)a3, a4);
        }
// END_CODE_4


    // this default case is for NSYSCALLS  (in inc/syscall.h)
    // if the syscall number is invlaid, we return -E_INVAL
// END_CODE_3

	default:
		return -E_INVAL;
	}
}

