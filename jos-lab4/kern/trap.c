#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};


static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < ARRAY_SIZE(excnames))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}


// XYZ: write a function declaration here...
// e.g., void t_divide();
//

// CODE_3

// no error codes
void t_divide();
void t_debug();
void t_nmi();
void t_brkpt();
void t_oflow();
void t_bound();
void t_illop();
void t_device();
void t_fperr();
void t_mchk();
void t_simderr();
void t_syscall();

// with error codes
void t_dblflt();
void t_tss();
void t_segnp();
void t_stack();
void t_gpflt();
void t_pgflt();
void t_align();


void t_irq_timer();
void t_irq_kbd();
void t_irq_2();
void t_irq_3();
void t_irq_serial();
void t_irq_5();
void t_irq_6();
void t_irq_spurious();
void t_irq_8();
void t_irq_9();
void t_irq_10();
void t_irq_11();
void t_irq_12();
void t_irq_13();
void t_irq_ide();
void t_irq_15();



// END_CODE_3

void
trap_init(void)
{
	extern struct Segdesc gdt[];

    /*
     *
     * HINT
     * Do something like this: SETGATE(idt[T_DIVIDE], 0, GD_KT, t_divide, 0);
     * if your trap handler's name for divide by zero is t_device.
     * Additionally, you should declare trap handler as a function
     * to refer that in C code... (see the comment XYZ above)
     *
     */
	// LAB 3: Your code here.


// CODE_3

    // the GD_KT is the "global descriptor number" for "kernel text"
    // the last arg is for the level at which the exception/interrupt will
    // be available (user, kernel, etc.)
         // brkpt and syscall need to be avial to user level 3

    // no error codes
    SETGATE(idt[T_DIVIDE], 0, GD_KT, t_divide, 0);
    SETGATE(idt[T_DEBUG], 0, GD_KT, t_debug, 0);
    SETGATE(idt[T_NMI], 0, GD_KT, t_nmi, 0);
    SETGATE(idt[T_BRKPT], 0, GD_KT, t_brkpt, 3);
    SETGATE(idt[T_OFLOW], 0, GD_KT, t_oflow, 0);
    SETGATE(idt[T_BOUND], 0, GD_KT, t_bound, 0);
    SETGATE(idt[T_ILLOP], 0, GD_KT, t_illop, 0);
    SETGATE(idt[T_DEVICE], 0, GD_KT, t_device, 0);
    SETGATE(idt[T_FPERR], 0, GD_KT, t_fperr, 0);
    SETGATE(idt[T_MCHK], 0, GD_KT, t_mchk, 0);
    SETGATE(idt[T_SIMDERR], 0, GD_KT, t_simderr, 0);
    SETGATE(idt[T_SYSCALL], 0, GD_KT, t_syscall, 3);


    // with error codes
    SETGATE(idt[T_DBLFLT], 0, GD_KT, t_dblflt, 0);
    SETGATE(idt[T_TSS], 0, GD_KT, t_tss, 0);
    SETGATE(idt[T_SEGNP], 0, GD_KT, t_segnp, 0);
    SETGATE(idt[T_STACK], 0, GD_KT, t_stack, 0);
    SETGATE(idt[T_GPFLT], 0, GD_KT, t_gpflt, 0);
    SETGATE(idt[T_PGFLT], 0, GD_KT, t_pgflt, 0);
    SETGATE(idt[T_ALIGN], 0, GD_KT, t_align, 0);

// END_CODE_3


    SETGATE(idt[IRQ_OFFSET + IRQ_TIMER], 0, GD_KT, t_irq_timer, 0);
    SETGATE(idt[IRQ_OFFSET + IRQ_KBD], 0, GD_KT, t_irq_kbd, 0);
    SETGATE(idt[IRQ_OFFSET + 2], 0, GD_KT, t_irq_2, 0);
    SETGATE(idt[IRQ_OFFSET + 3], 0, GD_KT, t_irq_3, 0);
    SETGATE(idt[IRQ_OFFSET + IRQ_SERIAL], 0, GD_KT, t_irq_serial, 0);
    SETGATE(idt[IRQ_OFFSET + 5], 0, GD_KT, t_irq_5, 0);
    SETGATE(idt[IRQ_OFFSET + 6], 0, GD_KT, t_irq_6, 0);
    SETGATE(idt[IRQ_OFFSET + IRQ_SPURIOUS], 0, GD_KT, t_irq_spurious, 0);
    SETGATE(idt[IRQ_OFFSET + 8], 0, GD_KT, t_irq_8, 0);
    SETGATE(idt[IRQ_OFFSET + 9], 0, GD_KT, t_irq_9, 0);
    SETGATE(idt[IRQ_OFFSET + 10], 0, GD_KT, t_irq_10, 0);
    SETGATE(idt[IRQ_OFFSET + 11], 0, GD_KT, t_irq_11, 0);
    SETGATE(idt[IRQ_OFFSET + 12], 0, GD_KT, t_irq_12, 0);
    SETGATE(idt[IRQ_OFFSET + 13], 0, GD_KT, t_irq_13, 0);
    SETGATE(idt[IRQ_OFFSET + IRQ_IDE], 0, GD_KT, t_irq_ide, 0);
    SETGATE(idt[IRQ_OFFSET + 15], 0, GD_KT, t_irq_15, 0);


	// Per-CPU setup
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// The example code here sets up the Task State Segment (TSS) and
	// the TSS descriptor for CPU 0. But it is incorrect if we are
	// running on other CPUs because each CPU has its own kernel stack.
	// Fix the code so that it works for all CPUs.
	//
	// Hints:
	//   - The macro "thiscpu" always refers to the current CPU's
	//     struct CpuInfo;
	//   - The ID of the current CPU is given by cpunum() or
	//     thiscpu->cpu_id;
	//   - Use "thiscpu->cpu_ts" as the TSS for the current CPU,
	//     rather than the global "ts" variable;
	//   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
	//   - You mapped the per-CPU kernel stacks in mem_init_mp()
	//   - Initialize cpu_ts.ts_iomb to prevent unauthorized environments
	//     from doing IO (0 is not the correct value!)
	//
	// ltr sets a 'busy' flag in the TSS selector, so if you
	// accidentally load the same TSS on more than one CPU, you'll
	// get a triple fault.  If you set up an individual CPU's TSS
	// wrong, you may not get a fault until you try to return from
	// user space on that CPU.
	//
	// LAB 4: Your code here:
// CODE_4
    // we need to initialize all CPU's instead of only the BSP CPU now
    //for (int i = 0; i < NCPU; i++) {
    //    thiscpu->cpu_ts
    //}

    // setup the TSS for thiscpu
    thiscpu->cpu_ts.ts_esp0 = KSTACKTOP - (thiscpu->cpu_id * (KSTKSIZE+KSTKGAP));
    thiscpu->cpu_ts.ts_ss0 = GD_KD;
    thiscpu->cpu_ts.ts_iomb = sizeof(struct Taskstate);

    // init the TSS into the GDT for thiscpu
    // CHECK
	gdt[(GD_TSS0 >> 3) + cpunum()] = SEG16(STS_T32A, (uint32_t) (&thiscpu->cpu_ts),
					sizeof(struct Taskstate) - 1, 0);
	gdt[(GD_TSS0 >> 3) + cpunum()].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
    // we can use a bitshift (for *8) to access the TSS of the correct CPU
	ltr(GD_TSS0 + (cpunum() << 3));

	// Load the IDT, the CPU's all use the same IDT, so we don't have to idx to
    // a specific CPU's IDT for this part
	lidt(&idt_pd);
// END_CODE_4
// BELOW IS THE OLD CODE FOR SINGLE CPU


	//// Setup a TSS so that we get the right stack
	//// when we trap to the kernel.
	//ts.ts_esp0 = KSTACKTOP;
	//ts.ts_ss0 = GD_KD;
	//ts.ts_iomb = sizeof(struct Taskstate);

	//// Initialize the TSS slot of the gdt.
	//gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),
	//				sizeof(struct Taskstate) - 1, 0);
	//gdt[GD_TSS0 >> 3].sd_s = 0;

	//// Load the TSS selector (like other segment selectors, the
	//// bottom three bits are special; we leave them 0)
	//ltr(GD_TSS0);

	//// Load the IDT
	//lidt(&idt_pd);
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p from CPU %d\n", tf, cpunum());
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

	// Handle clock interrupts. Don't forget to acknowledge the
	// interrupt using lapic_eoi() before calling the scheduler!
	// LAB 4: Your code here.
// CODE_3
   // here we dispatch the whatever the passed-in trap was
   // the tf structs have a prop called trapno which tells
   // what type of trap it is (by number)
   uint32_t envid;
   if (curenv == NULL) {
       envid = 0;
   } else {
       envid = curenv->env_id;
   }
    switch (tf->tf_trapno) {
        case T_PGFLT:
            {
                //cprintf("Page fault from %p at va %p eip %p\n", envid, rcr2(), tf->tf_eip);
                // see page_fault_handler() below
                return page_fault_handler(tf);
            }
        case T_BRKPT:
            {
                //cprintf("Trap from %p trap number %d from eip %p\n", envid, tf->tf_trapno, tf->tf_eip);
                // when we hit a breakpoint, we need to run the monitor func
                // we basically treat our kernel monitor as a primititve debugger
                return monitor(tf);
            }
        case T_SYSCALL:
            {
                // we need to send the curr register values and we also need
                // to get the return val from the syscall so that the user
                // program can get that value back after the syscall so that
                // it knows how the syscall went, and we send this back in the
                // EAX of the tf
               // cprintf("syscall from TF with %p, %p, %p, %p, %p, %p) from eip %p\n",
               //         envid,
               //         tf->tf_regs.reg_eax,
               //         tf->tf_regs.reg_edx,
               //         tf->tf_regs.reg_ecx,
               //         tf->tf_regs.reg_ebx,
               //         tf->tf_regs.reg_edi,
               //         tf->tf_regs.reg_esi,
               //         tf->tf_eip);

                int32_t ret = syscall(tf->tf_regs.reg_eax,
                        tf->tf_regs.reg_edx,
                        tf->tf_regs.reg_ecx,
                        tf->tf_regs.reg_ebx,
                        tf->tf_regs.reg_edi,
                        tf->tf_regs.reg_esi
                        );
                tf->tf_regs.reg_eax = ret;
                return;
            }
        case (IRQ_OFFSET + IRQ_TIMER):
            {
                 // handle timer interrupts (lapic_eoi)
                 lapic_eoi();
                 // then tell the CPU to search for a RUNNABLE env and schedule it,
                 // and this is how we enable preemptive multitasking
                 sched_yield();
                 return;
            }
    }

// END_CODE_3

	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	else {
		env_destroy(curenv);
		return;
	}
}

