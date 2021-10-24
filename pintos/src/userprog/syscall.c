#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>   // syscall-number 
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h" // for checking invalid address

static void syscall_handler (struct intr_frame *);
static void chech_addr(void *addr);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void 
check_addr(void *addr){
  if (!is_user_vaddr(addr)){
    exit(-1);
  }
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // using switch case 
  printf ("system call!\n");
  thread_exit ();
}


