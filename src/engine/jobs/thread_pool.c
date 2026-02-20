#include "thread_pool.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

struct thread_pool_s *g_thread_pool = nullptr;

void try_exec_job() {
    pthread_mutex_lock(&g_thread_pool->job_queue.mutex);

    if (g_thread_pool->job_queue.stop || g_thread_pool->job_queue.jobs.size <= 0) {
        pthread_mutex_unlock(&g_thread_pool->job_queue.mutex);
        return;
    }

    printf("Thread 0x%x taking hold of a job.\n", (unsigned long) pthread_self());

    job_t *job = h_dequeue(&g_thread_pool->job_queue.jobs);
    pthread_mutex_unlock(&g_thread_pool->job_queue.mutex);
    if (job->func)
        job->func(job->args);
    if (job->counter)
        atomic_fetch_sub(job->counter, 1);
}

void *internal_worker_thread_fn(void *args) {
    struct job_queue_s *job_queue = (struct job_queue_s *)args;

    while(true) {
        pthread_mutex_lock(&job_queue->mutex);
        while (job_queue->jobs.size <= 0 && ! job_queue->stop)
            pthread_cond_wait(&job_queue->cond, &job_queue->mutex);
        if (g_thread_pool->job_queue.stop) {
            pthread_mutex_unlock(&job_queue->mutex);
            break;
        }
        pthread_mutex_unlock(&job_queue->mutex);
        try_exec_job();
    }

    return nullptr;
}

void initialize_thread_pool() {

#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int cores = sysinfo.dwNumberOfProcessors;
    long nthreads = fmax(1, cores - 2);
#else
    long nthreads = fmax(1, sysconf(_SC_NPROCESSORS_ONLN) - 2);
#endif


    g_thread_pool = calloc(1,sizeof(struct thread_pool_s));

    g_thread_pool->job_queue.jobs = H_CREATE_QUEUE(job_t, 1024);
    pthread_mutex_init(&g_thread_pool->job_queue.mutex, nullptr);
    pthread_cond_init(&g_thread_pool->job_queue.cond, nullptr);
    g_thread_pool->job_queue.stop = false;

    g_thread_pool->workers_count = nthreads;
    g_thread_pool->workers = calloc(nthreads, sizeof(pthread_t));
    for (size_t i = 0; i < nthreads; ++i)
        pthread_create(&g_thread_pool->workers[i], nullptr, internal_worker_thread_fn, &g_thread_pool->job_queue);

    printf("initialized thread pool with %d workers.\n", nthreads);
}

void push_job(job_t job) {
    pthread_mutex_lock(&g_thread_pool->job_queue.mutex);
    if (g_thread_pool->job_queue.jobs.size >= g_thread_pool->job_queue.jobs.cap) {
        fprintf(stderr, "Pushed too many jobs, job queue is full.");
    }
    else
        H_ENQUEUE(job_t, g_thread_pool->job_queue.jobs, job);
    pthread_cond_signal(&g_thread_pool->job_queue.cond);
    pthread_mutex_unlock(&g_thread_pool->job_queue.mutex);
}

void destroy_thread_pool() {
    pthread_mutex_lock(&g_thread_pool->job_queue.mutex);
    g_thread_pool->job_queue.stop = true;
    pthread_cond_broadcast(&g_thread_pool->job_queue.cond);
    pthread_mutex_unlock(&g_thread_pool->job_queue.mutex);

    for (int i = 0; i < g_thread_pool->workers_count; ++i)
        pthread_join(g_thread_pool->workers[i], nullptr);

    pthread_mutex_destroy(&g_thread_pool->job_queue.mutex);
    pthread_cond_destroy(&g_thread_pool->job_queue.cond);

    h_queue_free(&g_thread_pool->job_queue.jobs);

    free(g_thread_pool->workers);
    free(g_thread_pool);
    printf("Destroyed thread pool.");
}
