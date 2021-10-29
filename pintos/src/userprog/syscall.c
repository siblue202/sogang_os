#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>   // syscall-number 
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h" // for checking invalid address
#include "devices/shutdown.h" // for SYS_HALT
#include "userprog/process.h" // for SYS_EXEC

static void syscall_handler (struct intr_frame *);
static void check_addr(void *addr);
// static void get_argument(void *esp, uint32_t *arg, int count);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void 
check_addr(void *addr){
  if (!is_user_vaddr(addr)){
    // printf("invalid addr. \n");
    exit(-1);
  }
}

/*
static void
get_argument(void *esp, uint32_t *arg, int count){
  // printf("start get_argument\n"); // debud
  int i;
  esp += 4;
  check_addr(esp);
  // printf("check_addr is done\n"); // debug
  for(i = 0; i<count; i++){
    // printf("[%d] get \n", i); // debug
    esp += 4;
    check_addr(esp);
    arg[i] = *(uint32_t *)esp;
  }
  // printf("arg get !\n"); // debug
}
*/

void halt(void){
  shutdown_power_off();
}


void exit(int status){
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_current()->exit_status = status;

  /*
  if(status == 0){
    thread_current()->is_run = false;
  } 

  thread_current()->exit_status = status;
  sema_up(thread_current()->p_sema);
  */

  thread_exit(); // thread->state = thread_dying. 
}

tid_t exec(const char* cmd_line){
  // sema_down(thread_current()->c_sema);
  return process_execute(cmd_line);
}

int wait(tid_t pid){
  return process_wait(pid);
}

int read(int fd, void* buffer, unsigned size){
  int i;
  if(fd ==0){
    for(i=0; i<size; i++){
      if(((char *)buffer)[i] == '\0'){
        break;
      }
      return i;
    }
  }
  return -1;
}

int write(int fd, const void* buffer , unsigned size){
  if(fd == 1){
    putbuf(buffer, size);
    return size; 
  }
  return 0;
}

int fibonacci(int n){
  int t1 = 0;
  int t2 = 1;
  int nextTerm, i;

  for(i = 1; i<= n; ++i){
    nextTerm = t1 + t2;
    t1 = t2;
    t2 = nextTerm;
  }

  return t1;
}

int max_of_four_int(int a, int b, int c, int d){
  int i;
  int max = a;
  int arr[3] = {b, c, d}; 

  for (i = 0; i<3; i++){
    if(max < arr[i]){
      max = arr[i];
    }
  }
  return max;
}

static void
syscall_handler (struct intr_frame * f) 
{
  // uint32_t arg[3]; 

  // //getting stack_pointer
  void *esp = f->esp;
  check_addr(esp);
  
  //getting syscall number 
  int number = (int)*(uint32_t *)esp;
  
  // printf("syscall number : %d\n", number); // echo -> syscall : 9 debug

  // // hex_dump(esp, esp, 100, 1); // debug
  // printf("esp : %p\n", esp);

  /* 
  we need to implement function (halt, exit, exec, wait, read(stdin), write(stdout))
  + int fibonacci(int n)
  + int max_of_four_int(int a, int b, int c, int d)
  */
  // hex_dump(f->esp, f->esp, 100, 1); // debug


  switch (number) // (*(uint32_t *)(f->esp))
  {
    case SYS_HALT:
      halt();
      break;
    
    case SYS_EXIT:
      check_addr(esp+4);
      // get_argument(esp, arg, 1);
      exit((int)*(uint32_t *)(esp+4));
      //process_exit((int)arg[0]);

      break;
    
    case SYS_EXEC:
      check_addr(esp+4);
      // get_argument(esp, arg, 1);
      f->eax = exec((char *)*(uint32_t *)(esp+4));
      //process_execute((char *)arg[1]);

      break;

    case SYS_WAIT:
      check_addr(esp+4);
      // get_argument(esp, arg, 1);
      f->eax = wait((tid_t)*(uint32_t *)(esp+4));
      //process_wait((tid_t)arg[1]);

      break;

    case SYS_CREATE:
      break;
      
    case SYS_REMOVE:
      break;
      
    case SYS_OPEN:
      break;
      
    case SYS_FILESIZE:
      break;
      
    case SYS_READ:
      check_addr(esp+20);
      check_addr(esp+24);
      check_addr(esp+28);
      // get_argument(esp, arg, 3);
      f->eax = read((int)*(uint32_t *)(esp+20), (void *)*(uint32_t *)(esp+24), (unsigned)*(uint32_t *)(esp+28));

      break;
      
    case SYS_WRITE:
      check_addr(esp+20);
      check_addr(esp+24);
      check_addr(esp+28);
      // hex_dump(f->esp+20, f->esp+20, 100, 1); // debug
      // get_argument(esp, arg, 3); // int fd, const void* buffer, unsigned size
      f->eax = write((int)*(uint32_t *)(esp+20), (void *)*(uint32_t *)(esp+24), (unsigned)*(uint32_t *)(esp+28));

      break;
      
    case SYS_SEEK:
      break;
      
    case SYS_TELL:
      break;
    
    case SYS_CLOSE:
      break;

    case SYS_FIBONACCI:
      check_addr(esp+4);
      f->eax = fibonacci((int)*(uint32_t *)(esp+4));

      break;
    case SYS_MAXOFFOURINT:
      check_addr(esp+4);
      check_addr(esp+8);
      check_addr(esp+12);
      check_addr(esp+16);
      f->eax = max_of_four_int((int)*(uint32_t *)(esp+4), (int)*(uint32_t *)(esp+8), (int)*(uint32_t *)(esp+12), (int)*(uint32_t *)(esp+16));

      break;
      
    
    default:
      break;
  }

  // printf ("system call!\n");
  // thread_exit (0);
}




