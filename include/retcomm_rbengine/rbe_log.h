#ifndef RETCOMM_RBENGINE_LOG_H
#define RETCOMM_RBENGINE_LOG_H

/*
 * Library log sink. Default: stderr + flush (MotK behaviour). Hosts with
 * their own logging (BattleShip port_log, launcher ring files) install a
 * sink; each call delivers one fully formatted message (trailing newline
 * included, as authored at the call site).
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*RbeLogSink)(void *ctx, const char *line);

/* NULL sink restores the stderr default. */
void rbe_set_log_sink(RbeLogSink sink, void *ctx);

void rbe_logf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* RETCOMM_RBENGINE_LOG_H */
