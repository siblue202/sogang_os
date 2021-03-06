#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "filesys/file.h" 
// jgh 
#include "devices/timer.h"
#include "threads/fixed-point.h"    // jgh for BSD 

#ifdef USERPROG
#include "userprog/process.h"
#endif
//JGH 

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;

/* JGH 
list of sleeping process. 
checking every ticks to wake up (timer_interrupt() -> thread_tick() -> thread_wake_up()) */ 
struct list sleeping_list;

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame 
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

#ifndef USERPROG
/* Project #3. */
bool thread_prior_aging;
int load_avg;               // for BSD
#endif

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;
// jgh for BSD 
int load_avg; // system-wide value

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  list_init (&ready_list);
  list_init (&all_list);
  // jgh 
  list_init (&sleeping_list);
  // jgh_end

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);

  //JGH
  sema_init(&(initial_thread->c_sema), 0);
  sema_init(&(initial_thread->mem_sema), 0);
  sema_init(&(initial_thread->load_sema), 0);
  sema_init(&(initial_thread->sleeping_sema), 0);
  sema_init(&(initial_thread->wakeup_sema), 0);
  // sema_init(&(initial_thread->exec_sema), 0);
  list_init(&(initial_thread->child));
  list_init(&(initial_thread->lock_waiter));      // for proj3
  initial_thread->init_priority = initial_thread->priority; // for proj 3 ?????? priority ?????? 
  initial_thread->lock_wait = NULL;               // for proj3
  
  // for BSD
  if(thread_mlfqs == true){
    initial_thread->nice = 0;             // for BSD
    initial_thread->recent_cpu = 0;       // for BSD
    int left_priority = sub_x_y(int_to_fp(PRI_MAX), div_x_n(initial_thread->recent_cpu,4));  // for BSD
    int caled_priority = fp_to_int_zero(sub_x_n(left_priority, (initial_thread->nice *2)));// for BSD
    if(caled_priority > PRI_MAX){
      initial_thread->priority = PRI_MAX;
    } else if(caled_priority < PRI_MIN){
      initial_thread->priority = PRI_MIN;
    } else{
      initial_thread->priority = caled_priority;
    }
  }
  
  //JGH 
  for(int i =0; i<128; i++){
    initial_thread->fd[i] = NULL;
  }

  // printf("thread_prior_aging is %d\n", thread_prior_aging); // debug
  //JGH_END

  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}


/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void) 
{
  /* Create the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create ("idle", PRI_MIN, idle, &idle_started);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{
  struct thread *t = thread_current ();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();

  #ifndef USERPROG
  /* Project #3. */
  thread_wake_up ();
  // sema_down(&t->sleeping_sema);

  // /* Project #3. */
  if (thread_prior_aging == true){
    // printf("thread_aging start \n"); // debug
    thread_aging();
  }

  #endif
}

/* Prints thread statistics. */
void
thread_print_stats (void) 
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  struct thread *t; // child thread 
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;

  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */
  init_thread (t, name, priority);
  tid = t->tid = allocate_tid ();

  // JGH :: set child and parent  tid in struct thread 

  // printf("current tid : %d,   just created tid : %d \n", thread_current()->tid, tid); // debug
  // t->parant_tid = thread_current()-> tid;

  // struct semaphore sema;
  // sema_init(&sema, 0);

  // t->p_sema = thread_current()->c_sema = &sema;

  // printf("init start\n");
  sema_init(&(t->c_sema), 0);
  sema_init(&(t->mem_sema), 0);
  sema_init(&(t->load_sema), 0);
  sema_init(&(t->sleeping_sema), 0);
  sema_init(&(t->wakeup_sema), 0);
  // sema_init(&(t->exec_sema), 0);
  list_init(&(t->child));
  list_push_back(&(thread_current()->child), &(t->child_elem));
  // printf("init complete\n");
  list_init(&(t->lock_waiter));     // for proj3
  t->init_priority = priority;   // for proj3 init_priority ????????? 
  t->lock_wait = NULL;              // for proj3

  // for BSD
  if(thread_mlfqs == true){
    t->nice = thread_current()->nice; // for BSD
    t->recent_cpu = thread_current()->recent_cpu; // for BSD
    int left_priority = sub_x_y(int_to_fp(PRI_MAX), div_x_n(t->recent_cpu,4));  // for BSD
    int caled_priority = fp_to_int_zero(sub_x_n(left_priority, (t->nice *2)));         // for BSD
    if(caled_priority > PRI_MAX){
      t->priority = PRI_MAX;
    } else if(caled_priority < PRI_MIN){
      t->priority = PRI_MIN;
    } else{
      t->priority = caled_priority;
    }
  }


  for(int i=0; i<128; i++){
    t->fd[i] = NULL;
  }
  // JGH_END

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

  /* Add to run queue. */
  thread_unblock (t);

  // JGH
  /* ????????? thread ??????????????? ???????????? thread??? ???????????? ?????? ??? ????????? thread??? ??????????????? ????????? thread_yield()*/
  thread_check_preemption();

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  // JGH
  // list_push_back (&ready_list, &t->elem);
  list_insert_ordered(&ready_list, &t->elem, value_more, NULL);
  // JGH_END 
  t->status = THREAD_READY;
  intr_set_level (old_level);
}

