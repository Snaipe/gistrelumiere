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

        switch (*s) {
        case '%': *(buf++) = '%'; ++i; break;
        case 'm': i += snprintf(buf, size - i, "%02d", st->wMonth); break;
        case 'M': i += snprintf(buf, size - i, "%02d", st->wMinute); break;
        case 'H': i += snprintf(buf, size - i, "%02d", st->wHour); break;
        case 'I': i += snprintf(buf, size - i, "%02d", (st->wHour + 11) % 12 + 1); break;
        case 'k': i += snprintf(buf, size - i, "%2d", st->wHour); break;
        case 'l': i += snprintf(buf, size - i, "%2d", (st->wHour + 11) % 12 + 1); break;
        case 'y': i += snprintf(buf, size - i, "%02d", st->wYear % 100); break;
        case 'Y': i += snprintf(buf, size - i, "%d", st->wYear); break;
        case 'd': i += snprintf(buf, size - i, "%02d", st->wDay); break;
        case 'f': i += snprintf(buf, size - i, "%03d", st->wMilliseconds); break;
        case 'S': i += snprintf(buf, size - i, "%02d", st->wSecond); break;
        default:
            *(buf++) = '%';
            *(buf++) = *s;
            i += 2;
            break;
        }
    }
    return i;
}

int now_fmt(const char *fmt, char *buf, size_t size)
{
    SYSTEMTIME now;
    GetLocalTime(&now);

    return !!strftime_compat(buf, size, fmt, &now);
}
