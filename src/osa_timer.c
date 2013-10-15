/** ============================================================================
 *
 *  osa_timer.c
 *
 *  Author     : xkf
 *
 *  Date       : Sep 03, 2013
 *
 *  Description: 
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
#include "osa_timer.h"
#include "osa_mutex.h"
#include "osa_task.h"
#include "osa_task_mgr.h"
#include "dlist.h"
#include "debug.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *  --------------------- Macro definition -------------------------------------
 */

/** ============================================================================
 *  @Macro:         Macro name
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */
#define MILLONS                 (1000000)

#define OSA_TIMER_EVENT_NUM_MAX (32)

#define OSA_TIMER_MIN_SHRESHOLD (16000)

#define OSA_TIMER_CRITICAL_ENTER()          \
    do {                                    \
        mutex_lock(&glb_osa_timer_mutex);   \
    } while (0)

#define OSA_TIMER_CRITICAL_LEAVE()          \
    do {                                    \
        mutex_unlock(&glb_osa_timer_mutex); \
    } while (0)

/*
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */
struct __osa_event_object_t;
typedef struct __osa_event_object_t osa_event_object_t;
struct __osa_event_object_t
{
    unsigned int    m_reserved[2];

    int             m_id;

    osa_event_t     m_event;
    unsigned int    m_delta_delay;
    unsigned int    m_saved_delay;
};

struct __osa_timer_object_t;
typedef struct __osa_timer_object_t osa_timer_object_t;
struct __osa_timer_object_t
{
    osa_event_object_t
                    m_event_objs[OSA_TIMER_EVENT_NUM_MAX];
    dlist_t         m_free_list;
    dlist_t         m_busy_list;

    unsigned int    m_resync_done;
    unsigned int    m_handle_done;

    unsigned int    m_elapsed_time;

    int             m_timer_id;
    osa_event_t     m_timer_event;

    struct timeval  m_sync_time;
    struct timeval  m_temp_time;

    pthread_cond_t  m_cond;
    pthread_mutex_t m_mutex;
};

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static status_t 
__osa_timer_task_external_main(void *ud, task_t tsk, msg_t **msg);

static osa_timer_object_t glb_osa_timer_obj;

static task_object_t glb_osa_timer_tsk_obj = 
{
    .m_name       = "OSA_TIMER_TSK",
    .m_main       = __osa_timer_task_external_main,
    .m_pri        = 0,
    .m_stack_size = 0,
    .m_init_state = 0,
    .m_userdata   = &glb_osa_timer_obj,
    .m_task       = TASK_INVALID_TSK
};

OSA_DECLARE_AND_INIT_MUTEX(glb_osa_timer_mutex);

static unsigned int glb_cur_init = 0;
/*
 *  --------------------- Local function forward declaration -------------------
 */
static status_t __osa_timer_do_initialize  (osa_timer_object_t *ptimer);

static status_t __osa_timer_do_deinitialize(osa_timer_object_t *ptimer);

static status_t __osa_timer_do_process(osa_timer_object_t *ptimer, task_t tsk, msg_t **msg);

static status_t
__osa_timer_init(osa_timer_object_t *ptimer);

static status_t
__osa_timer_event_alloc(osa_timer_object_t *ptimer, osa_event_object_t **pevent);

static status_t
__osa_timer_event_free(osa_timer_object_t *ptimer, osa_event_object_t *pevent);

static status_t
__osa_timer_event_handler(void *ud);

static status_t
__osa_timer_add_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent);

static status_t
__osa_timer_del_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent);

static status_t
__osa_timer_update_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent, unsigned int new_delay);

static void
__osa_timer_nanosleep(unsigned int micro_secs);

static status_t
__osa_timer_deinit(osa_timer_object_t *ptimer);

static bool
__osa_timer_event_find_match_fxn(dlist_element_t *elem, void *data)
{
    return ((((osa_event_object_t *)elem)->m_id) == (((unsigned int)data) - 1));
}

