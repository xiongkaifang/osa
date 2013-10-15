/** ============================================================================
 *
 *  osa_timer.h
 *
 *  Author     : xkf
 *
 *  Date       : Sep 03, 2013
 *
 *  Description: 
 *  ============================================================================
 */

#if !defined (__OSA_TIMER_H)
#define __OSA_TIMER_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_status.h"

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

/*
 *  --------------------- Data type definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field:         Field2 member
 *  ----------------------------------------------------------------------------
 */
typedef status_t (*OSA_EVENT_FXN_T)(void *ud);

struct __osa_event_t;
typedef struct __osa_event_t osa_event_t;
struct __osa_event_t
{
    OSA_EVENT_FXN_T m_fxn;
    void *          m_ud;
    unsigned int    m_delete;
};

/*
 *  --------------------- Public function declaration --------------------------
 */
status_t osa_timer_init(void);

status_t osa_timer_register(int *pid, unsigned int delay, osa_event_t *pevent);

status_t osa_timer_unregister(int id);

status_t osa_timer_deinit(void);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_TIMER_H) */
