// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <drivers/machine.h>
#include <errno.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/as.h>
#include <mm/heap.h>
#include <proc/context.h>
#include <proc/process.h>
#include <proc/scheduler.h>

/** Initialize support for threading.
 *
 * Called once at system boot.
 */
void threads_init(void) {}

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
void kill_thread(bool run_next, bool corrupted) {
    bool ipl = interrupts_disable();
    thread_t* thread_to_kill = get_current_thread();
    thread_t* next_thread = NULL;

    if (corrupted) {
        thread_to_kill = NULL;
        scheduler_remove_thread(thread_to_kill);
        interrupts_restore(ipl);
        return;
    }

    if (thread_to_kill->follower != NULL)
        next_thread = thread_to_kill->follower;

    remove_all_dependencies(queue, thread_to_kill);

    scheduler_remove_thread(thread_to_kill);
    thread_to_kill->status = FINISHED;
    thread_to_kill->follower = NULL;
    thread_to_kill->following = NULL;
    kfree(thread_to_kill);
    thread_to_kill = NULL;
    if (run_next) {
        if (next_thread == NULL) {
            scheduler_schedule_next();
        } else {
            thread_switch_to(next_thread);
        }
    }
    interrupts_restore(ipl);
}

/**
 * Wraps together entry function and kill function,
 * so after thread finishes its entry function,
 * it can proceed to get killed
 */
static void wrap(thread_t* new_thread) {
    printk("Tututu\n");
    retvalue = (*new_thread->entry_func)(new_thread->data);
    printk("Tutu\n");
    kill_thread(true, false);
    printk("Tu\n");
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
    bool ipl = interrupts_disable();
    thread_t* new_thread = (thread_t*)kmalloc(sizeof(thread_t));

    if (new_thread == NULL) {
        return ENOMEM;
    }

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
    new_thread->is_userspace = (flags == 1);

    new_thread->stack = kmalloc(THREAD_STACK_SIZE);
    if (new_thread->stack == NULL) {
        // kfree(new_thread);
        interrupts_restore(ipl);
        return ENOMEM;
    }
    new_thread->stack_top = (void*)((uintptr_t)new_thread->stack + THREAD_STACK_SIZE - sizeof(context_t));
    context_t* context = (context_t*)new_thread->stack_top;
    context->status = 0xff01;
    context->sp = (unative_t)new_thread->stack_top + sizeof(context_t);
    context->ra = (unative_t)wrap;
    new_thread->context = context;
    context->a0 = (unative_t)new_thread;

    *thread_out = new_thread;
    scheduler_add_ready_thread(*thread_out);

    interrupts_restore(ipl);
    return EOK;
}

/** Return information about currently executing thread.
 *
 * @retval NULL When no thread was started yet.
 */
thread_t* thread_get_current(void) {
    bool ipl = interrupts_disable();
    thread_t* ret_thread = get_current_thread();
    interrupts_restore(ipl);
    return ret_thread;
}

/** Yield the processor. 
 * Puts current thread to the end of the queue
 * and schedules next thread
*/
void thread_yield(void) {
    bool ipl = interrupts_disable();
    printk("Interrupted\n");
    thread_t* current_thread = get_current_thread();
    if (current_thread == NULL)
        printk("Null curr thread\n");
    current_thread->status = READY;
    rotate(current_thread);
    scheduler_schedule_next();
    interrupts_restore(ipl);
}

/** Current thread stops execution and is not scheduled until woken up. */
void thread_suspend(void) {
    bool ipl = interrupts_disable();
    thread_t* current_thread = get_current_thread();
    current_thread->status = SUSPENDED;
    rotate(current_thread);
    scheduler_schedule_next();
    interrupts_restore(ipl);
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
        kill_thread(true, false);
    }
}

/** Tells if thread already called thread_finish() or returned from the entry
 * function.
 *
 * @param thread Thread in question.
 */
