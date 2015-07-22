/*
 *  Copyright 2009 by Giaval Science Development Co. Ltd.                        ,_
 *                                                                              >' )
 *  All rights reserved. Property of Giaval Science Development Co. Ltd.        ( ( \
 *  Restricted rights to use, duplicate or disclose this code are             gsv''|\
 *  granted through contract.
 *
 */

/*!
 *  ======== debug.h ========
 *  The common header of the debugger
 *
 */

#ifndef __COMMON_DEBUG_H_
#define __COMMON_DEBUG_H_

#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define DBG_DETAILED	1
#define DBG_INFO		2
#define DBG_WARNING		3
#define DBG_ERROR		4
#define DBG_FATAL		5
#define DBG_SILENT		1000

extern int g_debug_level;

/* Global debug mode flag */
extern FILE* gbl_debug_logger;
extern FILE* gbl_debug_logger2;

extern const char* const gbl_months[12];

extern pthread_mutex_t gbl_debug_mutex;

/* Error message */

#define DBG(level, fmt, arg...) if((level) >= g_debug_level && gbl_debug_logger) { \
	time_t tmt; \
	struct tm tm;\
	time(&tmt); \
	localtime_r(&tmt, &tm);\
	pthread_mutex_lock(&gbl_debug_mutex); \
	fprintf(gbl_debug_logger, "[%s %d @ %s %02d, %02d:%02d:%02d] " fmt, __FILE__, __LINE__,  \
			gbl_months[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, \
		##arg) ; \
	fflush(gbl_debug_logger); \
	if(gbl_debug_logger2) { \
		fprintf(gbl_debug_logger2, "[%s %d @ %s %02d, %02d:%02d:%02d] " fmt, __FILE__, __LINE__,  \
				gbl_months[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, \
				##arg) ; \
		fflush(gbl_debug_logger2); \
	} \
	debugger_update(&tm); \
	pthread_mutex_unlock(&gbl_debug_mutex); }

#define ERR(fmt, arg...) DBG(DBG_ERROR, "ERROR: " fmt, ##arg)


/* Function error codes */
#define SUCCESS             0
#define FAILURE             -1

/* Thread error codes */
#define THREAD_SUCCESS      (Void *) 0
#define THREAD_FAILURE      (Void *) -1

#ifdef  TRUE
#undef  TRUE
#endif
#define TRUE    (1)

#ifdef  FALSE
#undef  FALSE
#endif
#define FALSE   (0)

#define SUCCEEDED(r)		((r) >= 0)
#define FAILED(r)			((r) < 0)

// For back compatience

#define DBG1				DBG
#define ERR1				ERR

void debugger_init(FILE* fp, const char* logger_folder);
void debugger_destroy(void);
void debugger_update(struct tm* tm);

void debugger_setlevel(int level);

/* Round up to DM368 cache line (32 bytes) */
#define ROUNDUP_TO_CACHELINE(x)  ((((x) + 31) >> 5) << 5)


#endif
