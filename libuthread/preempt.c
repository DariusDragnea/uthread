#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct itimerval timer;
struct itimerval old_timer;
struct sigaction old_action;

static bool preempt_enabled = false;

static void preempt_handler(int sig)
{
    (void)sig;
    uthread_yield();
}

void preempt_disable(void)
{
    if (!preempt_enabled) {
        return;
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}

void preempt_enable(void)
{
    if (!preempt_enabled) {
        return;
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void preempt_start(bool preempt)
{
    if (!preempt) {
        return;
    }
    preempt_enabled = true;

    struct sigaction sa;
    sa.sa_handler = &preempt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGVTALRM, &sa, &old_action);

    getitimer(ITIMER_VIRTUAL, &old_timer);
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000000 / HZ;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000 / HZ;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

void preempt_stop(void)
{
    preempt_enabled = false;
    sigaction(SIGVTALRM, &old_action, NULL);
    setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
}
