/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	osa_cmd.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-01-03
 *
 *  @Description:   // 用于详细说明此程序文件完成的主要功能，与其它模块
 *                  // 或函数的接口，输出值，取值范围、含义及参数间的控
 *                  // 制、顺序、独立或依赖等关系
 *                  //
 *
 *                  The header file for osa command.
 *
 *  @Others:        // 其它内容说明
 *
 *  @Function List: // 主要函数列表，每条记录就包括函数名及功能简要说明
 *      1.  ...
 *      2.  ...
 *
 *  @History:       // 修改历史记录列表，每条修改记录就包括修改日期、修改
 *                  // 时间及修改内容简述
 *
 *  <author>        <time>       <version>      <description>
 *
 *  xiong-kaifang   2014-01-03     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__OSA_CMD_H)
#define __OSA_CMD_H

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
#define OSA_CMD_ARGS_MAX    (32)

#define OSA_CMD_MSGS_SIZE   (512)

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
enum __osa_cmd_arg_type_t; typedef enum __osa_cmd_arg_type_t osa_cmd_arg_type_t;
enum __osa_cmd_arg_type_t
{
    OSA_CMD_ARG_STRING   = 0,
    OSA_CMD_ARG_SIGNED	 = 1,
    OSA_CMD_ARG_UNSIGNED = 2,
    OSA_CMD_ARG_IPADDR	 = 3,
    OSA_CMD_ARG_DATETIME = 4,
    OSA_CMD_ARG_BOOLEAN  = 5,
};

enum __osa_cmd_access_t; typedef enum __osa_cmd_access_t osa_cmd_access_t;
enum __osa_cmd_access_t
{
    OSA_CMD_ACCESS_ALL   = 0,
    OSA_CMD_ACCESS_ADMIN = 1,
    OSA_CMD_ACCESS_USER  = 2,
};

struct __osa_cmd_arg_t; typedef struct __osa_cmd_arg_t osa_cmd_arg_t;
struct __osa_cmd_arg_t
{
    osa_cmd_arg_type_t
                    m_type;
    const char    * m_tag;
    const char    * m_name;
    void          * m_default_value;
    unsigned int    m_required;
};

struct __osa_cmd_t; typedef struct __osa_cmd_t osa_cmd_t;
struct __osa_cmd_t
{
    unsigned int    m_reserved[2];

    unsigned int    m_id;

    const char    * m_cmd;
    const char    * m_description;

    osa_cmd_access_t
                    m_access;

    unsigned int    m_nargs;
    osa_cmd_arg_t * m_args;
};

union __osa_cmd_req_arg_t; typedef union __osa_cmd_req_arg_t osa_cmd_req_arg_t;
union __osa_cmd_req_arg_t
{
    int             m_iValue;
    unsigned int    m_uValue;
    char *          m_strValue;
    unsigned char * m_ustrValue;
    void *          m_pValue;
};

struct __osa_cmd_req_t; typedef struct __osa_cmd_req_t osa_cmd_req_t;
struct __osa_cmd_req_t
{
    int             m_id;

    osa_cmd_req_arg_t
                    m_args[OSA_CMD_ARGS_MAX];

    status_t        m_status;
    unsigned char   m_msgs[OSA_CMD_MSGS_SIZE];
};

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:      // 函数名称
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
status_t osa_cmd_request(osa_cmd_req_t *preq);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__OSA_CMD_H) */