/*
 *  --------------------- Public function definition ---------------------------
 */
status_t osa_timer_init(void)
{
    status_t status = OSA_SOK;
    osa_timer_object_t *ptimer = &glb_osa_timer_obj;

    OSA_TIMER_CRITICAL_ENTER();

    if (glb_cur_init++ == 0) {
        status = __osa_timer_init(ptimer);
    }

    OSA_TIMER_CRITICAL_LEAVE();

    return status;
}

status_t osa_timer_register(int *pid, unsigned int delay, osa_event_t *pevent)
{
    status_t status = OSA_SOK;
    osa_event_object_t *pevt_obj = NULL;
    osa_timer_object_t *ptimer   = &glb_osa_timer_obj;

    if (pid == NULL || pevent == NULL) {
        return OSA_EARGS;
    }

    (*pid) = -1;

	pthread_mutex_unlock(&ptimer->m_mutex);

    status = __osa_timer_event_alloc(ptimer, &pevt_obj);

    if (OSA_ISERROR(status)) {
        pthread_mutex_unlock(&ptimer->m_mutex);
        return status;
    }

    /* Initialize event object */
    pevt_obj->m_event = *pevent;
    pevt_obj->m_saved_delay = pevt_obj->m_delta_delay = delay * 1000;

    status = __osa_timer_add_event(ptimer, pevt_obj);

    if (OSA_ISERROR(status)) {
        __osa_timer_event_free(ptimer, pevt_obj);
    } else {
        pthread_cond_signal(&ptimer->m_cond);
        (*pid) = pevt_obj->m_id;
    }

	pthread_mutex_unlock(&ptimer->m_mutex);

    return status;
}

status_t osa_timer_update_event(int id, unsigned int new_delay)
{
	status_t status = OSA_SOK;
	osa_event_object_t *pevent = NULL;
	osa_timer_object_t *ptimer = &glb_osa_timer_obj;
	
	pthread_mutex_lock(&ptimer->m_mutex);
	
	status = dlist_search_element(&ptimer->m_busy_list, (void *)(id + 1),
								  (dlist_element_t **)&pevent, __osa_timer_event_find_match_fxn);
	
	if (!OSA_ISERROR(status) && pevent != NULL) {
		status |= __osa_timer_update_event(ptimer, pevent, new_delay);
	}

	pthread_mutex_unlock(&ptimer->m_mutex);
	
	return status;
}

status_t osa_timer_unregister(int id)
{
    status_t status = OSA_SOK;
    osa_event_object_t *pevent = NULL;
    osa_timer_object_t *ptimer = &glb_osa_timer_obj;

	pthread_mutex_lock(&ptimer->m_mutex);

    status = dlist_search_element(&ptimer->m_busy_list, (void *)(id + 1),
								  (dlist_element_t **)&pevent, __osa_timer_event_find_match_fxn);
	
	if (!OSA_ISERROR(status) && pevent != NULL) {
		status |= __osa_timer_del_event(ptimer, pevent);
	    status |= dlist_put_tail(&ptimer->m_free_list, (dlist_element_t *)pevent);
	}

	pthread_mutex_unlock(&ptimer->m_mutex);

    return status;
}

status_t osa_timer_deinit(void)
{
    status_t status = OSA_SOK;
    osa_timer_object_t *ptimer = &glb_osa_timer_obj;

    OSA_TIMER_CRITICAL_ENTER();

    if (--glb_cur_init == 0) {
        status = __osa_timer_deinit(ptimer);
    }

    OSA_TIMER_CRITICAL_LEAVE();

    return status;
}

/*
 *  --------------------- Local function definition ----------------------------
 */
