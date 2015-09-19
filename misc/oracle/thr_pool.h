/** ============================================================================
 *
 *  Copyright (C), 1987 - 2015, xiong-kaifang Tech. Co, Ltd.
 *
 *  @File Name:	thr_pool.h
 *
 *  @Author: xiong-kaifang   Version: v1.0   Date: 2015-09-08
 *
 *  @Description:   //	用于详细说明此程序文件完成的主要功能，与其它模块
 *		            //	或函数的接口，输出值，取值范围、含义及参数间的控
 *		            //	制、顺序、独立或依赖等关系
 *		            //
 *
 *	                The header file for thread pool(By Oracle).
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
 *  xiong-kaifang   2015-09-08     v1.0	        Write this module.
 *
 *  ============================================================================
 */

#if !defined (__THR_POOL_H)
#define __THR_POOL_H

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

/*
 *  --------------------- Data type definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          thr_pool_t
 *
 *  @Description:   The thr_pool_t type is opaque to the client.
 *                  It is created by thr_pool_create() and must be passed
 *                  unmodified to the remainder of the interfaces.
 *
 *  @Field:         Field1 member
 *
 *  @Field:         Field2 member
 *  ----------------------------------------------------------------------------
 */
typedef	struct thr_pool	thr_pool_t;

/*
 *  --------------------- Public function declaration --------------------------
 */

/** =============================================================================
 *
 *  @Function:	    thr_pool_create
 *
 *  @Description:   Create a thread pool.
 *
 *  @Calls:	        //	被本函数调用的函数清单
 *
 *  @Called By:	    //	调用本函数的函数清单
 *
 *  @Table Accessed://	被访问的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Table Updated: //	被修改的表（此项仅对于牵扯到数据库操作的程序）
 *
 *  @Input:	        min_threads: the minimum number of threads kept in the pool,
 *                               always available to perform work requests.
 *
 *                  max_threads: the maximum number of threads that can be
 *                               in the pool, performing work requests.
 *
 *                  linger: the number of seconds excess idle worker threads
 *                          (greater than min_threads) linger before exiting.
 *
 *                  attr: attributes of all worker threads (can be NULL); can be
 *                        destroyed after calling thr_pool_create().
 *
 *  @Output:	    //	对输出参数的说明
 *
 *  @Return:	    On error, thr_pool_create() returns NULL with errno set to
 *                  the error code.
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:	    //	其它说明
 *
 *  ============================================================================
 */
extern	thr_pool_t	*thr_pool_create(uint_t min_threads, uint_t max_threads,
        uint_t linger, pthread_attr_t *attr);


/** =============================================================================
 *
 *  @Function:	    thr_pool_queue
 *
 *  @Description:   Enqueue a work request to the thread pool job queue.
 *                  If there are idle worker threads, awaken one to perform the job.
 *                  Else if the maximum number of workers has not been reached,
 *                  create a new worker thread to perform the job.
 *                  Else just return after adding the job to the queue;
 *                  an existing worker thread will perform the job when
 *                  it finishes the job it is currently performing.
 *
 *                  The job is performed as if a new detached thread were created for it:
 *                  pthread_create(NULL, attr, void *(*func)(void *), void *arg);
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
 *  @Return:	    On error, thr_pool_queue() returns -1 with errno set to the error code.
 *
 *  @Enter          //  Precondition
 *
 *  @Leave          //  Postcondition
 *
 *  @Others:	    //	其它说明
 *
 *  ============================================================================
 */
extern	int	thr_pool_queue(thr_pool_t *pool, void *(*func)(void *), void *arg);

/** =============================================================================
 *
 *  @Function:	    thr_pool_wait
 *
 *  @Description:   Wait for all queued jobs to complete.
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
extern	void	thr_pool_wait(thr_pool_t *pool);

/** =============================================================================
 *
 *  @Function:	    thr_pool_destroy
 *
 *  @Description:   Cancel all queued jobs and destroy the pool.
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
extern	void	thr_pool_destroy(thr_pool_t *pool);

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__THR_POOL_H) */