/* Returns the name of the running thread. */
const char *
thread_name (void) 
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void) 
{
  struct thread *t = running_thread ();
  
  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void) 
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void) //-> () -> (int status)
{
  ASSERT (!intr_context ()); 

  // debug
  // if(! list_empty(&ready_list)){
  //   struct list_elem *e;
  //   for(e= list_begin(&ready_list); e != list_end(&ready_list); e= list_next(e)){
  //     printf("ready_list in %s\n", list_entry(e, struct thread, elem)->name);
  //     printf("%s status is %d\n", list_entry(e, struct thread, elem)->name, list_entry(e, struct thread, elem)->status);
  //     printf("next_thread_to_run is %s\n", next_thread_to_run()->name);
  //     printf("%s 's priority is %d\n", list_entry(e, struct thread, elem)->name, list_entry(e, struct thread, elem)->priority);
  //   }
  // }
  // debug_end 

  // //JGH move to process.c
  // for(int i=0; i<128; i++){
  //   if(thread_current()->fd[i] != NULL){
  //     // file_close(thread_current()->fd[i]);
  //     file_close(thread_current()->fd[i]);
  //     thread_current()->fd[i] = NULL;
  //   }
  // }

#ifdef USERPROG
  process_exit (); //-> () -> (status)
#endif

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  intr_disable ();
  list_remove (&thread_current()->allelem);
  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread) {
    // list_push_back (&ready_list, &cur->elem);
    list_insert_ordered(&ready_list, &cur->elem, value_more, NULL);
  }
  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority) 
{
  if(thread_mlfqs != true){
    thread_current ()->init_priority = new_priority;
    // JGH 

    thread_lock_refresh();
    // ???????????? ?????? ??? ??????????????? ?????? ????????? ??????????????? ?????? 
    thread_check_preemption();
    
    // JGH_END
  }
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice UNUSED) 
{
  enum intr_level old_level;
  old_level = intr_disable ();

  thread_current()->nice = nice;

  struct thread *t = thread_current();
  // idle??? priority ?????? 
  if(strcmp(t->name, "idle") != 0){
    int left_priority = sub_x_y(int_to_fp(PRI_MAX), div_x_n(t->recent_cpu,4));  // for BSD
    int caled_priority = fp_to_int_zero(sub_x_n(left_priority, (t->nice *2)));// for BSD
    if(caled_priority > PRI_MAX){
      t->priority = PRI_MAX;
    } else if(caled_priority < PRI_MIN){
      t->priority = PRI_MIN;
    }
  }

  // bsd_cal_priority();
  
  intr_set_level (old_level);
  thread_check_preemption();
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{
  enum intr_level old_level;
  old_level = intr_disable ();

  int nice = thread_current()->nice;

  intr_set_level (old_level);
  return nice;
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void) 
{
  enum intr_level old_level;
  old_level = intr_disable ();

  int get_load_avg = fp_to_int_near(mul_x_n(load_avg, 100));

  intr_set_level (old_level);
  return get_load_avg;
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void) 
{
  enum intr_level old_level;
  old_level = intr_disable ();

  int recent_cpu = fp_to_int_near(mul_x_n(thread_current()->recent_cpu, 100));

  intr_set_level (old_level);
  return recent_cpu;
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED) 
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;) 
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux) 
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */ //-> () -> (0)
}

