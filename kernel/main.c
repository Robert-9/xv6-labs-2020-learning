#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
// void
// main()
// {
//   if(cpuid() == 0){
//     consoleinit();
//     printfinit();
//     printf("\n");
//     printf("xv6 kernel is booting\n");
//     printf("\n");
//     kinit();         // physical page allocator
//     kvminit();       // create kernel page table
//     kvminithart();   // turn on paging
//     procinit();      // process table
//     trapinit();      // trap vectors
//     trapinithart();  // install kernel trap vector
//     plicinit();      // set up interrupt controller
//     plicinithart();  // ask PLIC for device interrupts
//     binit();         // buffer cache
//     iinit();         // inode cache
//     fileinit();      // file table
//     virtio_disk_init(); // emulated hard disk
//     userinit();      // first user process
//     __sync_synchronize();
//     started = 1;
//   } else {
//     while(started == 0)
//       ;
//     __sync_synchronize();
//     printf("hart %d starting\n", cpuid());
//     kvminithart();    // turn on paging
//     trapinithart();   // install kernel trap vector
//     plicinithart();   // ask PLIC for device interrupts
//   }

//   scheduler();        
// }
void main()
{
  if(cpuid() == 0){
    uartinit();
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting on CPU %d\n", cpuid());
    printf("\n");

    printf("Initializing kernel subsystems...\n");
    kinit();         // physical page allocator
    printf("Physical page allocator initialized.\n");

    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    printf("Kernel page table initialized.\n");

    procinit();      // process table
    printf("Process table initialized.\n");

    trapinit();      // trap vectors
    trapinithart();  // install kernel trap vector
    printf("Trap vectors initialized.\n");

    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    printf("PLIC initialized.\n");

    binit();         // buffer cache
    iinit();         // inode cache
    fileinit();      // file table
    printf("File system initialized.\n");

    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    printf("First user process initialized.\n");

    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
    printf("hart %d initialized\n", cpuid());
  }

  printf("Entering scheduler on CPU %d\n", cpuid());
  scheduler();        
}