static status_t
__osa_timer_init(osa_timer_object_t *ptimer)
{
    status_t status = OSA_SOK;
    task_object_t *ptsk = &glb_osa_timer_tsk_obj;

    /* Register osa timer task */
    status = task_mgr_register(ptsk);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /* Start osa timer task */
    status = task_mgr_start(ptsk);
    if (OSA_ISERROR(status)) {
        status |= task_mgr_unregister(ptsk);
        return status;
    }

    gettimeofday(&ptimer->m_sync_time, NULL);

    /* Register osa timer event */
    ptimer->m_timer_event.m_fxn    = __osa_timer_event_handler;
    ptimer->m_timer_event.m_ud     = (void *)ptimer;
    ptimer->m_timer_event.m_delete = FALSE;

    status = osa_timer_register(&ptimer->m_timer_id, 100/* ms */, &ptimer->m_timer_event);

    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t
__osa_timer_event_alloc(osa_timer_object_t *ptimer, osa_event_object_t **pevent)
{
    status_t status = OSA_ENOENT;

    if (!dlist_is_empty(&ptimer->m_free_list)) {
        status = dlist_get_head(&ptimer->m_free_list, (dlist_element_t **)pevent);

        OSA_assert(OSA_SOK == status);
    }

    return status;
}

static status_t
__osa_timer_event_free(osa_timer_object_t *ptimer, osa_event_object_t *pevent)
{
    status_t status = OSA_SOK;

    status |= dlist_put_tail(&ptimer->m_free_list, (dlist_element_t *)pevent);

    OSA_assert(OSA_SOK == status);

    return status;
}

static status_t
__osa_timer_deinit(osa_timer_object_t *ptimer)
{
    status_t status = OSA_SOK;
    task_object_t *ptsk = &glb_osa_timer_tsk_obj;

    /* Stop osa timer task */
    status |= task_mgr_stop(ptsk);

    /* Unregister osa timer task */
    status |= task_mgr_unregister(ptsk);

    return status;
}

static status_t 
__osa_timer_task_external_main(void *ud, task_t tsk, msg_t **msg)
{
    status_t status = OSA_SOK;
    
    osa_timer_object_t * ptimer = NULL;

    ptimer = (osa_timer_object_t *)ud;

    switch (msg_get_cmd((*msg)))
    {
        case TASK_CMD_INIT:
            status |= task_set_state(tsk, TASK_STATE_INIT);
            status |= __osa_timer_do_initialize(ptimer);

            break;

        case TASK_CMD_EXIT:
            status |= task_set_state(tsk, TASK_STATE_EXIT);
            status |= __osa_timer_do_deinitialize(ptimer);

            break;

        case TASK_CMD_PROC:
            status |= task_set_state(tsk, TASK_STATE_PROC);
            status |= __osa_timer_do_process(ptimer, tsk, msg);

            break;

        default:
            break;
    }

    return status;
}

static status_t __osa_timer_do_initialize  (osa_timer_object_t *ptimer)
{
    int i;
    status_t status = OSA_SOK;

    memset(ptimer, 0, sizeof(*ptimer));

    status |= dlist_init(&ptimer->m_free_list);
    status |= dlist_init(&ptimer->m_busy_list);

    pthread_cond_init(&ptimer->m_cond, NULL);

    pthread_mutex_init(&ptimer->m_mutex, NULL);

    OSA_assert(OSA_SOK == status);

    for (i = 0; i < OSA_ARRAYSIZE(ptimer->m_event_objs); i++) {
        status |= dlist_initialize_element((dlist_element_t *)&ptimer->m_event_objs[i]);
        ptimer->m_event_objs[i].m_id = i;
        status |= dlist_put_tail(&ptimer->m_free_list, (dlist_element_t *)&ptimer->m_event_objs[i]);

        OSA_assert(OSA_SOK == status);
    }

    ptimer->m_elapsed_time = 0;

    /* Set two flags used to synchronize and handle events */
    ptimer->m_resync_done = FALSE;
    ptimer->m_handle_done = FALSE;

    return status;
}

static status_t __osa_timer_do_deinitialize(osa_timer_object_t *ptimer)
{
    status_t status = OSA_SOK;

    /* TODO: Handle the osa timer event */

    pthread_mutex_destroy(&ptimer->m_mutex);

    pthread_cond_destroy(&ptimer->m_cond);

    return status;
}

static int
__osa_timer_event_resync_apply_fxn(dlist_element_t * elem, void * data)
{
    osa_event_object_t *pevent = NULL;
    osa_timer_object_t *ptimer = NULL;

    if (elem == NULL || data == NULL) {
        return 0;
    }

    pevent = (osa_event_object_t *)elem;
    ptimer = (osa_timer_object_t *)data;

#if 0
    if (ptimer->m_elapsed_time >= pevent->m_delta_delay) {
        pevent->m_delta_delay   = 0;
        ptimer->m_elapsed_time -= pevent->m_delta_delay;
    } else {
        if (!ptimer->m_resync_done) {
            pevent->m_delta_delay -= ptimer->m_elapsed_time;
            ptimer->m_resync_done  = TRUE;
        }
    }
#endif

    if (!ptimer->m_resync_done) {
        if (ptimer->m_elapsed_time >= pevent->m_delta_delay) {
            pevent->m_delta_delay   = 0;
            ptimer->m_elapsed_time -= pevent->m_delta_delay;
        } else {
            pevent->m_delta_delay -= ptimer->m_elapsed_time;
            ptimer->m_resync_done  = TRUE;
        }
    }

    return 0;
}

static int
__osa_timer_event_handle_apply_fxn(dlist_element_t * elem, void * data)
{
    osa_event_object_t *pevent = NULL;
    osa_timer_object_t *ptimer = NULL;

    if (elem == NULL || data == NULL) {
        return 0;
    }

    pevent = (osa_event_object_t *)elem;
    ptimer = (osa_timer_object_t *)data;
    
#if 0
    if ((!ptimer->m_handle_done) && pevent->m_delta_delay == 0) {
        (*pevent->m_event.m_fxn)(pevent->m_event.m_ud);
    } else {
        ptimer->m_handle_done = TRUE;
    }
#endif

    if (!ptimer->m_handle_done) {
        if (pevent->m_delta_delay == 0) {
            (*pevent->m_event.m_fxn)(pevent->m_event.m_ud);
        } else {
            ptimer->m_handle_done = TRUE;
        }
    }

    return 0;
}

static int
__osa_timer_event_print_apply_fxn(dlist_element_t * elem, void * data)
{
    osa_event_object_t *pevent = NULL;

    if (elem == NULL) {
        return 0;
    }

    pevent = (osa_event_object_t *)elem;

    fprintf(stderr, "Event[0x%x], delta_delay=%d.\n", pevent, pevent->m_delta_delay);

    return 0;
}

static status_t __osa_timer_do_process(osa_timer_object_t *ptimer, task_t tsk, msg_t **msg)
{
    int i;
    unsigned int micro_time_sleep = 0;
    struct timeval now_time;
    osa_event_object_t *pevent = NULL;
    osa_event_object_t *pnext_node = NULL;
    status_t status = OSA_SOK;
    task_state_t tsk_state = TASK_STATE_PROC;

    /*
     *  Note: if we do not return from this routine right now, ack the msg first.
     */
    msg_set_status(*msg, OSA_SOK);
    status = task_ack_free_msg(tsk, *msg);

    (*msg) = NULL;

    while (!OSA_ISERROR(task_get_state(tsk, &tsk_state))
            && tsk_state != TASK_STATE_EXIT) {

        pthread_mutex_lock(&ptimer->m_mutex);

        if (dlist_is_empty(&ptimer->m_busy_list)) {
            pthread_cond_wait(&ptimer->m_cond, &ptimer->m_mutex);
        }
        status = dlist_first(&ptimer->m_busy_list, (dlist_element_t **)&pevent);

        OSA_assert(OSA_SOK == status && pevent != NULL);

        pthread_mutex_unlock(&ptimer->m_mutex);

        if (pevent->m_delta_delay) {

            if (pevent->m_delta_delay > OSA_TIMER_MIN_SHRESHOLD) {
                micro_time_sleep = OSA_TIMER_MIN_SHRESHOLD;
            } else {
                micro_time_sleep = pevent->m_delta_delay;
            }

            //usleep(micro_time_sleep);
            __osa_timer_nanosleep(micro_time_sleep);
        }

        /* Re-synchronize registered events */
        gettimeofday(&now_time, NULL);
        timersub(&now_time, &ptimer->m_sync_time, &ptimer->m_temp_time);
        if (ptimer->m_temp_time.tv_usec < 0) {
            ptimer->m_temp_time.tv_usec += MILLONS;
            ptimer->m_temp_time.tv_sec  -= 1;
        }
        if (ptimer->m_temp_time.tv_sec < 0) {
            ptimer->m_temp_time.tv_usec  = 0;
            ptimer->m_temp_time.tv_sec   = 0;
        }

        /* Update synchronized system time for osa timer */
        ptimer->m_sync_time = now_time;

        ptimer->m_elapsed_time = ptimer->m_temp_time.tv_sec * 1000 + ptimer->m_temp_time.tv_usec;

        pthread_mutex_lock(&ptimer->m_mutex);

        ptimer->m_resync_done = FALSE;
        status = dlist_map(&ptimer->m_busy_list, __osa_timer_event_resync_apply_fxn, (void *)ptimer);

        ptimer->m_handle_done = FALSE;
        status = dlist_map(&ptimer->m_busy_list, __osa_timer_event_handle_apply_fxn, (void *)ptimer);

        status = dlist_first(&ptimer->m_busy_list, (dlist_element_t **)&pevent);

		while (!OSA_ISERROR(status) && pevent != NULL && pevent->m_delta_delay == 0) {

            status = dlist_next(&ptimer->m_busy_list, (dlist_element_t *)pevent, (dlist_element_t **)&pnext_node);

			if (!pevent->m_event.m_delete) {
				status |= __osa_timer_update_event(ptimer, pevent, pevent->m_saved_delay);
            } else {
                status |= __osa_timer_del_event(ptimer, pevent);
                status |= dlist_put_tail(&ptimer->m_free_list, (dlist_element_t *)pevent);
            }
			
            pevent = pnext_node;
        }

        pthread_mutex_unlock(&ptimer->m_mutex);
    }

    return status;
}

static status_t 
__osa_timer_synchronize(void *ud, task_t tsk, msg_t *msg)
{
    status_t status = OSA_SOK;
    osa_timer_object_t *ptimer = NULL;

    ptimer = (osa_timer_object_t *)ud;

    if (ptimer == NULL) {
        return OSA_EARGS;
    }

    switch (msg_get_cmd(msg))
    {
        case TASK_CMD_EXIT:
            status |= __osa_timer_do_deinitialize(ptimer);
            status |= task_set_state(tsk, TASK_CMD_EXIT);
            break;

        default:
            break;
    }

    return status;
}

static status_t
__osa_timer_event_handler(void *ud)
{
    return task_synchronize(ud, glb_osa_timer_tsk_obj.m_task, __osa_timer_synchronize, 0);
}

static bool
__osa_timer_event_comp_match_fxn(dlist_element_t *elem, void *data)
{
    return ((((osa_event_object_t *)elem)->m_id) >= (((unsigned int)data) - 1));
}

static bool
__osa_timer_event_add_comp_match_fxn(dlist_element_t *elem, void *data)
{
    osa_event_object_t *pcur_event = NULL;
	osa_event_object_t *pnew_event = NULL;
	
	pcur_event = (osa_event_object_t *)elem;
	pnew_event = (osa_event_object_t *)data;

	if (pnew_event->m_delta_delay >= pcur_event->m_delta_delay) {
		pnew_event->m_delta_delay -= pcur_event->m_delta_delay;
		
		return false;
	} else {
		pcur_event->m_delta_delay -= pnew_event->m_delta_delay;

		return true;
	}
}

static status_t
__osa_timer_add_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent)
{
	status_t status = OSA_SOK;
    struct timeval now_time;
	osa_event_object_t *pnext_node = NULL;

    /*
     *  Re-synchronize osa timer event.
     */		
    gettimeofday(&now_time, NULL);
    timersub(&now_time, &ptimer->m_sync_time, &ptimer->m_temp_time);
    if (ptimer->m_temp_time.tv_usec < 0) {
        ptimer->m_temp_time.tv_usec += MILLONS;
        ptimer->m_temp_time.tv_sec  -= 1;
    }
    if (ptimer->m_temp_time.tv_sec < 0) {
        ptimer->m_temp_time.tv_usec  = 0;
        ptimer->m_temp_time.tv_sec   = 0;
    }

    /* Update synchronized system time for osa timer */
    ptimer->m_sync_time = now_time;

    ptimer->m_elapsed_time = ptimer->m_temp_time.tv_sec * 1000 + ptimer->m_temp_time.tv_usec;

    ptimer->m_resync_done = FALSE;
    status = dlist_map(&ptimer->m_busy_list, __osa_timer_event_resync_apply_fxn, (void *)ptimer);

	status = dlist_search_element(&ptimer->m_busy_list, (void *)pevent,
	                              (dlist_element_t **)&pnext_node, __osa_timer_event_add_comp_match_fxn);
	
	if (OSA_ISERROR(status) && pnext_node == NULL) {
        /*
         *  Two cases:
         *  case 1: the event list is empty.
         *
         *  case 2: the new event's delay time is bigger than that of the events'
         *          already registered.
         */
#if 0
		if (dlist_is_empty(&ptimer->m_busy_list)) {
			status = dlist_put_tail(&ptimer->m_busy_list, (dlist_element_t *)pevent);
		} else {
			status = dlist_put_tail(&ptimer->m_busy_list, (dlist_element_t *)pevent);
		}
#else
		status = dlist_put_tail(&ptimer->m_busy_list, (dlist_element_t *)pevent);
#endif

	} else {
		status = dlist_insert_before(&ptimer->m_busy_list, (dlist_element_t *)pevent, (dlist_element_t *)pnext_node);
	}
	
	return status;
}

