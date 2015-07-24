
/*
 *  module_cmds.h
 *
 *  This file is automatically generated on Mon Jan 20 14:14:19 2014
 *  DO NOT modify this file!!!
 */

#ifndef __MODULE_CMDS_H
#define __MODULE_CMDS_H

#include "osa_cmd.h"


/* module command codes */

#define MODULE_CMD_CREATE_CHANNEL                    100
#define MODULE_CMD_DELETE_CHANNEL                    101
#define MODULE_CMD_SET_BITRATE                       102

#define MODULE_CMDS_NUM                                3

extern osa_cmd_t glb_module_cmds[MODULE_CMDS_NUM];

#define MODULE_CMD_CREATE_CHANNEL_ARG_c                0
#define MODULE_CMD_DELETE_CHANNEL_ARG_c                0
#define MODULE_CMD_SET_BITRATE_ARG_c                   0
#define MODULE_CMD_SET_BITRATE_ARG_r                   1

#endif /* __MODULE_CMDS_H */