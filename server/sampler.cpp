#include "sampler.h"
#include "print.h"
#include "date.h"
#include "lock_log.h"
#include "arduino.h"

#define JSON_LOG_FMT                    \
    L"{"                                \
        L"\"data\": {"                  \
            L"\"counter\": \"%lu\", "   \
            L"\"date\": \"%S\", "       \
            L"\"light\": \"%d\""        \
        L"}"                            \
    L"}\r\n"

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
    int threshold;
    int prev_brightness;
    HANDLE stop;
    HANDLE logfile;
    CRITICAL_SECTION sync;
    CONDITION_VARIABLE condvar;
};

static int get_brightness(void) {
    return analogRead(A0);
}

static void log_brightness(struct sampler_ctx *ctx, int brightness)
{
    static volatile LONG counter = 0;

    LONG c = InterlockedIncrement(&counter);
    char date[256];
    if (!now_fmt("%d/%m/%Y %H:%M:%S.%f", date, sizeof (date)))
        return;
    
    Log(JSON_LOG_FMT, c, date, brightness);

    if (!(ctx->prev_brightness < ctx->threshold ^ brightness >= ctx->threshold)) {
        c = InterlockedIncrement(&counter);
        lock_log();
        hprintf(ctx->logfile, JSON_FILE_FMT, c, brightness, brightness >= ctx->threshold ? "ON" : "OFF", date);
        unlock_log();
    }
    ctx->prev_brightness = brightness;
}

static DWORD __cdecl loop(LPVOID param)
{
    struct sampler_ctx *ctx = (struct sampler_ctx*)param;

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

HANDLE start_sampling(HANDLE logfile, int sampling, int threshold, sampler_t *sampler)
{
    struct sampler_ctx *ctx = (struct sampler_ctx*)malloc(sizeof (*ctx));
    if (!ctx)
        goto err;

    ctx->sampling = sampling;
    ctx->stop = CreateEvent(NULL, FALSE, FALSE, NULL);
    ctx->logfile = logfile;
    ctx->prev_brightness = get_brightness();
    ctx->threshold = threshold;
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
