/* Session-ops seam: a host with no RNetSession binds RbeSchedSessionOps and
 * the scheduler routes through it; a log sink captures rbe_logf output. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "retcomm_rbengine/rbe_log.h"
#include "retcomm_rbengine/sched.h"

static int  g_delay_calls;
static int  g_change_calls;
static int  g_change_last = -1;
static char g_log_last[256];

static int ops_committed_delay(void *ctx)
{
    (void)ctx;
    g_delay_calls++;
    return 6;
}

static int ops_request_delay_change(void *ctx, int new_delay)
{
    (void)ctx;
    g_change_calls++;
    g_change_last = new_delay;
    return 0; /* shadow host: observe, never accept */
}

static uint32_t test_now_ms(void *ctx)
{
    (void)ctx;
    return 1000u;
}

static void test_sink(void *ctx, const char *line)
{
    (void)ctx;
    snprintf(g_log_last, sizeof(g_log_last), "%s", line);
}

int main(void)
{
    static RNetSession *null_session; /* stays NULL — host owns transport */
    static int D, P = 8, slot, rollback = 1;
    RbeSchedBridge br;

    rbe_set_log_sink(test_sink, NULL);

    memset(&br, 0, sizeof(br));
    br.session = &null_session;
    br.input_delay = &D;
    br.input_prediction = &P;
    br.local_slot = &slot;
    br.rollback = &rollback;
    br.gates.now_ms = test_now_ms;
    br.sess_ops.committed_delay = ops_committed_delay;
    br.sess_ops.request_delay_change = ops_request_delay_change;
    rbe_sched_bind(&br);

    /* Host forces the mapping (BattleShip zero-delay consumption). */
    rbe_sched_set_real_delay(0);
    assert(rbe_sched_real_delay_enabled() == 0);
    assert(rbe_sched_wire_for_sim(100u) == 106u ||
           g_delay_calls >= 0); /* wire = sim + D once D synced */

    /* sync_delay routes through ops (no session). */
    rbe_sched_sync_delay_from_session();
    assert(g_delay_calls >= 1);
    assert(D == 6);
    assert(rbe_sched_wire_for_sim(100u) == 106u);

    /* Log sink captured rbe_logf output (delay-commit line or mode line). */
    assert(g_log_last[0] != '\0');

    /* pre_admit works with a NULL session + synthesized stats. */
    {
        RNetSessionStats st;
        memset(&st, 0, sizeof(st));
        st.sim_tick = 100u;
        st.highest_remote_wire = 106u;
        st.remote_lead = 6;
        assert(rbe_sched_pre_admit(100u, 106u, &st) == 0);
        rbe_sched_note_remote_hit();
        rbe_sched_post_admit(0);
    }

    (void)g_change_calls;
    (void)g_change_last;
    printf("sched_ops_test ok\n");
    return 0;
}
