#include "job.h"

#include <stdlib.h>

job_mono_t job_mono_create(job_func_t func, void *arg) {
    return (job_mono_t){
    .job = (job_t){.func = func, .args = arg},
    };
}

void job_mono_execute(job_mono_t *job) {
    atomic_store(&job->done, 1);

    job->job.counter = &job->done;
    push_job(job->job);
}

void job_mono_destroy(job_mono_t *job) {
}

void job_mono_wait(job_mono_t *job) {
    while (atomic_load(&job->done) > 0) {
        try_exec_job();
    }
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

job_parallel_for_t job_for_create(job_func_t func, void *arg, size_t count, size_t chunks) {
    job_parallel_for_t job = {
        .job = (job_t){.func = func, .args = arg},
        .chunks = chunks,
        .count = count,
        .iter_data = nullptr
    };
    return job;
}

void job_for_execute(job_parallel_for_t *job) {

    if (job->chunks > job->count) {
        fprintf(stderr, "Too many chunks for job_parallel_for.");
        return;
    }

    job->iter_data = calloc(job->chunks, sizeof(struct job_parallel_for_iter_data_s));
    printf("Executing job_parallel_for on count %d with %d jobs.\n", job->count, job->chunks);

    size_t chunk_size = (job->count + job->chunks - 1) / job->chunks;

    atomic_store(&job->counter, job->chunks);

    for (int i=0; i<job->chunks; ++i) {

        size_t start = i * chunk_size;
        size_t end = start + chunk_size;
        end = end>job->count ? job->count : end;
        size_t amnt = end - start;

        if (start >= job->count)
            break;

        job->iter_data[i] = (struct job_parallel_for_iter_data_s) {
            .start = start,
            .amnt = amnt,
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
