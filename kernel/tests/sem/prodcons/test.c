// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

/*
 * Producer/consumer test. We spawn different numbers of producers and
 * consumers (they equal out at the end of the test, though) to test that
 * semaphores work. If all is well, the queue should be empty at the end
 * of the run and all threads should terminate.
 */

#include <ktest.h>
#include <lib/print.h>
#include <lib/queue.h>
#include <proc/sem.h>
#include <proc/thread.h>

#define LOOPS 100
#define BASE_COUNT 15
#define QUEUE_SIZE (BASE_COUNT * 2)
#define TOTAL_THREAD_COUNT (6 * BASE_COUNT)

#define WAIT_LOOPS BASE_COUNT

static sem_t queue_empty;
static sem_t queue_full;

static size_t producer_thread_count = 0;
static thread_t* producer_threads[TOTAL_THREAD_COUNT];

static size_t consumer_thread_count = 0;
static thread_t* consumer_threads[TOTAL_THREAD_COUNT];

static volatile size_t active_threads = 0;
static sem_t active_threads_guard;

static void register_thread(void) {
    sem_wait(&active_threads_guard);
    active_threads++;
    sem_post(&active_threads_guard);
}

static void unregister_thread(void) {
    sem_wait(&active_threads_guard);
    active_threads--;
    sem_post(&active_threads_guard);
}

static void* worker_producer(void* ignored) {
    register_thread();

    for (int i = 0; i < LOOPS; i++) {
        sem_wait(&queue_empty);
        // Here would be lock on the queue
        // itself and adding to it.
        sem_post(&queue_full);
        thread_yield();
    }

    unregister_thread();
    return NULL;
}

static void* worker_consumer(void* ignored) {
    register_thread();

    for (int i = 0; i < LOOPS; i++) {
        sem_wait(&queue_full);
        // Here would be lock on the queue
        // itself and removing from it.
        sem_post(&queue_empty);
        thread_yield();
    }

    unregister_thread();
    return NULL;
}

static void spawn_workers(size_t producers, size_t consumers) {
    printk("Will spawn %u producers and %u consumers ...\n", producers, consumers);
    while (producers > 0) {
        ktest_assert(producer_thread_count < TOTAL_THREAD_COUNT, "too many producers created");
        errno_t err = thread_create(&producer_threads[producer_thread_count], worker_producer, NULL, 0, "producer");
        ktest_assert_errno(err, "thread_create(producer)");
        producers--;
        producer_thread_count++;
    }
    while (consumers > 0) {
        ktest_assert(consumer_thread_count < TOTAL_THREAD_COUNT, "too many consumers created");
        errno_t err = thread_create(&consumer_threads[consumer_thread_count], worker_consumer, NULL, 0, "producer");
        ktest_assert_errno(err, "thread_create(producer)");
        consumers--;
        consumer_thread_count++;
    }
}

static void let_them_work() {
    for (int i = 0; i < WAIT_LOOPS; i++) {
        // if (active_threads == 1) {
        //     printk("Printing scheduler queue:\n");
        //     qnode_t* tmp = queue->front;
        //     while (1) {
        //         if (tmp == NULL)
        //             break;
        //         thread_t* t = tmp->key;
        //         printk("Thread name: %s, status %d\n", t->name, t->status);
        //         tmp = tmp->next;
        //     }
        //     printk("Printing sem full queue:\n");
        //     qnode_t* tmp2 = queue_full.thread_queue->front;
        //     while (1) {
        //         if (tmp2 == NULL)
        //             break;
        //         thread_t* t = tmp2->key;
        //         printk("Thread name: %s, status %d\n", t->name, t->status);
        //         tmp2 = tmp2->next;
        //     }
        //     printk("Printing sem empty queue:\n");
        //     qnode_t* tmp3 = queue_empty.thread_queue->front;
        //     while (1) {
        //         if (tmp3 == NULL)
        //             break;
        //         thread_t* t = tmp3->key;
        //         printk("Thread name: %s, status %d\n", t->name, t->status);
        //         tmp3 = tmp3->next;
        //     }
        //     printk("Done printing...");
        // }
        thread_yield();
    }
}

void kernel_test(void) {
    ktest_start("sem/prodcons");

    errno_t err = sem_init(&active_threads_guard, 1);
    ktest_assert_errno(err, "sem_init(active_threads_guard)");

    err = sem_init(&queue_empty, QUEUE_SIZE);
    ktest_assert_errno(err, "sem_init(queue_empty)");
    err = sem_init(&queue_full, 0);
    ktest_assert_errno(err, "sem_init(queue_full)");

    spawn_workers(BASE_COUNT * 3, BASE_COUNT * 1);
    let_them_work();
    spawn_workers(BASE_COUNT * 2, BASE_COUNT * 2);
    let_them_work();
    spawn_workers(BASE_COUNT * 1, BASE_COUNT * 3);
    let_them_work();

    while (true) {
        sem_wait(&active_threads_guard);
        size_t still_active = active_threads;
        sem_post(&active_threads_guard);

        printk("There are %u active threads ...\n", still_active);
        if (still_active == 0) {
            break;
        }

        let_them_work();
    }

    printk("Broke from while(true)\n");

    for (size_t i = 0; i < producer_thread_count; i++) {
        err = thread_join(producer_threads[i], NULL);
        ktest_assert_errno(err, "thread_join(producer)");
    }

    for (size_t i = 0; i < consumer_thread_count; i++) {
        err = thread_join(consumer_threads[i], NULL);
        ktest_assert_errno(err, "thread_join(producer)");
    }

    ktest_assert(sem_get_value(&queue_full) == 0, "queue still contains items");
    ktest_assert(sem_get_value(&queue_empty) == QUEUE_SIZE, "queue is not empty");

    ktest_passed();
}
