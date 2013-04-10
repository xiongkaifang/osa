/*
 * debug.c
 *
 *  Created on: Dec 2, 2011
 *      Author: ztao
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"


/* Global debugger loggers */
FILE* gbl_debug_logger;
FILE* gbl_debug_logger2 = NULL;

pthread_mutex_t gbl_debug_mutex;

static const char* dbg_logger_folder;
static struct tm old_tm;

static int  cur_init = 0;

int g_debug_level;

const char* const gbl_months[12] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void debugger_init(FILE* fp, const char* logger_folder)
{
    if (cur_init++ == 0) {
        g_debug_level = DBG_INFO;
        gbl_debug_logger = fp;
        dbg_logger_folder = logger_folder;
        pthread_mutex_init(&gbl_debug_mutex, NULL);
    }
}

void debugger_destroy(void)
{
	time_t tmt;
	time(&tmt);
	struct tm tm;

    if (--cur_init == 0) {
        localtime_r(&tmt, &tm);

        if(gbl_debug_logger2) {
            fprintf(gbl_debug_logger2, "\n\nSystem logger stopped at %s %02d, %02d:%02d:%02d,\n",
                    gbl_months[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            fclose(gbl_debug_logger2);
        }
        pthread_mutex_destroy(&gbl_debug_mutex);
    }
}

void debugger_update(struct tm* tm)
{
	if(dbg_logger_folder == NULL) {
		return;
	}

	if(old_tm.tm_year != tm->tm_year || old_tm.tm_mon != tm->tm_mon || old_tm.tm_mday != tm->tm_mday) {
		char filename[128];
		old_tm = *tm;
		sprintf(filename, "%s/rtspserver_%04d%02d%02d.log", dbg_logger_folder,
				tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

		if(gbl_debug_logger2) {
			fprintf(gbl_debug_logger2, "\n\nSystem logger stopped at %s %02d, %02d:%02d:%02d,\n",
				gbl_months[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
			fclose(gbl_debug_logger2);
		}

		gbl_debug_logger2 = fopen(filename, "a");

		if(gbl_debug_logger2) {
			fprintf(gbl_debug_logger2, "\n\nSystem logger started at %s %02d, %02d:%02d:%02d,\n",
					gbl_months[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
		}
	}
}

void debugger_setlevel(int level)
{
	g_debug_level = level;
}
