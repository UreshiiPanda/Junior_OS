// User-level page fault handler support.
// Rather than register the C page fault handler directly with the
// kernel as the page fault handler, we register the assembly language
// wrapper in pfentry.S, which in turns calls the registered C
// function.

#include <inc/lib.h>


// Assembly language pgfault entrypoint defined in lib/pfentry.S.
extern void _pgfault_upcall(void);

// Pointer to currently installed C-language pgfault handler.
void (*_pgfault_handler)(struct UTrapframe *utf);

//
// Set the page fault handler function.
// If there isn't one yet, _pgfault_handler will be 0.
// The first time we register a handler, we need to
// allocate an exception stack (one page of memory with its top
// at UXSTACKTOP), and tell the kernel to call the assembly-language
// _pgfault_upcall routine when a page fault occurs.
//
void
set_pgfault_handler(void (*handler)(struct UTrapframe *utf))
{
	int r;

	if (_pgfault_handler == 0) {
		// First time through!
		// LAB 4: Your code here.
		//panic("set_pgfault_handler not implemented");

// CODE_4
        // we are running in ring 3, so we will use our syscall wrappers now

        // note that a page fault can cause another page fault, so we can have
        // a recursive stream of page faults that we need to handle. So while
        // handling the curr fault the handler is gonna generate another fault
        // so for this, we need to check if the TF is coming from the User
        // Exception Stack (UXSTACK) or not, cuz the first fault is going to
        // be coming from the User space, but the nested fault is then going
        // to be coming from the Kernel Exception Stack (

        // note: in the syscalls, JOS supports the convention that 0 reps the
        // curenv->env_id, so we can use 0 for our envid

        // first we need to alloc a page to store the incoming User TF, this
        // page should also have user permissions
        if (sys_page_alloc(0, (void *) (UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W) != 0) {
            panic("in sys_pgfault_handler: page alloc for the user TF failed\n");
        }
        // then we need to set the env_pgfault_upcall
        if (sys_env_set_pgfault_upcall(0, _pgfault_upcall) != 0) {
            panic("in sys_pgfault_handler: set upcall failed due to this env not existing or the caller does not have permissions to change this env\n");
        }
        //cprintf("in pgfault.c: the checks have passed\n");
// END_CODE_4

	}

	// Save handler pointer for assembly to call.
	_pgfault_handler = handler;
}
