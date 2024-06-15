# Junior OS

#### A simple x86 kernel written in C and Assembly

<a name="readme-top"></a>

<!-- smallsh gif -->
![small_shell](https://github.com/UreshiiPanda/SmallShell/assets/39992411/e5b00409-8922-4c6d-b11f-c6a555ac96da)


<!-- ABOUT THE PROJECT -->
## About The Project

This is a simple kernel program written in C and Assembly as part of an OS-2 course for students. Students are given 
a lot of bioler-plate code for this one, and my code has been left messy and unfinished (I made it through lab 4 out of 6) as this was merely for learning, 
but some of the topics covered and pieces of the kernel that I implemented include:
  - navigating x86 addressing hell: physical addresses, virtual linear addresses, virtual segmented addresses
  - setting up the kernel monitor's backtrace for debugging
  - Booting: the BIOS, bootloading, switching from Real-Mode to Protected-Mode, reading in the ELF kernel
  - Memory Management: setting up physical memory for peripheral devices, the kernel, booting, and free memory,
      - setting up memory mapping from physical to virtual, setting up a paging system with page directories, page
      - tables, page faults, page permissions, etc.
  - Setting user processes/environments: switching between user-mode and kernel-mode, setting up environment state, setting up trapframes with stored registers,
        - setting up parent/child processes, setting up hardware interrupts, process/environment exceptions, etc.
  - Multi-tasking: setting up system calls and syscall wrappers, setting up a simple round-robin scheduler,
        - setting up multi-processing from the boot-strap processor to the application processors, setting up per-CPU states/kernel-stacks/registers/etc,
        - setting up simple kernel locks/unlocks, setting up a copy-on-write system, setting up a simple fork() replica
        - setting up kernel preemption and clock interrupts, and implementing a simple IPC system via page-sharing

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## How to Run

While MIT's JOS link here [MIT JOS](https://pdos.csail.mit.edu/6.828/2018/overview.html) can explain this process better than I can, basically
you will need to install QEMU (a complete x86 computer emulator) and a compiler toolchain: including assembler, linker, C compiler, and debugger (GDB), 
for compiling and testing your kernel within QEMU. This repo can then be cloned and you can pick up where I left off within the JOS program.


### Installation / Execution Steps

1. Install QEMU and an appropriate compiler toolchain
2. Clone the repo
   ```sh
      git clone https://github.com/UreshiiPanda/Junior_OS.git
   ```



<p align="right">(<a href="#readme-top">back to top</a>)</p>