/* Returns the running thread. */
struct thread *
running_thread (void) 
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  enum intr_level old_level;

  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;
  t->priority = priority;
  t->magic = THREAD_MAGIC;

  old_level = intr_disable ();
  list_push_back (&all_list, &t->allelem);
  intr_set_level (old_level);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size) 
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void) 
{
  if (list_empty (&ready_list))
    return idle_thread;
  else
    return list_entry (list_pop_front (&ready_list), struct thread, elem);
}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();
  
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
    {
      ASSERT (prev != cur);
      palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void) 
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void) 
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);

// // JGH : find given tid_t's struct thread
// struct thread * 
// thread_find(tid_t tid){
//   struct thread *t; 
//   struct list_elem *e;


//   for(e = list_begin(&all_list); e!= list_end(&all_list);e=e->next){
//     t = list_entry(e, struct thread, allelem);
//     if(t->tid == tid){
//       return t;
//     }
//   }
//   return NULL;
// }

/* timer_sleep() call thread_sleeping 
1. set sleep_time 
2. block thread
3. push in sleeping_list */

void 
thread_sleeping(int64_t ticks){
  struct thread *c = thread_current();
  enum intr_level old_level;

  old_level = intr_disable ();

  if(c != idle_thread){
    c->wakeup_time = ticks;
    list_push_back(&sleeping_list, &c->elem);
    thread_block();
  }
  intr_set_level (old_level);
  
  // printf("end thread_sleeping\n");
  // sema_up(&(c->sleeping_sema));
}


/* timer_interrupt()-> thread_tick()-> thread_wake_up()
look up thread struct of first element in sleeping_list
if time when thread was slept < 100 
1. pop at sleep_list
2. set sleep_time, 0
3. set thread status THREAD_READY
4. push in ready_listx
5. thread_yield()
    */
void 
thread_wake_up(){
  // printf("... start point thread_wakeup()\n");
  if(! list_empty(&sleeping_list)){
    struct list_elem *e;
    int64_t start = timer_ticks ();
    for(e = list_begin(&sleeping_list); e!= list_end(&sleeping_list); e=list_next(e)){
      struct thread *c = list_entry(e, struct thread, elem);

      if (start >= c->wakeup_time){
        e= list_remove(e);
        e= list_prev(e);
        c->wakeup_time = 0;
        thread_unblock(c);
      // thread_yield();
      }
    }
    // printf("... end point thread_wakeup()\n");
    // sema_up(&(thread_current()->wakeup_sema));
  }
}

/* Returns true if value A is more than value B, false
   otherwise. */
bool
value_more (const struct list_elem *a_, const struct list_elem *b_,
            void *aux UNUSED) 
{
  const struct thread *a = list_entry (a_, struct thread, elem);
  const struct thread *b = list_entry (b_, struct thread, elem);
  
  return a->priority > b->priority;
}

/* Returns true if value A is less than value B, false
   otherwise. */
bool
value_less (const struct list_elem *a_, const struct list_elem *b_,
            void *aux UNUSED) 
{
  const struct thread *a = list_entry (a_, struct thread, elem);
  const struct thread *b = list_entry (b_, struct thread, elem);
  
  return a->priority < b->priority;
}

void 
thread_check_preemption(void)
{ 
  if(! list_empty(&ready_list)){
    list_sort(&ready_list, value_more, NULL);
    struct thread *first_ready_thread = list_entry(list_begin(&ready_list), struct thread, elem);

    if(first_ready_thread->priority > thread_get_priority()){
      thread_yield();
    }
  }
  
}