static status_t
__osa_timer_del_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent)
{
	status_t status = OSA_SOK;
	osa_event_object_t *pnext_node = NULL;
	
	status = dlist_next(&ptimer->m_busy_list, (dlist_element_t *)pevent, (dlist_element_t **)&pnext_node);
	
	if (!OSA_ISERROR(status) && pnext_node != NULL) {
		pnext_node->m_delta_delay += pevent->m_delta_delay;
	}
	
	status = dlist_remove_element(&ptimer->m_busy_list, (dlist_element_t *)pevent);
	
	return status;
}

static status_t
__osa_timer_update_event(osa_timer_object_t *ptimer, osa_event_object_t *pevent, unsigned int new_delay)
{
	status_t status = OSA_SOK;
	
	status |= __osa_timer_del_event(ptimer, pevent);
	
	pevent->m_delta_delay = new_delay;
	pevent->m_saved_delay = new_delay;
	
	status |= __osa_timer_add_event(ptimer, pevent);
	
	return status;
}

static void
__osa_timer_nanosleep(unsigned int micro_secs)
{
    struct timespec delay_time, remain_time;
    int ret;

    delay_time.tv_sec  = micro_secs / 1000000;

    delay_time.tv_nsec = (micro_secs % 1000000) * 1000;

    do {
        ret = nanosleep(&delay_time, &remain_time);
        if(ret < 0 && remain_time.tv_sec > 0 && remain_time.tv_nsec > 0) {
            /* restart for remaining time */
            delay_time = remain_time;
        } else {
            break;
        }
    } while(1);
}

#if defined(__cplusplus)
}
#endif
