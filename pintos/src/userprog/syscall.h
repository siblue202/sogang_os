#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h" // for tid_t

void syscall_init (void);

//JGH
void halt(void);
void exit(int status);
tid_t exec(const char* cmd_line);
int wait(tid_t pid);
int read(int fd, void* buffer, unsigned size);
int write(int fd, const void* buffer , unsigned size);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

#endif /* userprog/syscall.h */
