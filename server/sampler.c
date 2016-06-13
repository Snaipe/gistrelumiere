#include "sampler.h"
#include "print.h"
#include "date.h"

#define JSON_LOG_FMT                    \
    "{"                                 \
        "\"data\": {"                   \
            "\"counter\": \"%lu\", "    \
            "\"date\": \"%s\", "        \
            "\"light\": \"%d\""         \
        "}"                             \
    "}\r\n"

#define JSON_FILE_FMT                   \
    "{"                                 \
        "\"data\": {"                   \
            "\"counter\": \"%lu\", "    \
            "\"light\": \"%d\", "       \
            "\"status\": \"%s\", "      \
            "\"date\": \"%s\""          \
        "}"                             \
    "}\r\n"                             \

struct sampler_ctx {
    int sampling;
    HANDLE stop;
    HANDLE logfile;
    CRITICAL_SECTION sync;
    CONDITION_VARIABLE condvar;
};

static int get_brightness(void) {
    return 100;
}

static void log_brightness(struct sampler_ctx *ctx, int brightness)
{
    static volatile LONG counter = 0;

    LONG c = InterlockedIncrement(&counter);
    char date[256];
    if (!now_fmt("%d/%m/%Y %H:%M:%S:%f", date, sizeof (date)))
        return;

    printf(JSON_LOG_FMT, c, date, brightness);
    hprintf(ctx->logfile, JSON_FILE_FMT, c, date, brightness);
}

static DWORD loop(struct sampler_ctx *ctx)
{
    EnterCriticalSection(&ctx->sync);
    for (;;) {
        if (WaitForSingleObject(ctx->stop, 0) == WAIT_OBJECT_0)
            break;

        int brightness = get_brightness();
        log_brightness(ctx, brightness);

        SleepConditionVariableCS(&ctx->condvar, &ctx->sync, ctx->sampling);
    }
    LeaveCriticalSection(&ctx->sync);

    free(ctx);
    return 0;
}

HANDLE start_sampling(HANDLE logfile, int sampling, sampler_t *sampler)
{
    struct sampler_ctx *ctx = malloc(sizeof (*ctx));
    if (!ctx)
        goto err;

    ctx->sampling = sampling;
    ctx->stop = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ctx->stop)
        goto err;

    InitializeConditionVariable(&ctx->condvar);
    InitializeCriticalSection(&ctx->sync);

    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) loop,
                                 ctx, 0, NULL);

    if (!thread)
        goto err;
    *sampler = ctx;
    return thread;
err:
    if (ctx) {
        if (ctx->stop)
            CloseHandle(ctx->stop);
        free(ctx);
    }
    return NULL;
}

void shutdown_sampling(sampler_t sampler)
{
    SetEvent(sampler->stop);
}