bool thread_has_finished(thread_t* thread) {
    bool ipl = interrupts_disable();
    bool ret_val;
    if (thread == NULL || thread->status == FINISHED)
        ret_val = true;
    else
        ret_val = false;
    interrupts_restore(ipl);

    return ret_val;
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
    bool ipl = interrupts_disable();
    errno_t ret_err;
    if (thread == NULL || thread->status == WAITING)
        ret_err = EINVAL;
    else if (thread->status == FINISHED)
        ret_err = EEXITED;
    else if (thread->status == READY || thread->status == RUNNING)
        ret_err = EOK;
    else {
        thread->status = READY;
        ret_err = EOK;
    }

    // according to suspend test, we should call yield here, because wake-upper will die before
    // it can be joined with INIT thread and still return EOK from that join
    // thread_yield();
    interrupts_restore(ipl);
    return ret_err;
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
    bool ipl = interrupts_disable();

    if (thread == NULL) {
        interrupts_restore(ipl);
        return EINVAL;
    }

    if (thread->status == KILLED) {
        interrupts_restore(ipl);
        return EKILLED;
    }

    if (thread->status == FINISHED) {
        interrupts_restore(ipl);
        return EOK;
    }

    if (thread->status < 0 || thread->status > 5) // consider corrupted threads as invalid, kill them without running next, meaning current thread continues
    {
        printk("Thread is corrupted\n");
        kill_thread(false, true);
        interrupts_restore(ipl);
        return EOK;
    }

    if (thread->follower != NULL) {
        interrupts_restore(ipl);
        return EOK;
    }
    thread_t* current_thread = get_current_thread();
    thread->follower = current_thread;
    current_thread->following = thread;
    thread_get_current()->status = WAITING;
    thread_switch_to(thread);
    if (retval != NULL) {
        *retval = retvalue;
    }

    if (thread->status == KILLED) {
        printk("Ekilled return\n");
        interrupts_restore(ipl);
        return EKILLED;
    }

    interrupts_restore(ipl);
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
    bool ipl = interrupts_disable();
    thread_t* current_thread = get_current_thread();
    set_current_thread(thread);

    if (current_thread == NULL) {
        if (thread == NULL) {
            return;
        }

        thread->status = RUNNING;
        if (thread->address_space == NULL)
            cpu_switch_context((void**)debug_get_stack_pointer(), (void**)&thread->stack_top, 1);
        else
            cpu_switch_context((void**)debug_get_stack_pointer(), (void**)&thread->stack_top, thread->address_space->asid);
    } else {
        thread->status = RUNNING;
        rotate(current_thread);
        if (thread->address_space == NULL)
            cpu_switch_context((void**)&current_thread->stack_top, (void**)&thread->stack_top, 1);
        else
            cpu_switch_context((void**)&current_thread->stack_top, (void**)&thread->stack_top, thread->address_space->asid);
    }
    interrupts_restore(ipl);
}

/** Create a new thread with new address space.
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
 * @param as_size Address space size, aligned at page size (0 is correct though not very useful).
 * @return Error code.
 * @retval EOK Thread was created and started (added to ready queue).
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval INVAL Invalid flags (unused).
 */
errno_t thread_create_new_as(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, size_t as_size) {
    bool ipl = interrupts_disable();
    errno_t ret_val = thread_create(thread_out, entry, data, flags, name);
    if (ret_val == ENOMEM) {
        printk("But thread was null\n");
        interrupts_restore(ipl);
        return ENOMEM;
    }
    (*thread_out)->address_space = as_create(as_size, 0);
    if ((*thread_out)->address_space == NULL) {
        printk("But as was null\n");
        // kfree(thread_out);
        interrupts_restore(ipl);
        return ENOMEM;
    }
    interrupts_restore(ipl);
    return ret_val;
}

/** Get address space of given thread. */
as_t* thread_get_as(thread_t* thread) {
    return thread->address_space;
}

/** Kills given thread.
 *
 * Note that this function shall work for any existing thread, including
 * currently running one.
 *
 * Joining a killed thread results in EKILLED return value from thread_join.
 *
 * @param thread Thread to kill.
 * @return Error code.
 * @retval EOK Thread was killed.
 * @retval EINVAL Invalid thread.
 * @retval EEXITED Thread already finished its execution.
 */
errno_t thread_kill(thread_t* thread) {
    bool ipl = interrupts_disable();
    if (thread == NULL) {
        interrupts_restore(ipl);
        return EINVAL;
    }

    if (thread->status == KILLED) {
        interrupts_restore(ipl);
        return EOK;
    }

    if (thread->status == FINISHED) {
        interrupts_restore(ipl);
        return EEXITED;
    }

    thread_t* current_thread = get_current_thread();

    thread_t* thread_to_kill = thread;

    remove_all_dependencies(queue, thread_to_kill);
    scheduler_remove_thread(thread_to_kill);
    thread_to_kill->status = KILLED;
    thread_to_kill->follower = NULL;
    thread_to_kill->following = NULL;
    if (thread_to_kill->address_space != NULL) {
        as_destroy(thread_to_kill->address_space);
    }
    kfree(thread_to_kill);
    thread_to_kill = NULL;
    if (thread == current_thread) {
        interrupts_restore(ipl);
        scheduler_schedule_next();
        return EOK;
    } else {
        interrupts_restore(ipl);
        return EOK;
    }
}