void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Halt the CPU if some other CPU has called panic()
	extern char *panicstr;
	if (panicstr)
		asm volatile("hlt");

	// Re-acqurie the big kernel lock if we were halted in
	// sched_yield()
	if (xchg(&thiscpu->cpu_status, CPU_STARTED) == CPU_HALTED)
		lock_kernel();
	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	// cprintf("Incoming TRAP frame at %p\n", tf);

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Acquire the big kernel lock before doing any
		// serious kernel work.
		// LAB 4: Your code here.
// CODE_4
        // this CPU will be switching from user to kernel mode for the trap,
        // so apply the lock before that
        lock_kernel();
// END_CODE_4
		assert(curenv);

		// Garbage collect if current enviroment is a zombie
		if (curenv->env_status == ENV_DYING) {
			env_free(curenv);
			curenv = NULL;
			sched_yield();
		}

		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNING)
		env_run(curenv);
	else
		sched_yield();
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 3: Your code here.
// CODE_3
    // the kernel takes in syscall's from user programs which allow user programs
    // to pass pointers to the kernel. These pointers point to user buffers
    // to be read/written to. But the kernel needs to make sure that any page
    // faults that occur when de-reffing these pointers were on behalf of the
    // user program and not the kernel itself. So the kernel needs to check that
    // the address of this pointer is in the user address space and that it is
    // valid in the page table. And if the kernel itself page-faults when it de-refs
    // the pointer, then it must itself panic and terminate itself

    // the cs prop in the tf contains a val which represents the privilege level
    // of the address via the last 2 bits of the CS register. This val is 0 when
    // it is a kernel-level fault
    if ((tf->tf_cs&0x3) == 0) {
        panic("in page_fault_handler, the fault was a kernel-level fault instead of a user-level fault");
    }

