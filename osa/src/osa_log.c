/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_log.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2016-09-08
 *
 *  @Description:   The osa logger.
 *
 *
 *  @Version:       v1.0
 *
 *  @Function List: // 主要函数及功能
 *      1.  －－－－－
 *      2.  －－－－－
 *
 *  @History:       // 历史修改记录
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2016-09-08     v1.0	        Write this module.
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa_log.h"
#include "osa_mutex.h"

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
struct __osa_log_object_t;
typedef struct __osa_log_object_t osa_log_object_t;
struct __osa_log_object_t
{
    osa_log_params_t
                    m_log_prm;

    FILE *          m_out;
    FILE *          m_out2;
    unsigned int    m_log_level;
    osa_mutex_t     m_mutex;
    struct tm       m_tm;
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
static unsigned int         glb_cur_init = 0;

static osa_log_object_t     glb_log_obj;

static const char * const   GT_NAME = "osa_log";

static const char * const   LOG_STR[] = {
    "Verbose",
    "Debug",
    "Info",
    "Warning",
    "Error",
    "Fatal",
    "Panic",
    "Quiet",
    NULL
};

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function forward declaration.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  @Calls:	        // 被本函数调用的函数清单
 *
 *  @Called By:	    // 调用本函数的函数清单
 *
 *  @Table Accessed:// 被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: // 被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        // 对输入参数的说明
 *
 *  @Output:        // 对输出参数的说明
 *
 *  @Return:        // 函数返回值的说明
 *
 *  @Enter          // Precondition
 *
 *  @Leave          // Postcondition
 *
 *  @Others:        // 其它说明
 *
 *  ============================================================================
 */
static status_t
__osa_log_init(osa_log_object_t *pobj, const osa_log_params_t *prm);

static status_t __osa_log_deinit(osa_log_object_t *pobj);

static void     __osa_log_update(struct tm *ptm);

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:      Public function definition.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  @Calls:	        // 被本函数调用的函数清单
 *
 *  @Called By:	    // 调用本函数的函数清单
 *
 *  @Table Accessed:// 被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: // 被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        // 对输入参数的说明
 *
 *  @Output:        // 对输出参数的说明
 *
 *  @Return:        // 函数返回值的说明
 *
 *  @Enter          // Precondition
 *
 *  @Leave          // Postcondition
 *
 *  @Others:        // 其它说明
 *
 *  ============================================================================
 */
status_t osa_log_init(const osa_log_params_t *prm)
{
    status_t         status = OSA_SOK;
    osa_log_object_t * pobj = &glb_log_obj;

    if (glb_cur_init++ == 0) {
        status = __osa_log_init(pobj, prm);
    }

    return status;
}

void     osa_log(void *ctx, int level, const char *fmt, ...)
{
    osa_log_object_t * pobj = &glb_log_obj;

    if (level >= pobj->m_log_level) {

        osa_mutex_lock(pobj->m_mutex);

        struct tm tim;
        struct timeval tv;
        va_list ap;

        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &tim);

        if (pobj->m_out != NULL) {
            fprintf(pobj->m_out, "%d-%02d-%02dT%02d:%02d:%02d.%03d|%12s| %s: ",
                    tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday, tim.tm_hour, tim.tm_min, tim.tm_sec,
                    tv.tv_usec / 1000, (unsigned char *)ctx, LOG_STR[level]);
            va_start(ap, fmt);
            vfprintf(pobj->m_out, fmt, ap);
            va_end(ap);
            fflush(pobj->m_out);
        }

        if (pobj->m_out2 != NULL) {
            fprintf(pobj->m_out2, "%d-%02d-%02dT%02d:%02d:%02d.%03d|%12s| ",
                    tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday, tim.tm_hour, tim.tm_min, tim.tm_sec,
                    tv.tv_usec / 1000, (unsigned char *)ctx);
            va_start(ap, fmt);
            vfprintf(pobj->m_out2, fmt, ap);
            va_end(ap);
            fflush(pobj->m_out2);
        }

        __osa_log_update(&tim);

        osa_mutex_unlock(pobj->m_mutex);
    }
}

