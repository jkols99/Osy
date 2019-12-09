// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <errno.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/heap.h>
#include <proc/context.h>
#include <proc/scheduler.h>

/** Initialize support for threading.
 *
 * Called once at system boot.
 */
void threads_init(void) {
    //unneccessary...
}

static void* retvalue;

/**
 * Kills current thread, this function is either called 
 * 1) After thread finishes naturally
 * 2) Its forced to be killed via thread_finish
 * Next thread is determined by:
 * 1) Natural call of scheduler_schedule_next
 * 2) Follower of thread being killed
 * 
 * Killed thread is removed from all dependencies in queue 
 * and then from queue
*/
static void kill_thread(void) {
    interrupts_disable();
    thread_t* thread_to_kill = get_current_thread();
    thread_t* next_thread = NULL;

    if (thread_to_kill->follower != NULL)
        next_thread = thread_to_kill->follower;

    remove_all_dependencies(queue, thread_to_kill);

    scheduler_remove_thread(thread_to_kill);
    thread_to_kill->status = FINISHED;
    thread_to_kill->follower = NULL;
    thread_to_kill->following = NULL;
    kfree(thread_to_kill);
    thread_to_kill = NULL;
    if (next_thread == NULL) {
        interrupts_restore(false);
        scheduler_schedule_next();
    } else {
        interrupts_restore(false);
        thread_switch_to(next_thread);
    }
}

/**
 * Wraps together entry function and kill function,
 * so after thread finishes its entry function,
 * it can proceed to get killed
 */
static void wrap(thread_t* new_thread) {
    retvalue = (*new_thread->entry_func)(new_thread->data);
    kill_thread();
}

/** Create a new thread.
 *
 * The thread is automatically placed into the queue of ready threads.
 *
 * This function allocates space for both stack and the thread_t structure
 * (hence the double <code>**</code> in <code>thread_out</code>.
 *
 * @param thread_out Where to place the initialized thread_t structure.
 * @param entry Thread entry function.
 * @param data Data for the entry function.
 * @param flags Flags (unused).
 * @param name Thread name (for debugging purposes).
 * @return Error code.
 * @retval EOK Thread was created and started (added to ready queue).
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval INVAL Invalid flags (unused).
 */
errno_t thread_create(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name) {
    interrupts_disable();
    thread_t* new_thread = (thread_t*)kmalloc(sizeof(thread_t));

    if (new_thread == NULL)
        return ENOMEM;

    for (size_t i = 0; i < THREAD_NAME_MAX_LENGTH; i++) {
        if (name == NULL)
            break;
        new_thread->name[i] = *name++;
    }

    new_thread->entry_func = entry;
    new_thread->data = data;
    new_thread->status = READY;
    new_thread->following = NULL;
    new_thread->follower = NULL;

    new_thread->stack = kmalloc(THREAD_STACK_SIZE);
    if (new_thread->stack == NULL)
        return ENOMEM;
    new_thread->stack_top = (void*)((uintptr_t)new_thread->stack + THREAD_STACK_SIZE - sizeof(context_t));
    context_t* context = (context_t*)new_thread->stack_top;
    context->status = 0xff01;
    context->sp = (unative_t)new_thread->stack_top + sizeof(context_t);
    context->ra = (unative_t)wrap;
    new_thread->context = context;
    context->a0 = (unative_t)new_thread;

    *thread_out = new_thread;
    scheduler_add_ready_thread(*thread_out);

    interrupts_restore(false);
    return EOK;
}

/** Return information about currently executing thread.
 *
 * @retval NULL When no thread was started yet.
 */
thread_t* thread_get_current(void) {
    return get_current_thread();
}

/** Yield the processor. 
 * Puts current thread to the end of the queue
 * and schedules next thread
*/
void thread_yield(void) {
    interrupts_disable();
    thread_t* current_thread = get_current_thread();
    current_thread->status = READY;
    rotate(current_thread);
    interrupts_restore(false);
    scheduler_schedule_next();
}

/** Current thread stops execution and is not scheduled until woken up. */
void thread_suspend(void) {
    thread_t* current_thread = get_current_thread();
    current_thread->status = SUSPENDED;
    rotate(current_thread);
    scheduler_schedule_next();
}

/** Terminate currently running thread.
 *
 * Thread can (normally) terminate in two ways: by returning from the entry
 * function of by calling this function. The parameter to this function then
 * has the same meaning as the return value from the entry function.
 *
 * Note that this function never returns.
 *
 * @param retval Data to return in thread_join.
 */
void thread_finish(void* retval) {
    retvalue = retval;
    while (1) {
        kill_thread();
    }
}

/** Tells if thread already called thread_finish() or returned from the entry
 * function.
 *
 * @param thread Thread in question.
 */
bool thread_has_finished(thread_t* thread) {
    if (thread == NULL || thread->status == FINISHED)
        return true;

    return false;
}

/** Wakes-up existing thread.
 *
 * Note that waking-up a running (or ready) has no effect (i.e. the
 * function shall not count wake-ups and suspends).
 *
 * Note that waking-up a thread does not mean that it will immediatelly start
 * executing.
 *
 * @param thread Thread to wake-up.
 * @return Error code.
 * @retval EOK Thread was woken-up (or was already ready/running).
 * @retval EINVAL Invalid thread.
 * @retval EEXITED Thread already finished its execution.
 */
errno_t thread_wakeup(thread_t* thread) {
    if (thread == NULL || thread->status == WAITING)
        return EINVAL;

    if (thread->status == FINISHED)
        return EEXITED;

    if (thread->status == READY || thread->status == RUNNING)
        return EOK;

    thread->status = READY;
    // according to suspend test, we should call yield here, because wake-upper will die before
    // it can be joined with INIT thread and still return EOK from that join
    // thread_yield();

    return EOK;
}

/** Joins another thread (waits for it to terminate.
 *
 * Note that <code>retval</code> could be <code>NULL</code> if the caller
 * is not interested in the returned value.
 *
 * @param thread Thread to wait for.
 * @param retval Where to place the value returned from thread_finish.
 * @return Error code.
 * @retval EOK Thread was joined.
 * @retval EBUSY Some other thread is already joining this one.
 * @retval EINVAL Invalid thread.
 */
errno_t thread_join(thread_t* thread, void** retval) {
    if (thread == NULL)
        return EINVAL;

    if (thread->status == FINISHED)
        return EOK;

    if (thread->follower != NULL)
        return EBUSY;

    thread_t* current_thread = get_current_thread();
    thread->follower = current_thread;
    current_thread->following = thread;
    thread_get_current()->status = WAITING;

    thread_switch_to(thread);
    if (retval != NULL) {
        *retval = retvalue;
    }

    return EOK;
}

/** Switch CPU context tothread a different thread.
 *
 * Note that this function must work even if there is no current thread
 * (i.e. for the very first context switch in the system).
 *
 * @param thread Thread to switch to.
 */
void thread_switch_to(thread_t* thread) {
    interrupts_disable();
    thread_t* current_thread = get_current_thread();
    set_current_thread(thread);
    if (current_thread == NULL) {
        if (thread == NULL) {
            return;
        }

        thread->status = RUNNING;
        interrupts_restore(false);
        cpu_switch_context(NULL, (void**)&thread->stack_top, 1);
    } else {
        thread->status = RUNNING;
        rotate(current_thread);
        interrupts_restore(false);
        cpu_switch_context((void**)&current_thread->stack_top, (void**)&thread->stack_top, 1);
    }
}
