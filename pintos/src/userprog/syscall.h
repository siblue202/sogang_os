#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/thread.h" // for tid_t

// for syscall read & write

void syscall_init (void);

//JGH_sogang_proj1
void halt(void);
void exit(int status);
tid_t exec(const char* cmd_line);
int wait(tid_t pid);
int read(int fd, void* buffer, unsigned size);
int write(int fd, const void* buffer , unsigned size);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

//JGH_sogang_proj2
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

#endif /* userprog/syscall.h */