void     osa_vlog(void *ctx, int level, const char *fmt, va_list vl)
{
}

int      osa_log_get_level(void)
{
    int                level;
    osa_log_object_t * pobj = &glb_log_obj;

    osa_mutex_lock(pobj->m_mutex);

    level = pobj->m_log_level;

    osa_mutex_unlock(pobj->m_mutex);

    return level;
}

status_t osa_log_set_level(int level)
{
    osa_log_object_t * pobj = &glb_log_obj;

    if (level < OSA_LOG_VERBOSE || level > OSA_LOG_QUIET) {
        return OSA_EINVAL;
    }

    osa_mutex_lock(pobj->m_mutex);

    pobj->m_log_level = level;

    osa_mutex_unlock(pobj->m_mutex);

    OSA_LOG(GT_NAME, OSA_LOG_INFO, "Logger system set log level to: %s.\n", LOG_STR[pobj->m_log_level]);

    return OSA_SOK;
}


void     osa_log_set_callback(void (*callback)(void *, int, const char *, va_list))
{
}

void     osa_log_default_callback(void *ctx, int level, const char *fmt, va_list vl)
{
}

status_t osa_log_deinit(void)
{
    status_t         status = OSA_SOK;
    osa_log_object_t * pobj = &glb_log_obj;

    if (--glb_cur_init == 0) {
        status = __osa_log_deinit(pobj);
    }

    return status;
}


/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:      Local function definition.
 *
 *  @Description:   // 函数功能、性能等的描述
 *
 *  ============================================================================
 */
static status_t
__osa_log_init(osa_log_object_t *pobj, const osa_log_params_t *prm)
{
    status_t status = OSA_SOK;

    memset(pobj, 0, sizeof(*pobj));

    memcpy(&pobj->m_log_prm, prm, sizeof(*prm));

    pobj->m_out       = prm->m_out;
    pobj->m_log_level = prm->m_level;

    status = osa_mutex_create(&pobj->m_mutex);
    if (OSA_ISERROR(status)) {
        return status;
    }

    if (pobj->m_log_prm.m_folder != NULL) {
        unsigned char filename[128];
        time_t        now_time;
        struct tm     tim;

        time(&now_time);
        localtime_r(&now_time, &tim);

        snprintf(filename, sizeof(filename) - 1, "%s/%s_%04d%02d%02d.log",
                pobj->m_log_prm.m_folder,
                pobj->m_log_prm.m_name,
				tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday);

        pobj->m_out2 = fopen(filename, "ab");

        OSA_LOG(GT_NAME, OSA_LOG_INFO, "Logger system started.\n");
    }

    return status;
}

static status_t
__osa_log_deinit(osa_log_object_t *pobj)
{
    status_t status = OSA_SOK;

    OSA_LOG(GT_NAME, OSA_LOG_INFO, "Logger system stopped.\n\n");

    if (pobj->m_out2 != NULL) {
        fclose(pobj->m_out2);
    }

    status = osa_mutex_delete(&pobj->m_mutex);

    return status;
}

static void __osa_log_update(struct tm *ptm)
{
    osa_log_object_t * pobj = &glb_log_obj;

	if(pobj->m_out2 == NULL) {
		return;
	}

	if (pobj->m_tm.tm_year != ptm->tm_year 
            || pobj->m_tm.tm_mon != ptm->tm_mon
            || pobj->m_tm.tm_mday != ptm->tm_mday) {
		char filename[128];
		pobj->m_tm = *ptm;

        fclose(pobj->m_out2);
        
        snprintf(filename, sizeof(filename) - 1, "%s/%s_%04d%02d%02d.log",
                pobj->m_log_prm.m_folder,
                pobj->m_log_prm.m_name,
				pobj->m_tm.tm_year + 1900, pobj->m_tm.tm_mon + 1, pobj->m_tm.tm_mday);

        pobj->m_out2 = fopen(filename, "ab");
	}
}

#if defined(__cplusplus)
}
#endif
