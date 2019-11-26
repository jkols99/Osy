// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <errno.h>
#include <mm/heap.h>
#include <proc/context.h>
#include <proc/scheduler.h>
#include <lib/print.h>
#include <debug/code.h>

/** Initialize support for threading.
 *
 * Called once at system boot.
 */
void threads_init(void) {
    //unneccessary...
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
    new_thread->waiting_for = NULL;
    new_thread->is_waiting_for_me = NULL;
    new_thread->stack = kmalloc(THREAD_STACK_SIZE);
    new_thread->stack_top = (void*)((uintptr_t)new_thread->stack + THREAD_STACK_SIZE - sizeof(context_t));
    context_t* context = (context_t*)new_thread->stack_top;
    context->status = 0xff01;
    context->sp = (unative_t)new_thread->stack_top + sizeof(context_t);
    context->ra = (unative_t)entry;
    context->a0 = (unative_t)data;
    new_thread->context = context;
    *thread_out = new_thread;
    scheduler_add_ready_thread(*thread_out);
    return EOK;
}

/** Return information about currently executing thread.
 *
 * @retval NULL When no thread was started yet.
 */
thread_t* thread_get_current(void) {
    return get_current_thread();
}

/** Yield the processor. */
void thread_yield(void) {
    thread_t* current_thread = get_current_thread();
    printk("Current running thread: %s\n", current_thread->name);
    scheduler_remove_thread(current_thread);
    printk("AFTER REMOVE DUMP\n");
    dump_queue_info(queue);
    scheduler_add_ready_thread(current_thread);
    printk("After yield dump: \n");
    dump_queue_info(queue);
    scheduler_schedule_next();
} // init -> waiting, worker -> ready

/** Current thread stops execution and is not scheduled until woken up. */
void thread_suspend(void) {
    thread_get_current()->status = WAITING;
    thread_yield();
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
    while (1) {
    }
    // panic(); //nesmie to sem dojst
}

/** Tells if thread already called thread_finish() or returned from the entry
 * function.
 *
 * @param thread Thread in question.
 */
bool thread_has_finished(thread_t* thread) {
    return thread->status == FINISHED;
}

/** Wakes-up existing thread.
 *
 * Note that waking-up a running (or ready) thread has no effect (i.e. the
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
    if (thread->status == READY || thread->status == RUNNING)
        return EINVAL;
    if (thread->status == FINISHED)
        return EEXITED;

    return ENOIMPL;
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
    thread_t* current_thread = get_current_thread();
    thread->is_waiting_for_me = current_thread;
    current_thread->waiting_for = thread;
    printk("Before suspend\n");
    thread_suspend();

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
    thread_t* current_thread = get_current_thread();
    set_current_thread(thread);
    printk("DUMP IN THREAD SWITCH\n");
    dump_queue_info(queue);
    if (current_thread == NULL)
    {
        printk("Switching from NULL to %s\n", thread->name);
        cpu_switch_context((void**)debug_get_stack_pointer(), (void**)&thread->stack_top, 1); //mozno stack pointer, moze byt aj null
    }
    else
    {
        printk("Switching from %s to %s\n", current_thread->name, thread->name);
        cpu_switch_context((void**)&current_thread->stack_top, (void**)&thread->stack_top, 1);
    }
}
