#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "print.h"
#include "panic.h"

int hprintf(HANDLE h, const char *fmt, ...)
{
    /* WriteFile is *not* atomic after a 512 byte boundary, we need to lock */
    static CRITICAL_SECTION sync;
    static int init = 0;

    /* We use the heap lock to assure mutual exclusion during initialization */
    if (!init) {
        HANDLE heap = GetProcessHeap();
        if (!HeapLock(heap))
            panic("Could not lock the global heap lock");

        if (!init) {
            InitializeCriticalSection(&sync);
            init = 1;
        }

        if (!HeapUnlock(heap))
            panic("Could not unlock the global heap lock");
    }

    int sz;
    char *buf = NULL;
    va_list vl;
    va_list vlc;

    va_start(vl, fmt);
    va_copy(vlc, vl);
    sz = _vscprintf(fmt, vlc);
    va_end(vlc);

    int rc = -1;
    if (sz <= 0)
        goto end;
    ++sz;

    buf = (char*)malloc(sz);
    if (!buf)
        goto end;

    rc = vsnprintf(buf, sz, fmt, vl);
    if (rc <= 0)
        goto end;

    DWORD written;

    EnterCriticalSection(&sync);
    if (!WriteFile(h, buf, rc, &written, NULL)) {
        rc = -1;
        errno = EIO;
    } else {
        rc = written;
    }
    LeaveCriticalSection(&sync);
    
end:
    free(buf);
    va_end(vl);
    return rc;
}


