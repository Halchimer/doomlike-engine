#pragma once

#include <stdatomic.h>

#include "thread_pool.h"

typedef struct job_mono_s {
    job_t job;
    atomic_int done;
} job_mono_t;

struct job_parallel_for_iter_data_s;

typedef struct job_parallel_for_s {
    job_t job;
    size_t chunk_size;
    size_t count;

    atomic_int counter;

    struct job_parallel_for_iter_data_s *iter_data;
} job_parallel_for_t;

typedef struct job_parallel_for_args_s {
    void *arg;
    size_t i;
} job_parallel_for_args_t;

job_parallel_for_t job_for_create(job_func_t func, void *arg, size_t count, size_t chunk_size);
void job_for_execute(job_parallel_for_t *job);
void job_for_destroy(job_parallel_for_t *job);
void job_for_wait(job_parallel_for_t *job);