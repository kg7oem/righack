/*
 * runloop.c
 *
 *  Created on: May 24, 2018
 *      Author: tyler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>

#include "external/autodie.h"
#include "runloop.h"
#include "types.h"
#include "util.h"

struct run_once_private {
    bool should_run;
    runloop_stateful_cb cb;
    void *context;
};

struct run_once_list {
    struct run_once_private *run;
    struct run_once_list *next;
};

// each thread has their own event loop...
// this should explode some heads
static TLOCAL uv_loop_t *thread_loop = NULL;

// if execution is happening with control given
// to the runloop then this is set true
static TLOCAL bool in_runloop_context = false;

static TLOCAL struct run_once_list *run_once_queue = NULL;
static TLOCAL struct run_once_list *run_once_last = NULL;
static TLOCAL uv_prepare_t *run_once_prepare = NULL;
static TLOCAL bool run_once_prepare_started = false;

void
runloop_bootstrap(void) {
    log_debug("bootstrapping the runloop");
    thread_loop = ad_malloc(sizeof(uv_loop_t));
    uv_loop_init(thread_loop);

    run_once_prepare = ad_malloc(sizeof(uv_prepare_t));
    uv_prepare_init(thread_loop, run_once_prepare);
}

static void
cleanup_handle_cb(uv_handle_t *handle) {
    free(handle);

    log_trace("freed a handle");

    if (handle == (struct uv_handle_s *)run_once_prepare) {
        log_trace("setting run_once_prepare to null");
        run_once_prepare = NULL;
    }
}

static void
walk_runloop_cb(uv_handle_t *handle, UNUSED void *context) {
    if (! uv_is_closing(handle)) {
        log_trace("closing handle in runloop walker");
        uv_close(handle, cleanup_handle_cb);
    }
}

void
runloop_cleanup(void) {
    if (thread_loop == NULL) {
        return;
    }

    log_debug("starting to cleanup the runloop");

    // close any filehandles that are not already closing
    log_lots("walking the runloop to close needed handles");
    uv_walk(thread_loop, walk_runloop_cb, NULL);

    // start the runloop back up so it can process all the queued
    // cleanup callbacks
    log_debug("starting the runloop again to process the cleanup call backs");
    uv_run(thread_loop, UV_RUN_DEFAULT);
    log_debug("out of the runloop now");

    if (uv_loop_close(thread_loop) == UV_EBUSY) {
        util_fatal("could not cleanup runloop because it was not empty");
    }

    free(thread_loop);
    thread_loop = NULL;

    log_lots("runloop is completely cleaned up");
}

bool
runloop_run(void) {
    if (thread_loop == NULL) {
        util_fatal("runloop_run() called but there was no per thread runloop defined");
    }

    log_debug("about to give control to runloop");
    in_runloop_context = true;
    bool retval = uv_run(thread_loop, UV_RUN_DEFAULT);
    in_runloop_context = false;
    log_debug("control from runloop returned; retval = %i", retval);

    return retval;
}

bool
runloop_has_control(void) {
    return in_runloop_context;
}

void
run_once_prepare_cb(uv_prepare_t *prepare) {
    log_trace("Inside the run once prepare handler");
    struct run_once_list *p = run_once_queue;

    while(p != NULL) {
        p->run->cb(p->run->should_run, p->run->context);

        struct run_once_list *old = p;
        p = p->next;

        log_trace("running a run_once job");

        free(old->run);
        free(old);
    }

    run_once_queue = NULL;

    uv_prepare_stop(prepare);
    run_once_prepare_started = false;
}

struct run_once
runloop_run_once(runloop_stateful_cb cb, void *context) {
    struct run_once_private *new_job = ad_malloc(sizeof(struct run_once_private));
    struct run_once_list *queue_entry = ad_malloc(sizeof(struct run_once_list));
    struct run_once retval = {
            .cb = cb,
            .context = context,
            .private = new_job,
    };

    log_trace("scheduled job to run at the next runloop iteration");

    new_job->cb = retval.cb;
    new_job->context = retval.context;
    new_job->should_run = true;

    queue_entry->run = new_job;
    queue_entry->next = NULL;

    if (run_once_queue == NULL) {
        run_once_queue = queue_entry;
        run_once_last = queue_entry;
    } else {
        run_once_last->next = queue_entry;
        run_once_last = queue_entry;
    }

    if (! run_once_prepare_started) {
        uv_prepare_start(run_once_prepare, run_once_prepare_cb);
    }

    return retval;
}
