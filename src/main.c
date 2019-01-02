#include <signal.h>
#include "engine.h"

static engine_t engine;

int main()
{
    int res;
    if(engine_init(&engine) != 0)
    {
        error("main", "an error occured, exiting...");
        exit(1);
    }
    res =  engine_run(&engine);

    engine_deinit(&engine);

    exit(res);
}
