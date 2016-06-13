#include "date.h"
#include "httpsrv.h"
#include "sampler.h"

int route_stop(HANDLE srv, PHTTP_REQUEST req)
{
    respond(srv, req, 200, "OK", "Server shut down.\r\n");
    return 1;
}

int route_gistrelumiere(HANDLE srv, PHTTP_REQUEST req)
{
    respond(srv, req, 200, "OK", "Hey! You hit the server!\r\n");
    return 0;
}

int main(int argc, const char *argv[])
{
    static struct route routes[] = {
        { L"http://mygalileo:8080/gistrelumiere/",   route_gistrelumiere },
        { L"http://mygalileo:8080/stop/",            route_stop          },
        { NULL,                                     NULL                }
    };

    int sampling = 100;
    if (argc >= 2)
        sampling = atoi(argv[1]);

    char logpath[256];
    if (!now_fmt("%Y%m%d_%H%M%S.log", logpath, sizeof (logpath)))
        return -1;

    HANDLE logfile = CreateFile(logpath, GENERIC_READ | GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (logfile == INVALID_HANDLE_VALUE)
        return -1;

    HANDLE srv;
    HANDLE thttp = start_server(logfile, routes, &srv);
    if (!thttp)
        return -1;

    sampler_t sampler;
    HANDLE tsampl = start_sampling(logfile, sampling, &sampler);
    if (!tsampl)
        return -1;

    static char buf[128];
    for (;;) {
        scanf("%*s", sizeof (buf), buf);
        if (!strncmp(buf, "stop", sizeof (buf)))
            break;
    }

    shutdown_server(srv);
    shutdown_sampling(sampler);

    HANDLE hndls[] = { thttp, tsampl };
    size_t nhndls = sizeof (hndls) / sizeof (HANDLE);

    WaitForMultipleObjects(nhndls, hndls, TRUE, INFINITE);
    for (size_t i = 0; i < nhndls; ++i)
        CloseHandle(hndls[i]);
    CloseHandle(logpath);
}
