#include <stddef.h>
#include <stdlib.h>

#include "private.h"
#include "queue.h"
#include "sem.h"

struct semaphore {
    size_t count;
    queue_t waiting_list;
};

sem_t sem_create(size_t count)
{
    sem_t new_sem = malloc(sizeof(struct semaphore));
    if (new_sem == NULL) {
        return NULL;
    }
    new_sem->count = count;
    new_sem->waiting_list = queue_create();
    if (new_sem->waiting_list == NULL) {
        free(new_sem);
        return NULL;
    }
    return new_sem;
}

int sem_destroy(sem_t sem)
{
    if (sem == NULL) {
        return -1;
    }
    if (queue_length(sem->waiting_list) > 0) {
        return -1;
    }
    queue_destroy(sem->waiting_list);
    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    if (sem == NULL) {
        return -1;
    }
    preempt_disable();
    while (sem->count == 0) {
        struct uthread_tcb *current_thread = uthread_current();
        if (current_thread == NULL) {
            preempt_enable();
            return -1;
        }
        queue_enqueue(sem->waiting_list, current_thread);
        preempt_enable();
        uthread_block();
        preempt_disable();
    }
    sem->count--;
    preempt_enable();
    return 0;
}

int sem_up(sem_t sem)
{
    if (sem == NULL) {
        return -1;
    }
    preempt_disable();
    sem->count++;
    if (queue_length(sem->waiting_list) > 0) {
        struct uthread_tcb *current_thread;
        queue_dequeue(sem->waiting_list, (void **)&current_thread);
        uthread_unblock(current_thread);
    }
    preempt_enable();
    return 0;
}
