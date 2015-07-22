/*
 *  Copyright 2011 by Beijing e-Trafficvision Technology Co.Ltd.
 *                                                                    ,_
 *  All rights reserved. Property of Beijing e-Trafficvision         >' )
 *  Technology Co.Ltd. Restricted rights to use, duplicate or        ( ( \
 *  disclose this code are granted through contract.               gsv''|\
 *
 */
/*
 *  socket_ops.c
 *
 *      Rountines to manipulate socket.
 */

/* include system header files */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>
#include <fcntl.h>

/* include user defined header files */
#include "socket_ops.h"

#if defined (__cplusplus)
extern  "C" {
#endif  /* if defined __cplusplus */

/*  
 *  Macro definitions.
 */
#define BACKLOG             5   // maximum length of penging connections

#define CONNECTION_TIMEOUT  (2) // 2s for connection timeout

//#define OSA_ETIMEOUT        (-1)

/* Pirvate data type definitions */

/* Globale variables definition*/

/* Private functions forward declaration */

static inline
void initialize_timeout(struct timeval *ptv, unsigned long timewait)
{
    ptv->tv_sec  = (timewait / 1000);
    ptv->tv_usec = (timewait % 1000) * 1000;
}

/*  
 *  Function definitions.
 *
 */
int ssocket_init(const struct sockaddr_in *addr, int addr_len)
{
    int           optval;
    int           socket_fd;
    status_t  status = OSA_SOK;

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd == -1) {
        return OSA_EFAIL;
    }

    /*
     *  Reuse ip address and port.
     */
    optval = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, 
                    sizeof(optval)) < 0) {
    }

    if(bind(socket_fd, (struct sockaddr *)addr, addr_len) < 0) {
        close(socket_fd);
        return OSA_EFAIL;
    }

    if(listen(socket_fd, BACKLOG) < 0) {
        close(socket_fd);
        return OSA_EFAIL;
    }

    return socket_fd;
}

int csocket_init(const char *ip, int port)
{
    int                 socket_fd;
    int                 addr_len;
    struct sockaddr_in  server_addr;
    status_t        status = OSA_SOK;

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd == -1) {
        return OSA_EFAIL;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    addr_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_aton(ip, &server_addr.sin_addr) == 0) {
        return OSA_EFAIL;
    }

    if(connect(socket_fd, (struct sockaddr *)&server_addr, addr_len) < 0) {
        close(socket_fd);

        return OSA_EFAIL;
    }

    return socket_fd;
}

#if 1
int csocket_init2(const char *ip, int port, unsigned int timeout)
{
    int                 r;
    int                 socket_fd;
    int                 addr_len;
    struct timeval      tv;
    fd_set              wr_fds;
    int                 optval;
    socklen_t           optlen;
    struct sockaddr_in  server_addr;
    status_t        status = OSA_SOK;

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd == -1) {
        return OSA_EFAIL;
    }

    r = socket_set_noblocking(socket_fd);
    if (r < 0) {
        return OSA_EFAIL;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    addr_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if(inet_aton(ip, &server_addr.sin_addr) == 0) {
        return OSA_EFAIL;
    }

    r = connect(socket_fd, (struct sockaddr *)&server_addr, addr_len);
    if (r < 0) {
        /*
         *  Trying to connect with timeout.
         */

        /* Initialilze connection timeout */
        initialize_timeout(&tv, timeout);

        /* Init fdset */
        FD_ZERO(&wr_fds);
        FD_SET(socket_fd, &wr_fds);

        r = select(socket_fd + 1, NULL, &wr_fds, NULL, &tv);
        if (r < 0 && errno != EINTR) {
                status = OSA_EFAIL;
        } else if (r > 0) {
            optval = 0;
            optlen = sizeof(int);

            if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, 
                    (void *)&optval, &optlen) < 0) {
                status = OSA_EFAIL;
            }

            if (!OSA_ISERROR(status)) {
                /* Check the value returned */
                if (!optval) {
                    /* Connection is established */
                    r = 0;
                } else {
                    r = -1;
                }
            }
        } else {
            /* Connection timeout */
            //DBG(DBG_WARNING, "csocket_init2: Connection timeout.\n");
            status = OSA_ETIMEOUT;
        }
    }

    if (!OSA_ISERROR(status) && r == 0) {
        r = socket_set_blocking(socket_fd);
    } else {
        close(socket_fd);
        socket_fd = -1;
    }

    return socket_fd;
}
#endif

