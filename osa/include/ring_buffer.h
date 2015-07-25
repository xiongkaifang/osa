/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	ring_buffer.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2013-08-16
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file for ring buffer.
 *
 *  @Others:	    //	其它内容说明
 *
 *  @Function List: //	主要函数列表，每条记录就包括函数名及功能简要说明
 *	    1.  ...
 *	    2.  ...
 *
 *  @History:	    //	修改历史记录列表，每条修改记录就包括修改日期、修改
 *	        	    //	时间及修改内容简述
 *
 *	<author>	    <time>	     <version>	    <desc>
 *  xiong-kaifang   2013-08-16     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__RING_BUFFER_H)
#define __RING_BUFFER_H

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
status_t ring_buffer_init(ring_buffer_t *ring, void *paddr, unsigned int size);

status_t ring_buffer_writer_acquire(ring_buffer_t *ring, void **ptr, unsigned int acq_size);

status_t ring_buffer_writer_release(ring_buffer_t *ring, unsigned int rel_size);

status_t ring_buffer_write_cancel  (ring_buffer_t *ring);

status_t ring_buffer_reader_acquire(ring_buffer_t *ring, void **ptr, unsigned int *pacq_size);

status_t ring_buffer_reader_release(ring_buffer_t *ring, unsigned int rel_size);

status_t ring_buffer_reader_cancel (ring_buffer_t *ring);

status_t ring_buffer_deinit(ring_buffer_t *ring);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__RING_BUFFER_H) */
