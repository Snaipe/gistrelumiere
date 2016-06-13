#include "httpsrv.h"

#define REQUEST_BUFSIZE 4096

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)                      \
    do {                                                                    \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue = (RawValue); \
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength =        \
            (USHORT) strlen(RawValue);                                      \
    } while(0)

void respond(HANDLE queue, PHTTP_REQUEST req, int code,
        const char *reason, const char *entity)
{
    HTTP_RESPONSE resp = {
        .StatusCode = code,
        .pReason = reason,
        .ReasonLength = strlen(reason),
    };

    ADD_KNOWN_HEADER(resp, HttpHeaderContentType, "text/html");

    HTTP_DATA_CHUNK dataChunk;
    if (entity) {
        dataChunk.DataChunkType           = HttpDataChunkFromMemory;
        dataChunk.FromMemory.pBuffer      = (char *) entity;
        dataChunk.FromMemory.BufferLength = strlen(entity);
        resp.EntityChunkCount         = 1;
        resp.pEntityChunks            = &dataChunk;
    }

    DWORD rc = HttpSendHttpResponse(queue, req->RequestId, 0, &resp,
                    NULL, NULL, NULL, 0, NULL, NULL);

    if (rc != NO_ERROR) {
        printf("HttpSendHttpResponse failed with %lu \n", rc);
    }
}

struct server_ctx {
    HANDLE queue;
    const struct route *routes;
    HANDLE logfile;
};

static DWORD loop(struct server_ctx *ctx)
{
    static char rbuf[REQUEST_BUFSIZE];

    HTTP_REQUEST_ID rid;
    HTTP_SET_NULL_ID(&rid);

    for (;; HTTP_SET_NULL_ID(&rid)) {
        PHTTP_REQUEST req = (void *)rbuf;
        RtlZeroMemory(req, REQUEST_BUFSIZE);

        ULONG rc = HttpReceiveHttpRequest(ctx->queue, rid, 0, req,
                                          REQUEST_BUFSIZE, NULL, NULL);
        if (rc == ERROR_MORE_DATA) {
            printf("Http request too big, dropping it\n");
            respond(ctx->queue, req, 500, "Request too big", NULL);
            continue;
        }

        if (rc == ERROR_CONNECTION_INVALID && !HTTP_IS_NULL_ID(&rid)) {
            printf("Http request contains garbage, dropping it\n");
            respond(ctx->queue, req, 500, "Invalid request", NULL);
            continue;
        }

        if (rc != NO_ERROR)
            break;

        if (req->Verb != HttpVerbGET) {
            respond(ctx->queue, req, 503, "Not implemented", NULL);
            continue;
        }

        const struct route *r;
        for (r = ctx->routes; r->path; ++r) {
            size_t len = req->CookedUrl.FullUrlLength
                       - req->CookedUrl.QueryStringLength;

            if (!wcsncmp(r->path, req->CookedUrl.pFullUrl, len))
                break;
        }

        if (r->path && r->hndl(ctx->queue, req))
            break;
    }

    for (const struct route *r = ctx->routes; r->path; ++r)
        HttpRemoveUrl(ctx->queue, r->path);
    CloseHandle(ctx->queue);
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
    free(ctx);

    return 0;
}

HANDLE start_server(HANDLE logfile, const struct route *routes, PHANDLE srv)
{
    HTTPAPI_VERSION ver = HTTPAPI_VERSION_1;
    ULONG rc = HttpInitialize(ver, HTTP_INITIALIZE_SERVER, NULL);
    if (rc != NO_ERROR) {
        printf("HttpInitialize failed with %lu \n", rc);
        return NULL;
    }

    struct server_ctx *ctx = NULL;
    HANDLE queue = NULL;

    rc = HttpCreateHttpHandle(&queue, 0);
    if (rc != NO_ERROR) {
        printf("HttpCreateHttpHandle failed with %lu \n", rc);
        goto err;
    }

    for (const struct route *r = routes; r->path; ++r) {
        rc = HttpAddUrl(queue, r->path, NULL);
        if (rc != NO_ERROR) {
            printf("HttpCreateHttpHandle failed with %lu \n", rc);
            goto err;
        }
    }

    ctx = malloc(sizeof (struct server_ctx));
    if (!ctx)
        goto err;

    ctx->queue = queue;
    ctx->routes = routes;
    ctx->logfile = logfile;

    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) loop,
                                 ctx, 0, NULL);

    if (!thread)
        goto err;
    *srv = queue;
    return thread;

err:
    for (const struct route *r = routes; r->path; ++r)
        HttpRemoveUrl(queue, r->path);
    if (queue)
        CloseHandle(queue);
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
    return NULL;
}

void shutdown_server(HANDLE srv)
{
    HttpShutdownRequestQueue(srv);
}
