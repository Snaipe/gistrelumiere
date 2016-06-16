#ifndef HTTPSRV_H_
# define HTTPSRV_H_

# include "precomp.h"

typedef int (*route_handle)(HANDLE srv, PHTTP_REQUEST);

struct route {
    const wchar_t *path;
    route_handle hndl;
};

HANDLE start_server(HANDLE logfile, const struct route *routes, PHANDLE srv);
void shutdown_server(HANDLE srv);
void respond(HANDLE queue, PHTTP_REQUEST req, int code,
        const char *reason, const char *entity);
void respond_file(HANDLE queue, PHTTP_REQUEST req, int code,
	const char *reason, HANDLE file);

#endif /* !HTTPSRV_H_ */
