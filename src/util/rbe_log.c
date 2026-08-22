#include "retcomm_rbengine/rbe_log.h"

#include <stdarg.h>
#include <stdio.h>

static RbeLogSink g_sink;
static void      *g_sink_ctx;

void rbe_set_log_sink(RbeLogSink sink, void *ctx)
{
    g_sink = sink;
    g_sink_ctx = ctx;
}

void rbe_logf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (g_sink) {
        char line[1024];
        vsnprintf(line, sizeof(line), fmt, ap);
        g_sink(g_sink_ctx, line);
    } else {
        vfprintf(stderr, fmt, ap);
        fflush(stderr);
    }
    va_end(ap);
}