/*
 *  This call return the number of bytes successfully received,
 *  or socket error, time out.
 */
status_t
socket_recv(int sockfd, char *buf, size_t len, size_t *psize, unsigned long timewait)
{
    int             ret;
    struct timeval  tv;
    fd_set          rd_fds;
    status_t    status = OSA_SOK;

    /* Initialize the time out */
    initialize_timeout(&tv, timewait);

    (*psize) = 0;

    /* Init fdset */
    FD_ZERO(&rd_fds);
    FD_SET(sockfd, &rd_fds);

    /* select: synchronous I/O multiplexing */
    ret = select(sockfd + 1, &rd_fds, NULL, NULL, &tv);
    if(ret > 0) {
        if(FD_ISSET(sockfd, &rd_fds)) {

            /* The sockfd is available to read */
            ret = recv(sockfd, buf, len, 0);
            if (ret < 0) {
                /* TODO: Error accurred, set error code */
                status   = OSA_EFAIL;
            } else if(ret == 0) {
                /* The peer has performed an orderly shutdown */
                status   = OSA_EFAIL;
            } else {
                /* Return the bytes received */
                (*psize) = ret;
            }
        }
    } else if(ret == 0) {
        /* Error: Timeout */
        //DBG(DBG_INFO, "socket_recv: Timeout.\n");
        status =  OSA_ETIMEOUT;
    } else {
        /* Error: General fail  */
        status =  OSA_EFAIL;
    }

    return status;
}

/*
 *  This call return the number of bytes successfully sent,
 *  or socket error, time out.
 */
status_t
socket_send(int sockfd, const char *buf, size_t len, size_t *psize, unsigned long timewait)
{
    int             ret;
    struct timeval  tv;
    fd_set          wr_fds;
    status_t    status = OSA_SOK;

    /* Initialize the time out */
    initialize_timeout(&tv, timewait);

    (*psize) = 0;

    /* Init fdset */
    FD_ZERO(&wr_fds);
    FD_SET(sockfd, &wr_fds);

    /* select: synchronous I/O multiplexing */
    ret = select(sockfd + 1, NULL, &wr_fds, NULL, &tv);
    if(ret > 0) {
        if(FD_ISSET(sockfd, &wr_fds)) {

            /* The sockfd is available to write */
            //ret = send(sockfd, buf, len, 0);
            ret = send(sockfd, buf, len, MSG_DONTWAIT | MSG_NOSIGNAL);
            if (ret < 0) {
                /* TODO: Error accurred, set error code */
                status =  OSA_EFAIL;
            } else {
                /* Return the bytes sent */
                (*psize) = ret;
            }
        }
    } else if(ret == 0) {
        /* Error: Timeout */
        status =  OSA_ETIMEOUT;
    } else {
        /* Error: General fail  */
        status =  OSA_EFAIL;
    }

    return status;
}

/*
 *  This call return the number of bytes received, waiting for receipt
 *  of the full amount requested, or socket error, time out.
 */
status_t
socket_recv2(int sockfd, char *buf, size_t len, unsigned long timewait)
{
    size_t          bytes_recv;
    size_t          bytes_remain;
    status_t    status = OSA_SOK;

    bytes_recv   = 0;
    bytes_remain = len;

    while( bytes_remain ) {

        /* socket recv */
        status = socket_recv(sockfd, buf, bytes_remain, &bytes_recv, timewait);
        if(!OSA_ISERROR(status)) {
            buf          += bytes_recv;
            bytes_remain -= bytes_recv;
        } else {
            /* Error */
            break;
        }
    }

    return status;
}

