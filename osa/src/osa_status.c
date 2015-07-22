/** ============================================================================
 *
 *  Copyright (C), 1987 - 2014, xiong-kaifang Tech. Co.,, Ltd.
 *
 *  @File name:	osa_status.c
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2014-03-27
 *
 *  @Description:   The description of this file.
 *	
 *	                The template format for source file.
 *
 *  @Version:	    v1.0
 *
 *  @Function List:  //	主要函数及功能
 *	    1.  －－－－－
 *	    2.  －－－－－
 *
 *  @History:	     //	历史修改记录
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2014-03-27     v1.0	        write this module.
 *
 *
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <string.h>

/*  --------------------- Include user headers   ---------------------------- */
#include "osa.h"
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
#define OSA_STATUS_DESCP_NUM    (14)

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
struct __osa_status_descp_t; typedef struct __osa_status_descp_t osa_status_descp_t;
struct __osa_status_descp_t
{
    unsigned int    m_id;
    const char    * m_msg;
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
static osa_status_descp_t glb_status_descps[OSA_STATUS_DESCP_NUM] = 
{
	{
		.m_id  = OSA_SOK,
		.m_msg = "Success.",
	},
	{
		.m_id  = OSA_EFAIL,
		.m_msg = "General failure.",
	},
	{
		.m_id  = OSA_EMEM,
		.m_msg = "Out of memory.",
	},
	{
		.m_id  = OSA_ENOENT,
		.m_msg = "No such entry.",
	},
	{
		.m_id  = OSA_EARGS,
		.m_msg = "Invalid argument.",
	},
	{
		.m_id  = OSA_EINVAL,
		.m_msg = "Invalid value.",
	},
	{
		.m_id  = OSA_EEXIST,
		.m_msg = "Entry already eixst.",
	},
	{
		.m_id  = OSA_ERES,
		.m_msg = "No enough resource.",
	},
	{
		.m_id  = OSA_EPERM,
		.m_msg = "Operation no permitted.",
	},
	{
		.m_id  = OSA_ECONNECT,
		.m_msg = "Network connect error.",
	},
	{
		.m_id  = OSA_ETIMEOUT,
		.m_msg = "I/O timeout.",
	},
	{
		.m_id  = OSA_EEOF,
		.m_msg = "Reach the end of file.",
	},
	{
		.m_id  = OSA_ENOTIPL,
		.m_msg = "Command not implemented.",
	},
	{
		.m_id  = OSA_EBADCMD,
		.m_msg = "Command not supported.",
	},
};

/*
 *  --------------------- Local function forward declaration -------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function forward declaration.
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

/*
 *  --------------------- Public function definition ---------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Public function definition.
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
const char * osa_status_get_description (status_t status)
{
    int   i;
    char *msg = NULL;

    for (i = 0; i < OSA_ARRAYSIZE(glb_status_descps); i++) {
        if (glb_status_descps[i].m_id == status) {
            msg = (char *)glb_status_descps[i].m_msg;
            break;
        }
    }

    return msg;
}

status_t osa_status_get_description2(status_t status, char *buf, int len)
{
    int      i;
    status_t retval = OSA_ENOENT;

    for (i = 0; i < OSA_ARRAYSIZE(glb_status_descps); i++) {
        if (glb_status_descps[i].m_id == status) {
            snprintf(buf, len - 1, "%s", glb_status_descps[i].m_msg);
            status = OSA_SOK;
            break;
        }
    }

    return retval;
}

/*
 *  --------------------- Local function definition ----------------------------
 */

/** ============================================================================
 *
 *  @Function:	    Local function definition.
 *
 *  @Description:   //	函数功能、性能等的描述
 *
 *  ============================================================================
 */

#if defined(__cplusplus)
}
#endif
