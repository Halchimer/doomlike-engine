#include "job.h"

#include <stdlib.h>

job_mono_t job_mono_create(job_func_t func, void *arg) {

}

void job_mono_destroy(job_mono_t *job) {
}

void job_mono_join(job_mono_t *job) {
}

// job parallel for

struct job_parallel_for_iter_data_s {
    size_t start;
    size_t amnt;

    void *arg;
    job_func_t func;
};

void job_parallel_for_iter_fn(void *arg) {
    struct job_parallel_for_iter_data_s *data = arg;
    for (int i=data->start; i<data->start + data->amnt; ++i) {
        job_parallel_for_args_t args = {data->arg, i};
        data->func(&args);
    }
}

job_parallel_for_t job_for_create(job_func_t func, void *arg, size_t count, size_t chunk_size) {
    job_parallel_for_t job = {
        .job = (job_t){.func = func, .args = arg},
        .chunk_size = chunk_size,
        .count = count,
        .iter_data = nullptr
    };
    atomic_store(&job.counter, (int)ceil((float)count/chunk_size) );
    return job;
}

void job_for_execute(job_parallel_for_t *job) {

    job->iter_data = calloc(job->chunk_size, sizeof(struct job_parallel_for_iter_data_s));
    size_t njobs = ceil(job->count / job->chunk_size);
    printf("Executing job_parallel_for on count %d with %d jobs.", job->count, njobs);
    for (int i=0; i<njobs; ++i) {
        job->iter_data[i] = (struct job_parallel_for_iter_data_s) {
            .start = i * job->chunk_size,
            .amnt = job->chunk_size,
            .func = job->job.func,
            .arg = job->job.args
        };

        push_job((job_t){job_parallel_for_iter_fn, &job->iter_data[i], &job->counter});
    }

}

void job_for_destroy(job_parallel_for_t *job) {
    free(job->iter_data);
    job->iter_data = nullptr;
}

void job_for_wait(job_parallel_for_t *job) {
    while (atomic_load(&job->counter) > 0) {
        try_exec_job();
    }
}