/*
 *  This call return the number of bytes sent, waiting for receipt
 *  of the full amount requested, or socket error, time out.
 */
status_t
socket_send2(int sockfd, const char *buf, size_t len, unsigned long timewait)
{
    size_t          bytes_send;
    size_t          bytes_remain;
    status_t    status = OSA_SOK;

    bytes_send   = 0;
    bytes_remain = len;

    while( bytes_remain ) {

        /* socket send */
        status = socket_send(sockfd, buf, bytes_remain, &bytes_send, timewait);
        if(!OSA_ISERROR(status)) {
            buf          += bytes_send;
            bytes_remain -= bytes_send;
        } else {
            /* Socket error */
            break;
        }
    }

    return status;
}

/*
 *  Local socket connection stop
 */
status_t socket_connect_stop(int                       sockfd,
                              const struct sockaddr_in *addr,
                              int                       addr_len,
                              int                       flags)
{
    int     retval;
    int     socket_fd;
    char    buf[10] = { 0 };
 
    if(flags == EXIT_FROM_RECV) {
        /* send some data to server */
        socket_fd = sockfd;

        //do {
            //retval = send(socket_fd, buf, sizeof(buf), 0);
            //close(socket_fd);
        //} while(retval < 0);

    } else if(flags == EXIT_FROM_ACCEPT) {
        /* create a socket endpoint for connection */
        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_fd == -1) {
            return OSA_EFAIL;
        }

        /* initiate a connection on a socket */
        if(OSA_ISERROR(connect(socket_fd, (struct sockaddr *)addr, addr_len))) {
            return OSA_EFAIL;
        }

        /* close the connection */
        close(socket_fd);
    }

    return OSA_SOK;
}

/*
 *  Get local host IP address.
 */
status_t  socket_get_addr_info(const char *interface, char *ip_str)
{
    status_t    status = OSA_SOK;
    int           fd;
    struct ifreq  ifr;

    memset(ip_str, 0, sizeof(ip_str));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* get IP address attached to the interface */
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

    if(OSA_ISERROR(ioctl(fd, SIOCGIFADDR, &ifr))) {
        return OSA_EFAIL;
    }

    close(fd);

    sprintf(ip_str, "%s", 
         inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    return status;
}

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
status_t socket_get_inet_address(const char *interface, char *ip_str)
{
    status_t status = OSA_EFAIL;
    int family, s;
    char host[NI_MAXHOST];
    struct ifaddrs *ifaddr, *ifa;

    if ((s = getifaddrs(&ifaddr)) == -1) {
        return status;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (strcmp(ifa->ifa_name, interface) || family != AF_INET) {
            continue;
        }

        s = getnameinfo(ifa->ifa_addr,
                (family == AF_INET) ? sizeof (struct sockaddr_in) : sizeof(struct sockaddr_in6),
                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if (s == 0) {
            sprintf(ip_str, "%s", host);
            status = OSA_SOK;
        }
    }

    return status;
}

int socket_set_noblocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);

	if(flags < 0) {
		return flags;
	}

	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int socket_set_blocking(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);

	if(flags < 0) {
		return flags;
	}

	return fcntl(fd, F_SETFL, (flags &= (~O_NONBLOCK)));
}

unsigned int socket_get_buffer_size(int fd, int opt)
{
    unsigned int    curSize = 0;
    socklen_t       szeLen  = sizeof (curSize);

    if (getsockopt(fd, SOL_SOCKET, opt, (void *)&curSize, &szeLen) < 0) {

        fprintf(stderr, "socket_get_buffer_size: Failed to get buffer size.\n");
        curSize = 0;
    }

    return curSize;
}

unsigned int  socket_set_buffer_size(int fd, int opt, unsigned int size)
{
    socklen_t       szeLen = sizeof (size);

    setsockopt(fd, SOL_SOCKET, opt, (void *)&size, szeLen);

    return socket_get_buffer_size(fd, opt);
}

#if defined (__cplusplus)
}
#endif  /* if defined __cplusplus */

