#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "date.h"
#include "httpsrv.h"
#include "sampler.h"
#include "panic.h"
#include "lock_log.h"

static volatile int running;
static HANDLE logfile;

int route_stop(HANDLE srv, PHTTP_REQUEST req)
{
    respond(srv, req, 200, "OK", "Server shut down.\r\n");
    running = 0;
    return 1;
}

int route_gistrelumiere(HANDLE srv, PHTTP_REQUEST req)
{
    lock_log();
    respond_file(srv, req, 200, "OK", logfile);
    unlock_log();
    return 0;
}

int main(int argc, const char *argv[])
{
    static struct route routes[] = {
        { L"http://mygalileo:8080/gistrelumiere",   route_gistrelumiere },
        { L"http://mygalileo:8080/stop",            route_stop          },
        { NULL,                                     NULL                }
    };

    int sampling = 100;
    if (argc >= 2)
        sampling = atoi(argv[1]);

    int threshold = 50;
    if (argc >= 3)
        threshold = atoi(argv[2]);

    char logpath[256];
    if (!now_fmt("%Y%m%d_%H%M%S.log", logpath, sizeof (logpath)))
        return -1;

    logfile = CreateFileA(logpath, GENERIC_READ | GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (logfile == INVALID_HANDLE_VALUE)
        panic("Could not open log file.");

    HANDLE srv;
    HANDLE thttp = start_server(logfile, routes, &srv);
    if (!thttp)
        panic("Could not start HTTP server.");

    sampler_t sampler;
    HANDLE tsampl = start_sampling(logfile, sampling, threshold, &sampler);
    if (!tsampl)
        panic("Could not start brightness sampler.");

    HANDLE stdinh = GetStdHandle(STD_INPUT_HANDLE);
    running = 1;

    static char buf[128];
    while (running) {
        if (WaitForSingleObject(stdinh, 0) != WAIT_OBJECT_0)
            continue;

        scanf("%128s", buf);
        if (!strncmp(buf, "quit", sizeof (buf)))
            break;
    }

    shutdown_server(srv);
    shutdown_sampling(sampler);

    HANDLE hndls[] = { thttp, tsampl };
    size_t nhndls = sizeof (hndls) / sizeof (HANDLE);

    WaitForMultipleObjects(nhndls, hndls, TRUE, INFINITE);
    for (size_t i = 0; i < nhndls; ++i)
        CloseHandle(hndls[i]);
    CloseHandle(logfile);
}
