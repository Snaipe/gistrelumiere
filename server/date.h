#ifndef DATE_H_
# define DATE_H_

# include <stddef.h>
# include "precomp.h"

int now_fmt(const char *fmt, char *buf, size_t size);
size_t strftime_compat(char *buf, size_t size, const char *fmt, PSYSTEMTIME st);

#endif /* !DATE_H_ */
