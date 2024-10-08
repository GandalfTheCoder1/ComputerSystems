#include <stdio.h>
#include "threads.h"

void f3(void *arg)
{
    int i = 1000;
    while (i != 999) {
        printf("thread 3: %d\n", i++);
        if (i == 1010) {
            i = 990;
        }
        thread_yield();
    }
}

void f2(void *arg)
{
    int i = 0;
    while(i != -1) {
        printf("thread 2: %d\n",i++);
        if (i == 10) {
            i = -10;
        }
        thread_yield();
    }
}

void f1(void *arg)
{
    int i = 100;
    struct thread *t2 = thread_create(f2, NULL);
    thread_add_runqueue(t2);
    struct thread *t3 = thread_create(f3, NULL);
    thread_add_runqueue(t3);
    while(i != 99) {
        printf("thread 1: %d\n", i++);
        if (i == 110) {
            i = 90;
        }
        thread_yield();
    }
}

int main(int argc, char **argv)
{
    struct thread *t1 = thread_create(f1, NULL);
    thread_add_runqueue(t1);
    thread_start_threading();
    printf("\nexited\n");
    return 0;
}
