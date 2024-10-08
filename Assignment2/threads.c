// TODO: necessary includes, if any
#include "threads.h"
#include <stdio.h>
// TODO: necessary defines, if any
struct thread;
struct thread *current_thread = NULL;

// TODO: definition of struct thread
struct thread {};

struct thread *thread_create(void (*f)(void *), void *arg);
void thread_add_runqueue(struct thread *t);
void thread_yield(void);
void dispatch(void);
void schedule(void);
void thread_exit(void);
void thread_start_threading(void);
