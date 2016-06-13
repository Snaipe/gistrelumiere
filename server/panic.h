#ifndef PANIC_H_
# define PANIC_H_

# ifndef noreturn
#  define noreturn __declspec(noreturn)
# endif

void noreturn panic(const char *fmt, ...);

#endif /* !PANIC_H_ */
