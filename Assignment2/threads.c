#include "threads.h"
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 4096 // Define a default stack size
// definition of struct thread
struct thread {
  jmp_buf context;
  void (*fct)(void *);
  void *arg;
  void *stack;
  void *stack_pointer;
  struct thread *next;
};
struct thread *head = NULL;
struct thread *current_thread = NULL;

void *align_stack(void *stack_top) {
  uintptr_t stack_addr = (uintptr_t)stack_top;
  // Align to the nearest 16-byte boundary
  stack_addr &= ~(uintptr_t)0xF;
  return (void *)stack_addr;
}

// Function to create and initialize a thread
struct thread *thread_create(void (*f)(void *), void *arg) {
  // Allocate memory for the thread structure
  struct thread *t = (struct thread *)malloc(sizeof(struct thread));
  if (t == NULL) {
    perror("Failed to allocate thread structure");
    return NULL;
  }

  // Allocate memory for the stack
  void *stack = malloc(STACK_SIZE);
  if (stack == NULL) {
    perror("Failed to allocate stack");
    free(t); // Clean up if stack allocation fails
    return NULL;
  }

  // Calculate the top of the stack (stack grows down)
  void *stack_top = (void *)((char *)stack + STACK_SIZE);

  // Ensure the stack pointer is 16-byte aligned
  stack_top = align_stack(stack_top);

  // Initialize the thread structure
  t->fct = f;
  t->arg = arg;
  t->stack = stack;
  t->stack_pointer = stack_top;

  if (setjmp(t->context) == 0) {
    // Set the initial stack pointer to be used when this thread is dispatched
    // Here we could modify the saved registers to point to the top of the
    // stack. In a more complete implementation, you'd modify `context`
    // registers to properly set the stack.
  }
  return t;
}

void thread_add_runqueue(struct thread *t) {
  if (head == NULL) {
    // If the runqueue is empty, point head to the new thread
    head = t;
    head->next =
        t; // Point the thread to itself, making it the only thread in the ring
    current_thread = head;
  } else {
    // If the runqueue is not empty, find the last thread in the ring
    struct thread *last = head;
    while (last->next != head) {
      last = last->next;
    }

    // Insert the new thread at the end of the ring
    last->next = t;
    t->next = head; // Ensure the new thread points back to the head
  }
}

void schedule(void) {
  if (head != NULL) {
    current_thread = current_thread->next;
  }
}
void dispatch() {
  if (current_thread != NULL) {
    // Restore the next thread's context and jump to its saved state
    longjmp(current_thread->context, 1);
  }
}
void thread_yield(void) {
  if (current_thread != NULL) {
    // Save the current thread's context. If setjmp returns 0, we are suspending
    // the thread.
    if (setjmp(current_thread->context) == 0) {
      // Call the scheduler to select the next thread
      schedule();
      // Dispatch the next thread
      dispatch();
    }
    // When the thread is resumed, it will return here with a non-zero value
  }
}
// Thread exit function: removes the current thread from the runqueue and frees
// its resources
void thread_exit() {
  struct thread *current_thread = current_thread;

  if (current_thread == NULL) {
    return; // No thread to exit
  }

  // If this is the last thread in the runqueue
  if (current_thread->next == current_thread) {
    // This is the only thread left, so we can clean up and exit the system
    free(current_thread->stack);
    free(current_thread);
    current_thread = NULL;
    printf("Last thread exited. No more threads to run.\n");
    exit(0); // Exit the program or return control to the main thread
  }

  // Find the previous thread in the circular linked list
  struct thread *prev_thread = current_thread;
  while (prev_thread->next != current_thread) {
    prev_thread = prev_thread->next;
  }

  // Remove the current thread from the ring
  prev_thread->next = current_thread->next;

  // Set the current thread to the next thread
  current_thread = current_thread->next;

  // Free the current thread's resources
  free(current_thread->stack);
  free(current_thread);

  // Dispatch the next thread (we must not return to the current thread, as it's
  // been removed)
  dispatch();

  // We should never reach here since dispatch switches to another thread
}

void thread_start_threading() {
  if (head == NULL) {
    printf("No threads to run.\n");
    return; // No threads added to the system
  }

  // Call the scheduler to choose the first thread
  schedule();

  // Dispatch the first thread and start threading
  dispatch();

  // We should never reach here, as dispatch transfers control to a thread
  // If we reach here, it means there are no more threads to run
  printf("Threading system stopped. No more threads.\n");
}
