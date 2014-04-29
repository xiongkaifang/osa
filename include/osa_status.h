/** ============================================================================
 *
 *  Copyright (C), 1987 - 2013, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_status.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-04-07
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The format for header file.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *	    1.  Date:
 *	        Author:
 *	        Modification:
 *	    2.  ...
 *
 *  ============================================================================
 */

#if !defined (__OSA_STATUS_H)
#define __OSA_STATUS_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */

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
#define OSA_STATUS_CLASS_GENERAL    (0x100000)
#define OSA_STATUS_CLASS_NETWORK    (0x200000)
#define OSA_STATUS_CLASS_IO         (0x400000)
#define OSA_STATUS_CLASS_APP        (0x800000)

#define OSA_STATUS_TYPE_SUCCESS     (0x000000)
#define OSA_STATUS_TYPE_INFO        (0x010000)
#define OSA_STATUS_TYPE_WARNING     (0x020000)
#define OSA_STATUS_TYPE_ERROR       (0x040000)
#define OSA_STATUS_TYPE_FATAL       (0x080000)

#define OSA_STATUS_FLAG_COUNT       (0x001000)

#ifdef  OSA_SOK
#undef  OSA_SOK
#endif

#ifdef  OSA_EFAIL
#undef  OSA_EFAIL
#endif

#define OSA_SOK                     (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_SUCCESS|0|100)
#define OSA_EFAIL                   (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|101)
#define OSA_EMEM                    (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|102)
#define OSA_ENOENT                  (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|103)
#define OSA_EARGS                   (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|104)
#define OSA_EINVAL                  (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|105)
#define OSA_EEXIST                  (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|106)
#define OSA_ERES                    (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|107)
#define OSA_EPERM                   (OSA_STATUS_CLASS_GENERAL|OSA_STATUS_TYPE_ERROR|0|108)
#define OSA_ECONNECT                (OSA_STATUS_CLASS_NETWORK|OSA_STATUS_TYPE_ERROR|0|200)
#define OSA_ETIMEOUT                (OSA_STATUS_CLASS_IO|OSA_STATUS_TYPE_ERROR|0|300)
#define OSA_EEOF                    (OSA_STATUS_CLASS_IO|OSA_STATUS_TYPE_WARNING|0|301)
#define OSA_ENOTIPL                 (OSA_STATUS_CLASS_APP|OSA_STATUS_TYPE_WARNING|0|600)
#define OSA_EBADCMD                 (OSA_STATUS_CLASS_APP|OSA_STATUS_TYPE_ERROR|0|601)

#ifdef  OSA_ISERROR
#undef  OSA_ISERROR
#endif
#define OSA_ISERROR(status)         (status != (OSA_SOK))

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
typedef int status_t;

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:	    //	函数名称
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  @Calls:	        //	被本函数调用的函数清单
 *
 *  @Called By:	    //	调用本函数的函数清单
 *
 *  @Table Accessed://	被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: //	被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        //	对输入参数的说明
 *
 *  @Output:	    //	对输出参数的说明
 *
 *  @Return:	    //	函数返回值的说明
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:	    //	其它说明
 *
 *  ============================================================================
 */
const char * osa_status_get_description (status_t status);

status_t     osa_status_get_description2(status_t status, char *buf, int len);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_STATUS_H) */
