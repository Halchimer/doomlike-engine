#pragma once

#include <hclib.h>
#include <pthread.h>
#include <stdatomic.h>

typedef void (*job_func_t)(void *);

typedef struct job_s {
    job_func_t func;
    void *args;

    // counter for detecting when it's done
    atomic_int *counter;
} job_t;

struct job_queue_s {
    h_queue_t jobs;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool stop;
};

struct thread_pool_s {
    pthread_t *workers;
    size_t workers_count;

    struct job_queue_s job_queue;
};
extern struct thread_pool_s *g_thread_pool;

void initialize_thread_pool();
void push_job(job_t job);
void try_exec_job();
void destroy_thread_pool();