// END_CODE_3


	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// It is convenient for our code which returns from a page fault
	// (lib/pfentry.S) to have one word of scratch space at the top of the
	// trap-time stack; it allows us to more easily restore the eip/esp. In
	// the non-recursive case, we don't have to worry about this because
	// the top of the regular user stack is free.  In the recursive case,
	// this means we have to leave an extra word between the current top of
	// the exception stack and the new stack frame because the exception
	// stack _is_ the trap-time stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 4: Your code here.


	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

    // if this curenv can't alloc a page to the exception stack, or if it
    // can't write to it, then just destroy this env
	// Destroy the environment that caused the fault.

    if (!curenv->env_pgfault_upcall) {
	    cprintf("[%08x] user fault va %08x ip %08x\n",
		    curenv->env_id, fault_va, tf->tf_eip);
	    print_trapframe(tf);
	    env_destroy(curenv);
    } else {


// CODE_4

    // we are using this to dispatch page faults to the user-mode handler
    // and first we want to create a TrapFrame for the user program, so let's
    // copy the kernel TrapFrame (passed in as tf) into a replica that we'll
    // call a UserTrapFrame
    uintptr_t utf_address;

    // we store this User TF at UXSTACK (user exception stack) cuz we want
    // to separate this execution from where the user program itself is running
    // (at USTACK)
    // but if this is a nested exception, then, we can nest exceptions until
    // the UXSTACK space is full
    if (ROUNDUP(tf->tf_esp, PGSIZE) == UXSTACKTOP) {
        // this exception came from the user exception stack, if we have nested
        // page faults, then we need to place each next TF below the previous
        // ones at the current ESP
        utf_address = (tf->tf_esp - 4 - sizeof(struct UTrapframe));
    } else {
        // if this is a new exception (The UXSTACK is empty), then we can just
        // place this first user TF right under UXSTACKTOP to start off the
        // TF stack
        utf_address = (UXSTACKTOP - sizeof(struct UTrapframe));
    }

    // now that we've figured out where the User TF address should be, we can
    // set the UTF at that location, and copy over the original TF struct
    struct UTrapframe* utf = (struct UTrapframe *)utf_address;
    //cprintf("in trap.c: here is utf va: [%08x]\n", utf->utf_fault_va);
    //cprintf("in trap.c: here is OG va: [%08x]\n", fault_va);
    //cprintf("in trap.c: here is UTF address: [%08x]\n", utf);
    //cprintf("in trap.c: here is curenv address: [%08x]\n", &curenv);

    user_mem_assert(curenv, (void *)utf_address, sizeof(struct UTrapframe), PTE_W | PTE_U);

    utf->utf_fault_va = fault_va;
    utf->utf_err = tf->tf_err;
    utf->utf_regs = tf->tf_regs;
    utf->utf_eip = tf->tf_eip;
    utf->utf_eflags = tf->tf_eflags;
    utf->utf_esp = tf->tf_esp;

    // we also need to check if the user exception stack is full and if the
    // curenv has write persmissions
    //cprintf("in trap.c: here is utf va: [%08x]\n", utf->utf_fault_va);
    //cprintf("in trap.c: here is OG va: [%08x]\n", fault_va);
    //cprintf("in trap.c: here is UTF address: [%08x]\n", utf);
    //cprintf("in trap.c: here is curenv address: [%08x]\n", &curenv);

    // then we update the values of the kernel TF,
    // and we now branch to curenv->env_pgfault_upcall. We can return now to
    // whatever the user program does next, but we want to execute the page fault
    // handler first, so we need to tell the curenv about where this new TF ESP
    // is at (Which we just set) and we need to pass the handler we want to use
    // for the page fault to the EIP

    curenv->env_tf.tf_eip = (uintptr_t) curenv->env_pgfault_upcall;
    curenv->env_tf.tf_esp = utf_address;

    // lastly, we resume the user program after the trap, but with the page fault
    // handler setup to run this time
    env_run(curenv);

    }
// END_CODE_4




}