void
thread_lock_acquire(struct lock *lock){
  if(lock->holder != NULL){
    thread_current()->lock_wait = lock;               // thread_current??? wait?????? ?????? lock pointer?????? 
    list_insert_ordered(&lock->holder->lock_waiter, &thread_current()->lock_waiter_elem, value_more_waiter, NULL);
    // prev
    // list_push_back(&lock->holder->lock_waiter, &thread_current()->lock_waiter_elem);  // holder??? lock_waiter??? thread_current ??????
    
    // holder's lock_waiter?????? ?????? priority??? ??? ??? ?????? 
    // prev
    // struct thread *max_p_thread = NULL;  
    // if(! list_empty(&lock->holder->lock_waiter)){  
    //   struct list_elem *max_elem = list_max(&lock->holder->lock_waiter, value_less, NULL);  
    //   max_p_thread = list_entry(max_elem, struct thread, elem);  
    // }
    // new 
    struct thread *max_p_thread = NULL;
    if(! list_empty(&lock->holder->lock_waiter)){
      if(list_entry(list_begin(&lock->holder->lock_waiter), struct thread, lock_waiter_elem)->priority > thread_current()->priority){
        max_p_thread = list_entry(list_begin(&lock->holder->lock_waiter), struct thread, lock_waiter_elem);
      } else {
        max_p_thread = thread_current();
      }
    }
    // new_end

  // nested priority(depth = 8). max_p_thread->priority??? lock??? ????????? ?????? thread??? donation. 
    struct thread *holder_of_holder = lock->holder;
    int count = 0; 
    while(count < 8){
      if(holder_of_holder->lock_wait != NULL && max_p_thread != NULL){
        count += 1;
        holder_of_holder->priority = max_p_thread->priority; // holder??? priority ?????? 
        // struct list_elem *e;
        // for(e= list_begin(&holder_of_holder->lock_waiter); e!= list_end(&holder_of_holder->lock_waiter); e= list_next(e)){
        //   list_entry(e, struct thread, elem)-> priority = max_p_thread->priority; // holder??? waiter?????? prioriry ?????? 
        // }
        holder_of_holder = holder_of_holder->lock_wait->holder;
      } else {
        holder_of_holder->priority = max_p_thread->priority;
        break;
      }
    }
  }
}


// void 
// thread_lock_release(struct lock *lock){
// // lock_waiter?????? release??? lock??? lock_wait?????? ?????? thread ????????? ?????? 
//   struct list_elem *e;
//   if(! list_empty(&thread_current()->lock_waiter)){
//     for(e= list_begin(&thread_current()->lock_waiter); e!= list_end(&thread_current()->lock_waiter); e=list_next(e)){
//       if(list_entry(e, struct thread, lock_waiter_elem)->lock_wait == lock){
//         e= list_remove(e);
//         e= list_prev(e);
//       }
//     }
//   }
  
//   // init_priority??? ?????????
//   thread_current()->priority = thread_current()->init_priority;

//   // lock_waiter?????? ?????? ?????? priority??? ?????? ?????? ?????? priority??? ?????? ?????????  init_priority??? ?????? 
//   if(! list_empty(&thread_current()->lock_waiter)){
//     if(thread_get_priority() < list_entry(list_max(&thread_current()->lock_waiter, value_less_waiter, NULL), struct thread, lock_waiter_elem)->priority){
//       thread_current()->priority = list_entry(list_max(&thread_current()->lock_waiter, value_less_waiter, NULL), struct thread, lock_waiter_elem)->priority;
//     }
//   }
//   // jgh_end
// }

void thread_lock_refresh(void){
   // init_priority??? ?????????
  thread_current()->priority = thread_current()->init_priority;

  // lock_waiter?????? ?????? ?????? priority??? ?????? ?????? ?????? priority??? ?????? ?????????  init_priority??? ?????? 
  if(! list_empty(&thread_current()->lock_waiter)){
    list_sort(&thread_current()->lock_waiter, value_more_waiter, NULL);

    struct thread *first_waiter_thread = list_entry(list_front(&thread_current()->lock_waiter), struct thread, lock_waiter_elem);
    if(first_waiter_thread->priority > thread_current()->priority){
      thread_current()->priority = first_waiter_thread->priority;
    }
  }
}

void 
thread_lock_remove(struct lock *lock){
  // lock_waiter?????? release??? lock??? lock_wait?????? ?????? thread ????????? ?????? 
  struct list_elem *e;
  if(! list_empty(&thread_current()->lock_waiter)){
    for(e= list_begin(&thread_current()->lock_waiter); e!= list_end(&thread_current()->lock_waiter); e=list_next(e)){
      if(list_entry(e, struct thread, lock_waiter_elem)->lock_wait == lock){
        e= list_remove(e);
        e= list_prev(e);
      }
    }
  }
}

