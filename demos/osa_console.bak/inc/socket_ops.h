/*
 *  Copyright 2011 by Beijing e-Trafficvision Technology Co.Ltd. 
 *                                                                    ,_
 *  All rights reserved. Property of Beijing e-Trafficvision         >' )
 *  Technology Co.Ltd. Restricted rights to use, duplicate or        ( ( \
 *  disclose this code are granted through contract.               gsv''|\
 *
 */
/*
 *  socket_ops.h
 *
 *      The rountines to manipulate socket.
 */

#ifndef SOCKET_OPS_H_INCLUDED
#define SOCKET_OPS_H_INCLUDED

/* include system header files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* include user defined header files */
#include "osa_status.h"

#if defined (__cplusplus)
extern  "C" {
#endif  /* if defined __cplusplus */

/*
 *  Macro definitions.
 */
#define EXIT_FROM_ACCEPT    (0)
#define EXIT_FROM_RECV      (1)

/* Public data type definitions */

/* Function prototype declearation */

extern int
ssocket_init (const struct sockaddr_in *addr, int addr_len);

extern int
csocket_init (const char *ip, int port);

extern int
csocket_init2(const char *ip, int port, unsigned int timeout);

extern status_t
socket_recv(int sockfd, char *buf, size_t len, size_t *psize, unsigned long timewait);

extern status_t
socket_send(int sockfd, const char *buf, size_t len, size_t *psize, unsigned long timewait);

extern status_t
socket_recv2(int sockfd, char *buf, size_t len, unsigned long timewait);

extern status_t
socket_send2(int sockfd, const char *buf, size_t len, unsigned long timewait);

extern  status_t socket_connect_stop(int                       sockfd,
                                       const struct sockaddr_in *addr,
                                       int                       addr_len,
                                       int                       flags);

extern  status_t socket_get_addr_info(const char *interface, char *ip_str);

/*
 *  Added by   : xiong-kaifang.
 *
 *  Date       : Nov 21, 2013.
 *
 *  Description:
 *
 *      Routine to get network interface ip address, maybe not portable.
 *
 */
extern  status_t socket_get_inet_address(const char *interface, char *ip_str);

extern  int socket_set_noblocking(int fd);

extern  int socket_set_blocking  (int fd);

extern  unsigned int socket_get_buffer_size(int fd, int opt);

extern  unsigned int  socket_set_buffer_size(int fd, int opt, unsigned int size);

#if defined (__cplusplus)
}
#endif  /* if defined __cplusplus */

#endif  /* SOCKET_OPS_H_INCLUDED */
