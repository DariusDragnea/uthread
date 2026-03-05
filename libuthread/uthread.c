#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "queue.h"
#include "uthread.h"

static ucontext_t main_ctx;
static struct uthread_tcb *current_thread = NULL;
static queue_t thread_queue;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    ZOMBIE,
} thread_state_t;

struct uthread_tcb {
    thread_state_t state;
    uthread_ctx_t *context;
    void *stack;
};

int check_if_all_threads_blocked(queue_t queue)
{
    int all_blocked = 1;
    int length = queue_length(queue);
    struct uthread_tcb *thread;

    for (int i = 0; i < length; i++) {
        queue_dequeue(queue, (void **)&thread);
        if (thread->state != BLOCKED) {
            all_blocked = 0;
        }
        queue_enqueue(queue, thread);
    }

    return all_blocked;
}

void zombify_all_threads(queue_t queue)
{
    int length = queue_length(queue);
    struct uthread_tcb *thread;

    for (int i = 0; i < length; i++) {
        queue_dequeue(queue, (void **)&thread);
        thread->state = ZOMBIE;
        queue_enqueue(queue, thread);
    }
}

struct uthread_tcb *uthread_current(void) { return current_thread; }

void uthread_yield(void)
{
    struct uthread_tcb *current = uthread_current();
    if (current == NULL) {
        return;
    }
    current->state = READY;
    uthread_ctx_switch(current->context, &main_ctx);
    return;
}

void uthread_exit(void)
{
    struct uthread_tcb *current = uthread_current();
    if (current == NULL) {
        return;
    }
    current->state = ZOMBIE;
    uthread_ctx_switch(current->context, &main_ctx);
    return;
}

int uthread_create(uthread_func_t func, void *arg)
{
    preempt_disable();
    struct uthread_tcb *new_thread = malloc(sizeof(struct uthread_tcb));
    if (new_thread == NULL) {
        return -1;
    }
    new_thread->state = READY;
    new_thread->context = malloc(sizeof(uthread_ctx_t));
    if (new_thread->context == NULL) {
        free(new_thread);
        return -1;
    }
    new_thread->stack = uthread_ctx_alloc_stack();
    if (new_thread->stack == NULL) {
        free(new_thread->context);
        free(new_thread);
        return -1;
    }
    int retval =
        uthread_ctx_init(new_thread->context, new_thread->stack, func, arg);
    if (retval == -1) {
        free(new_thread->context);
        uthread_ctx_destroy_stack(new_thread->stack);
        free(new_thread);
        return -1;
    }
    queue_enqueue(thread_queue, new_thread);
    preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    thread_queue = queue_create();
    if (thread_queue == NULL) {
        return -1;
    }

    int retval = uthread_create(func, arg);
    if (retval == -1) {
        return -1;
    }

    struct uthread_tcb *item;
    void *stack = uthread_ctx_alloc_stack();
    int init_retval = uthread_ctx_init(&main_ctx, stack, NULL, NULL);
    if (init_retval == -1) {
        uthread_ctx_destroy_stack(stack);
        queue_destroy(thread_queue);
        return -1;
    }

    if (preempt) {
        preempt_start(preempt);
        preempt_disable();
    }
    while (queue_length(thread_queue) > 0) {
        queue_dequeue(thread_queue, (void **)&item);
        if (item->state == READY) {
            current_thread = item;
            item->state = RUNNING;
            preempt_enable();
            uthread_ctx_switch(&main_ctx, item->context);
            preempt_disable();
            queue_enqueue(thread_queue, item);
        } else if (item->state == BLOCKED) {
            if (check_if_all_threads_blocked(thread_queue)) {
                zombify_all_threads(thread_queue);
                item->state = ZOMBIE;
            }
            queue_enqueue(thread_queue, item);
        } else if (item->state == ZOMBIE) {
            uthread_ctx_destroy_stack(item->stack);
            free(item->context);
            free(item);
        }
    }

    uthread_ctx_destroy_stack(stack);

    if (preempt) {
        preempt_disable();
        preempt_stop();
    }
    return 0;
}

void uthread_block(void)
{
    struct uthread_tcb *current = uthread_current();
    if (current == NULL) {
        return;
    }
    current->state = BLOCKED;
    uthread_ctx_switch(current->context, &main_ctx);
    return;
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    if (uthread == NULL) {
        return;
    }
    uthread->state = READY;
    return;
}