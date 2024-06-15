#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 4: Your code here.

// CODE_4
   // cprintf("entering sched_yield, I am CPU: %d\n", cpunum());
//
//
   // int i = 0;
   //// if (!curenv) {
   ////     curenv = &envs[0];
   ////     curenv->env_id = 0;
   //// } else {
   //// if curenv is not NULL, then init i at the env just after curenv
   // if (curenv != NULL) {
   //     i = ENVX(curenv->env_id) + 1;
   // }
   // struct Env* curr = &envs[i];
   // int start = i;
   // bool first_loop = true;
   // //struct Env curr = envs[ENVX(curenv->env_id)];
   // //cprintf("curr: %p\n", curr);
   // //cprintf("ENVX(curenv->env_id): %d\n", ENVX(curenv->env_id));
   // // cprintf("envs size: %d\n", sizeof(envs) / sizeof(envs[0]));
// //CODE_4
   // cprintf("in sched_yield 1, I AM ENV: %08x\n", curr->env_id);
// //END_CODE_4
   // for (;; i++) {
   //     // loop back to idx 0 if we've reached NENV
   //     if (i == NENV) {
   //         i = 0;
   //     }
   //     if (i == start && !first_loop) {
   //         break;
   //     }
   //     first_loop = false;

   //     //cprintf("in sched_yield, in for-loop, from start = %d, at i = %d, on CPU %d\n", start, i, cpunum());
   //     //cprintf("envs bytes size: %d\n", sizeof(envs));
   //     //cprintf("curr & curenv: %p, %p\n", curr, curenv);
   //     //iterate curr env pointer to the next env
   //     curr = &envs[i];
   //     //curr = &envs[ENVX(curenv->env_id) + i];
// //CODE_4
   // //cprintf("in sched_yield 2, I AM ENV: %08x\n", curr->env_id);
// //END_CODE_4
   //     // skip over any env that is NULL
   //     if (curr == NULL) {
   //         cprintf("skipping over a NULL env\n");
   //         continue;
   //     //} else if (curr == curenv) {
   //     //    // if we've looped back to curenv, then just continue running curenv
   //     //    //env_run(curenv);
   //     //    //cprintf("in sched_yield, curr has looped back to curenv, re-running curenv\n");
   //     //    break;
   //     }
   //     if (curr->env_status == ENV_RUNNING) {
   //         // this env is already running on another CPU, so do not try to run it from 2 different CPUs
   //         //cprintf("in sched_yield, this env is already ENV_RUNNING on another CPU\n");
   //     } else if (curr->env_status == ENV_RUNNABLE) {
   //         // we found a runnable env so run it
   //         cprintf("CPU %d is running an ENV_RUNNABLE env at: %p\n", cpunum(), curr);
   //         //curr->env_status = ENV_RUNNING;
   //         env_run(curr);
   //         break;
   //     }

   //     //if (i == start-1 && curenv != NULL) {
   //     //    // if we've looped back to curenv, then just continue running curenv
   //     //    // cprintf("in sched_yield, curr has looped back to curenv, re-running curenv\n");
   //     //    //env_run(curenv);
   //     //    break;
   //     //} else if (i == start-1 && curenv == NULL) {
   //     //    break;
   //     //}
   //     //if (i == ENVX(curenv->env_id) ) {
   //     //    break;
   //     //} else if (i == NENV) {
   //     //    i = 0;
   //     //}
   //     //cprintf("in for-loop, i: %d\n envs[i]: %p\n", i, envs[i]);
   //     //if (envs[i].env_status == ENV_RUNNABLE) {
   //     //    cprintf("running an ENV_RUNNABLE env");
   //     //    env_run(&envs[i]);
   //     //    return;
   //     //}
   // }
// CODE_4
    //cprintf("in sched_yield 3, I AM ENV: %08x\n", curr->env_id);
// END_CODE_4


// CODE_4

    int idx = 0;
    // first make sure that curenv is not NULL
    if (curenv != NULL) {
        // if curenv does already exist, then
        // start by checking the env that is right at curenv
        idx = ENVX(curenv->env_id) + 1;
    }
    struct Env* curr = &envs[idx];

   // loop thru all avail envs and check for one that is ready to run so that
   // the scheduler can rotate to it
    for (int i = 0; i < NENV; i++) {
        idx = (idx + i) % NENV;
        curr = &envs[idx];
        if (curr->env_status == ENV_RUNNABLE) {
            env_run(curr);
            break;
        }
    }
    // if curr has looped all the way around, and curenv is not running
    // on some other CPU, then just re-run curenv for this CPU
    if (curenv && curenv->env_status == ENV_RUNNING) {
        env_run(curenv);


    }

    // sched_halt()

// END_CODE_4


//
//
//
//
   // int i = 0;
   // cprintf("curenv: %p\n", curenv);
   // if (!curenv) {
   //     curenv = &envs[0];
   // }
   // else {
   //     i = ENVX(curenv->env_id)+1;
   // }
   // cprintf("curenv->env_id: %d\n", curenv->env_id);
   // cprintf("in sched_yield, before while-loop, on CPU #: %d\n", cpunum());
   // cprintf("NENV: %d\n, ENVX(curenv->env_id): %d\n", NENV, ENVX(curenv->env_id));
   // while (i != ENVX(curenv->env_id)) {
   //     if (i >= NENV) {
   //         if (NENV == 0) {
   //             break;
   //         }
   //         i = 0;
   //         //continue;
   //     }
   //     cprintf("in sched_yield, i: %d\n, envs[(ENVX(curenv->env_id) + i)]: %p\n, envs[(ENVX(curenv->env_id) + i)].env_status: %s\n", i, envs[(ENVX(curenv->env_id) + i)], envs[(ENVX(curenv->env_id) + i)].env_status);
// //       if (!envs[(ENVX(curenv->env_id) + i)]) {
// //           // if this env is NULL, then just go to next env
//
   //     if (envs[(ENVX(curenv->env_id) + i)].env_status == ENV_RUNNABLE) {
   //         // if this env is runnable then run it
   //         env_run(&envs[(ENVX(curenv->env_id) + i)]);
   //         return;
   //     }
   //     i++;
   // }

   // cprintf("in sched_yield, after while-loop\n");
   // if (envs[(ENVX(curenv->env_id) + i)].env_status == ENV_RUNNING) {
   //         // if we ended up right back at the env that is already running,
   //         // then just keep running this curenv
   //         env_run(curenv);
   //         return;
   // }


    //for (int i = ENVX(curenv->env_id)+1; i <= (ENVX(curenv->env_id) + i) % NENV; i++) {
    //    if (curenv == NULL) {
    //    }
    //    i = (i+1) % (sizeof(envs) / sizeof(envs[0]));
    //}



    // if there was no env ready, then sched_halt serves as an "idle func" for this CPU
    //cprintf("CPU %d is in sched_yield, going to sched halt\n", cpunum());



    //envs[0].env_status = ENV_NOT_RUNNABLE;
// END_CODE_4
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
             break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile (
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
        // LAB 4:
		// Uncomment the following line after completing exercise 13
		"sti\n"
		"1:\n"
		"hlt\n"
		"jmp 1b\n"
	: : "a" (thiscpu->cpu_ts.ts_esp0));
}

