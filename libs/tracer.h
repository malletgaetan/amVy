#ifndef TRACER_H
# include <stdio.h>
# include <stdarg.h>


void _trace(const char *fmt, ...);
void _trace_display(void);
void _trace_clear(void);

# ifdef DEBUG
#  define trace(fmt, ...) _trace(fmt, __VA_ARGS__)
#  define trace_display() _trace_display()
#  define trace_clear() _trace_clear()
# else
#  define trace(...)
#  define trace_display()
#  define trace_clear()
# endif

#endif
