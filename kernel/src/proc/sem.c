// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <exc.h>
#include <lib/queue.h>
#include <proc/sem.h>

/** Initializes given semaphore.
 *
 * @param sem Semaphore to initialize.
 * @param value Initial semaphore value (1 effectively creates mutex).
 * @return Error code.
 * @retval EOK Semaphore was successfully initialized.
 */
errno_t sem_init(sem_t* sem, int value) {
    interrupts_disable();
    queue_t* thread_queue = create_queue();
    sem_t new_sem = { value, thread_queue };
    *sem = new_sem;
    interrupts_restore(false);
    return EOK;
}

/** Destroys given semaphore.
 *
 * The function must panic if there are threads waiting for this semaphore.
 *
 * @param sem Semaphore to destroy.
 */
void sem_destroy(sem_t* sem) {
    bool ipl = interrupts_disable();
    if (queue_size(sem->thread_queue) > 0)
        panic("Attempting to destroy semaphore with waiting threads");
    else {
    }
    interrupts_restore(ipl);
}

/** Get current value of the semaphore.
 *
 * @param sem Semaphore to query.
 * @return Current semaphore value.
 */
int sem_get_value(sem_t* sem) {
    bool ipl = interrupts_disable();
    int ret_val = sem->value;
    interrupts_restore(ipl);
    return ret_val;
}

/** Locks (downs) the semaphore.
 *
 * Decrement the value of this semaphore. If the new value would be negative,
 * block and wait for someone to call sem_post() first.
 * 
 * @param sem Semaphore to be locked.
 */
void sem_wait(sem_t* sem) {
    bool ipl = interrupts_disable();
    if (sem->value == 0) {
        enqueue(sem->thread_queue, get_current_thread());
        thread_suspend();
    } else
        sem->value--;
    interrupts_restore(ipl);
}

/** Unlocks (ups/signals) the sempahore.
 *
 * Increment the value of this semaphore or wake-up one of blocked threads
 * inside sem_wait().
 * 
 * @param sem Semaphore to be unlocked.
 */
void sem_post(sem_t* sem) {
    bool ipl = interrupts_disable();
    if (sem->value > 0 && queue_size(sem->thread_queue) > 0) {
        thread_t* thread_to_wake_up = remove_and_return_first(sem->thread_queue);
        thread_wakeup(thread_to_wake_up);
    } else
        sem->value = sem->value + 1;
    interrupts_restore(ipl);
}

/** Try to lock the semaphore without waiting.
 *
 * If the call to sem_wait() would block, do nothing and return EBUSY.
 *
 * @param sem Semaphore to be locked.
 * @return Error code.
 * @retval EOK Semaphore was successfully locked.
 * @retval EBUSY Semaphore has value of 0 and locking would block.
 */
errno_t sem_trywait(sem_t* sem) {
    bool ipl = interrupts_disable();
    if (sem->value == 0) {
        interrupts_restore(ipl);
        return EBUSY;
    } else {
        sem->value--;
        interrupts_restore(ipl);
        return EOK;
    }
}
