#pragma once

#include <stdatomic.h>

#include "thread_pool.h"

typedef struct job_mono_s {
    job_t job;
    atomic_int done;
} job_mono_t;

job_mono_t job_mono_create(job_func_t func, void *arg);

void job_mono_execute(job_mono_t *job);

void job_mono_destroy(job_mono_t *job);

void job_mono_wait(job_mono_t *job);

// job parallel for

struct job_parallel_for_iter_data_s;

typedef struct job_parallel_for_s {
    job_t job;
    size_t chunks;
    size_t count;

    struct job_parallel_for_iter_data_s *iter_data;

    alignas(64) atomic_int counter;
} job_parallel_for_t;

typedef struct job_parallel_for_args_s {
    void *arg;
    size_t i;
} job_parallel_for_args_t;

job_parallel_for_t job_for_create(job_func_t func, void *arg, size_t count, size_t chunks);
void job_for_execute(job_parallel_for_t *job);
void job_for_destroy(job_parallel_for_t *job);
void job_for_wait(job_parallel_for_t *job);