#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#include "date.h"

size_t strftime_compat(char *buf, size_t size, const char *fmt, PSYSTEMTIME st)
{
    size_t i = 0;
    for (const char *s = fmt; *s; ++s) {
        if (i == size)
            return 0;

        if (*s != '%') {
            *(buf++) = *s;
            ++i;
            continue;
        }

        int n;
        switch (*(++s)) {
        case '%': *(buf++) = '%'; ++i; break;
        case 'm': n = _snprintf(buf, size - i, "%02d", st->wMonth); break;
        case 'M': n = _snprintf(buf, size - i, "%02d", st->wMinute); break;
        case 'H': n = _snprintf(buf, size - i, "%02d", st->wHour); break;
        case 'I': n = _snprintf(buf, size - i, "%02d", (st->wHour + 11) % 12 + 1); break;
        case 'k': n = _snprintf(buf, size - i, "%2d", st->wHour); break;
        case 'l': n = _snprintf(buf, size - i, "%2d", (st->wHour + 11) % 12 + 1); break;
        case 'y': n = _snprintf(buf, size - i, "%02d", st->wYear % 100); break;
        case 'Y': n = _snprintf(buf, size - i, "%d", st->wYear); break;
        case 'd': n = _snprintf(buf, size - i, "%02d", st->wDay); break;
        case 'f': n = _snprintf(buf, size - i, "%03d", st->wMilliseconds); break;
        case 'S': n = _snprintf(buf, size - i, "%02d", st->wSecond); break;
        default:
            buf[0] = '%';
            buf[1] = *s;
            n = 2;
            break;
        }
        buf += n;
        i += n;
    }
    *buf = 0;
    return i;
}

int now_fmt(const char *fmt, char *buf, size_t size)
{
    SYSTEMTIME now;
    GetLocalTime(&now);

    return !!strftime_compat(buf, size, fmt, &now);
}
