#include "precomp.h"
#include "panic.h"

static CRITICAL_SECTION sync;
static int init;

void lock_log(void)
{
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

    EnterCriticalSection(&sync);
}

void unlock_log(void)
{
    LeaveCriticalSection(&sync);
}