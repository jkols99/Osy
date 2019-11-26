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

static void kill_thread(void) {
        thread_t* thread_to_kill = get_current_thread();
        thread_t* next_thread = NULL;
        if (thread_to_kill->is_waiting_for_me != NULL)
            next_thread = thread_to_kill->is_waiting_for_me;
        remove_all_dependencies(queue, thread_to_kill);
        // printk("Before delete:\n");
        // dump_queue_info(queue);
        scheduler_remove_thread(thread_to_kill);
        // printk("After delete:\n");
        // dump_queue_info(queue);
        kfree(thread_to_kill);
        printk("Killing thread %s off\n", thread_to_kill->name);
        thread_to_kill = NULL;
        if (next_thread == NULL)
            scheduler_schedule_next();
        else
            thread_switch_to(next_thread);        
}

static void wrap(thread_t* new_thread)
{
    (*new_thread->entry_func)(new_thread->data);
    kill_thread();
}

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
    context->ra = (unative_t)wrap;
    new_thread->context = context;
    context->a0 = (unative_t)new_thread;
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
    current_thread->status = READY;
    // printk("Current running thread: %s\n", current_thread->name);
    rotate(current_thread);
    // printk("AFTER REMOVE DUMP\n");
    // dump_queue_info(queue);
    // printk("After yield dump: \n");
    // dump_queue_info(queue);
    scheduler_schedule_next();
} // init -> waiting, worker -> ready

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
    while(1) {
        kill_thread();
    }
}

static bool is_thread_in_queue(thread_t* target_thread)
{
    qnode_t* temp = queue->front;

    while (1)
    {
        if (temp == NULL)
            return false;
        if (temp->key == target_thread)
            return true;
        temp = temp->next;
    }
}


/** Tells if thread already called thread_finish() or returned from the entry
 * function.
 *
 * @param thread Thread in question.
 */
bool thread_has_finished(thread_t* thread) {
    if (thread == NULL)
        return true;
    if (is_thread_in_queue(thread))
        return false;
    
    return true;    
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
    if (thread == NULL)
        return EEXITED;

    if (thread->status == WAITING)
        return EINVAL;

    if (thread->status == READY || thread->status == RUNNING )
        return EOK;

    thread->status = READY;
    // according to suspend test, we should call yield here, because wake-upper will die before
    // it can be joined with INIT thread and still return EOK from that join
    thread_yield();

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
    // dump_queue_info(queue);
    if (!is_thread_in_queue(thread))
        return EINVAL;
    else
    {
        printk("join thread was valid\n");
    }
    
    thread_t* current_thread = get_current_thread();
    thread->is_waiting_for_me = current_thread;
    current_thread->waiting_for = thread;
    // printk("Before suspend\n");
    thread_get_current()->status = WAITING;
    // printk("Join dump:\n");
    // dump_queue_info(queue);
    thread_switch_to(thread);
    if (retval != NULL)
    {
        *retval = (void*)current_thread->context->v0;
        printk("v0: %p\n", (void*)current_thread->context->v0);
    }
    else
        printk("Was null\n");
    
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
    // printk("DUMP IN THREAD SWITCH\n");
    // dump_queue_info(queue);
    if (current_thread == NULL)
    {
        printk("Switching from NULL to %s\n", thread->name);
        if (thread == NULL)
            printk("Thread in switch was NULL\n");
        else
        {
            printk("Wasnt null");
        }
        
        thread->status = RUNNING;
        cpu_switch_context(NULL, (void**)&thread->stack_top, 1);
    }
    else
    {
        printk("Switching from %s to %s\n", current_thread->name, thread->name);
        thread->status = RUNNING;
        rotate(current_thread);
        cpu_switch_context((void**)&current_thread->stack_top, (void**)&thread->stack_top, 1);
    }
}
