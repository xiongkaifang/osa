
/*
 * osa_console_cmds.h
 *
 * This file is automatically generated on Tue Jul 28 16:48:16 2015
 * DO NOT modify this file!
 *
 */

#if !defined (__OSA_CONSOLE_CMDS_H)
#define __OSA_CONSOLE_CMDS_H

/* Module command codes */

#define OSA_CONSOLE_CMD_CREATE_CHANNEL               100
#define OSA_CONSOLE_CMD_DELETE_CHANNEL               101
#define OSA_CONSOLE_CMD_SET_BITRATE                  102

#define OSA_CONSOLE_CMDS_NUMS                          3

extern osa_cmd_t gbl_osa_console_cmds[OSA_CONSOLE_CMDS_NUMS];

#define OSA_CONSOLE_CMD_CREATE_CHANNEL_ARG_c               0
#define OSA_CONSOLE_CMD_DELETE_CHANNEL_ARG_c               0
#define OSA_CONSOLE_CMD_SET_BITRATE_ARG_c               0
#define OSA_CONSOLE_CMD_SET_BITRATE_ARG_r               1

#endif  /* if !defined (__OSA_CONSOLE_CMDS_H) */
