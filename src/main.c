#include <sys/time.h>

#include "engine/core/app.h"
#include "engine/core/ecs/ecs.h"
#include "engine/jobs/job.h"
#include "engine/layers/game_layer.h"
#include "engine/layers/level_editor_layer.h"

#define SQRT_NUM 524288

void sqrt_dt(void *data) {
    job_parallel_for_args_t *args = data;

    double x = sqrt(args->i);
    printf("%f\n", x);
}

i32 main(i32 argc, char *argv[]) {
    app_t app = init_app("Doomlike", argc, argv);
    set_layer(&app, &level_editor_layer);
    i32 rc = run_app(&app);
    destroy_app(&app);
    return rc;

    /*initialize_thread_pool(18);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    for (double i = 0;i<SQRT_NUM;++i) {
        double x = sqrt(i);
        printf("%f\n", x);
    }
    job_parallel_for_t job = job_for_create(sqrt_dt, nullptr, SQRT_NUM, 4096);

    job_for_execute(&job);
    job_for_wait(&job);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    double delta = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    delta /= 1000;
    printf("Took %f ms\n", delta);

    job_for_destroy(&job);

    destroy_thread_pool();*/

    return 0;
}
