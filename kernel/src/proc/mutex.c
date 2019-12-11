// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <exc.h>
#include <lib/print.h>
#include <proc/mutex.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

/** Initializes given mutex.
 *
 * @param mutex Mutex to initialize.
 * @return Error code.
 * @retval EOK Mutex was successfully initialized.
 */
errno_t mutex_init(mutex_t* mutex) {
    bool ipl = interrupts_disable();
    mutex_t new_mutex = { false, NULL };
    *mutex = new_mutex;
    interrupts_restore(ipl);
    return EOK;
}

/** Destroys given mutex.
 *
 * The function must panic if the mutex is still locked when being destroyed.
 *
 * @param mutex Mutex to destroy.
 */
void mutex_destroy(mutex_t* mutex) {
    bool ipl = interrupts_disable();
    if (mutex->locked)
        panic("Trying to destroy locked mutex");
    else {
    }
    interrupts_restore(ipl);
}

/** Locks the mutex.
 *
 * Note that when this function returns, the mutex must be locked.
 *
 * @param mutex Mutex to be locked.
 */

void mutex_lock(mutex_t* mutex) {
    bool ipl = interrupts_disable();
    while (mutex->locked) {
        thread_yield();
    }

    mutex->locked = true;
    mutex->holder = get_current_thread();
    interrupts_restore(ipl);
}

/** Unlocks the mutex.
 *
 * Note that when this function returns, the mutex might be already locked
 * by a different thread.
 *
 * This function shall panic if the mutex is unlocked by a different thread
 * than the one that locked it.
 *
 * @param mutex Mutex to be unlocked.
 */
void mutex_unlock(mutex_t* mutex) {
    bool ipl = interrupts_disable();
    thread_t* current_thread = get_current_thread();
    if (current_thread != mutex->holder)
        panic("Wrong thread tried to unlock mutex");
    else {
        mutex->holder = NULL;
        mutex->locked = false;
    }
    interrupts_restore(ipl);
}

/** Try to lock the mutex without waiting.
 *
 * If the mutex is already locked, do nothing and return EBUSY.
 *
 * @param mutex Mutex to be locked.
 * @return Error code.
 * @retval EOK Mutex was successfully locked.
 * @retval EBUSY Mutex is currently locked by a different thread.
 */
errno_t mutex_trylock(mutex_t* mutex) {
    bool ipl = interrupts_disable();
    if (mutex->locked) {
        interrupts_restore(ipl);
        return EBUSY;
    }

    mutex->locked = true;
    mutex->holder = get_current_thread();
    return EOK;
    interrupts_restore(ipl);
}
