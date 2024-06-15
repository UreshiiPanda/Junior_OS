// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

// LAB 1: add your command to here...
static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Show the backtrace of the current kernel stack", mon_backtrace },
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// LAB 1: Your code here.
    // HINT 1: use read_ebp().
    // HINT 2: print the current ebp on the first line (not current_ebp[0])

   // extern uint32_t bootstacktop;
   // extern uint32_t bootstack;
   // cprintf("bootstack: %08x", bootstack);
   // cprintf("bootstackTOP: %08x", bootstacktop);

   // uint32_t ebp;
   // asm volatile("movl %%ebp,%0" : "=r" (ebp));
   // cprintf("  ebp %08x\n", ebp);
       // uint32_t ebp = read_ebp();
       // uint32_t eip = ((uint32_t *)ebp)[1]; // Address of the return address (EIP)
       // cprintf("  ebp %08x  eip %08x  args", ebp, eip);
       //         // Print the first 5 arguments
       //         int i;
       //         for (i = 2; i < 7; i++) {
       //             cprintf(" %08x", ((uint32_t *)ebp)[i]);
       //         }
       //         cprintf("\n");

       //     if (ebp < 0xf0110000) {
       //         mon_backtrace(argc, argv, tf);
       //     }


    // get the initial address of the current ebp
    uint32_t ebp = read_ebp();
    // ebp is set to 0 in entry.S as starting point of stack
    while (ebp != 0) {
        // use pointer+offset to get eip and start of args
        uint32_t eip = *((uint32_t *) (ebp + 4));
        uint32_t *args = (uint32_t *) (ebp + 8);
        // get debug info from debuginfo_eip()
        struct Eipdebuginfo info;
        debuginfo_eip(eip, &info);
        // calc the offset of the curr eip from the address where this curr func in
        // info struct started
        uint32_t offset = eip - info.eip_fn_addr;
        cprintf(
            "  ebp %08x   eip %08x   args %08x %08x %08x %08x\n%08x\n         %s:%d: %.*s+%d\n",
            ebp, eip, args[0], args[1], args[2], args[3], args[4],
            info.eip_file, info.eip_line, info.eip_fn_namelen,
            info.eip_fn_name, offset
        );
        // recast ebp to be a pointer to the current address that ebp is at
        // then dereference that pointer so that we move ebp to the next
        // ebp address in the stack trace history
        ebp = *((uint32_t *) ebp);
    }


	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