bool
value_more_waiter(const struct list_elem *a_, const struct list_elem *b_,
            void *aux UNUSED)
{
  const struct thread *a = list_entry (a_, struct thread, lock_waiter_elem);
  const struct thread *b = list_entry (b_, struct thread, lock_waiter_elem);
  
  return a->priority > b->priority;
}

bool
value_less_waiter(const struct list_elem *a_, const struct list_elem *b_,
            void *aux UNUSED)
{
  const struct thread *a = list_entry (a_, struct thread, lock_waiter_elem);
  const struct thread *b = list_entry (b_, struct thread, lock_waiter_elem);
  
  return a->priority < b->priority;
}

void
thread_aging(void){
  struct list_elem *e; 
  for(e= list_begin(&ready_list); e!= list_end(&ready_list); e= list_next(e)){
    struct thread *t = list_entry(e, struct thread, elem);
    // printf("%s 's priority is %d\n", t->name, t->priority); // debug
    t->priority += 1;
  }
  // thread_check_preemption();
}

// priority = PRI_MAX - (recent_cpu/4) - (nice*2)
// cal when init thread and every 4 ticks 
void bsd_cal_priority(void){
  struct list_elem *e;
  for(e= list_begin(&all_list); e!= list_end(&all_list); e= list_next(e)){
    struct thread *t = list_entry(e, struct thread, allelem);
    int left_priority = sub_x_y(int_to_fp(PRI_MAX), div_x_n(t->recent_cpu,4));  // for BSD
    int caled_priority = fp_to_int_zero(sub_x_n(left_priority, (t->nice *2)));// for BSD
    if(caled_priority > PRI_MAX){
      t->priority = PRI_MAX;
    } else if(caled_priority < PRI_MIN){
      t->priority = PRI_MIN;
    } else{
      t->priority = caled_priority;
    }
  }
}

// recent_cpu = (2*load_avg)/(2*load_avg+1) * recent_cpu + nice
void bsd_cal_recent_cpu(void){
  int left_rate_of_decay = mul_x_n(load_avg, 2);
  int right_rate_of_decay = add_x_n(mul_x_n(load_avg, 2), 1);
  int rate_of_decay = div_x_y(left_rate_of_decay, right_rate_of_decay);
  struct list_elem *e;
  int prev_recent_cpu;
  int left_recent_cpu;
  for(e= list_begin(&all_list); e!= list_end(&all_list); e= list_next(e)){
    struct thread *t = list_entry(e, struct thread, allelem);
    if(strcmp(t->name, "idle") != 0){
      prev_recent_cpu = t->recent_cpu;
      left_recent_cpu = mul_x_y(rate_of_decay, prev_recent_cpu);
      t->recent_cpu = add_x_n(left_recent_cpu, t->nice);
    }
  }
}
 
void bsd_cal_load_avg(void){
  int ready_threads =0;
  struct list_elem *e;
  for(e= list_begin(&all_list); e!= list_end(&all_list); e= list_next(e)){
    struct thread *t = list_entry(e, struct thread, allelem);
    if(t->status == THREAD_RUNNING || t->status == THREAD_READY){
      if(strcmp(t->name, "idle") != 0){
        ready_threads += 1;
      }
    }
  }
  // load_avg = (59/60)*load_avg + (1/60)*ready_threads;
  int div_59_60 = div_x_y(int_to_fp(59), int_to_fp(60));
  int div_1_60 = div_x_y(int_to_fp(1), int_to_fp(60));
  int left_load_avg = mul_x_y(div_59_60, load_avg);
  int right_load_avg = mul_x_n(div_1_60, ready_threads);
  load_avg = add_x_y(left_load_avg, right_load_avg);
}

void 
bsd_increase_recent_cpu(void){
  if(strcmp(thread_current()->name, "idle") != 0){
    int plus_recent_cpu = add_x_n(thread_current()->recent_cpu, 1);
    thread_current()->recent_cpu = plus_recent_cpu;
  }
}