
/*
 * vcs_cmds.c
 *
 * This file is automatically generated on Tue Jul 28 15:25:49 2015
 * DO NOT modify this file!
 *
 */

#include "osa_cmd.h"
#include "vcs_cmds.h"
 
#define MAKE_DEFAULT_VALUE(x)   ((void *)(x))

static osa_cmd_arg_t vcs_cmds_logon_args[2] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "u",
        .m_name       = "username",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "p",
        .m_name       = "password",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_useradd_args[4] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "u",
        .m_name       = "username",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "p",
        .m_name       = "password",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "n",
        .m_name       = "full name",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "a",
        .m_name       = "access: admin|video",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_usermod_args[4] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "u",
        .m_name       = "username",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "p",
        .m_name       = "password",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "n",
        .m_name       = "full name",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "a",
        .m_name       = "access: admin|video",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_pwd_args[2] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "u",
        .m_name       = "username",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "p",
        .m_name       = "password",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_userdel_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "u",
        .m_name       = "username",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setnetparams_args[8] =
{
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "ip",
        .m_name       = "ip address",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "nm",
        .m_name       = "netmask",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "gw",
        .m_name       = "gateway ip",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "dhcp",
        .m_name       = "dhcp: 1|0",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(-1),
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "hn",
        .m_name       = "hostname",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(NULL),
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "dns1",
        .m_name       = "DNS 1 ip",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "dns2",
        .m_name       = "DNS 2 ip",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "dns3",
        .m_name       = "DNS 3 ip",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
};

static osa_cmd_arg_t vcs_cmds_setdatetime_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_DATETIME,
        .m_tag        = "t",
        .m_name       = "mmddhhmmCCYY",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setcolorbar_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_BOOLEAN,
        .m_tag        = "mode",
        .m_name       = "mode: yes|no",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_startjpegoutput_args[3] =
{
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "ip",
        .m_name       = "Remote ip address",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "rtp",
        .m_name       = "Remote RTP port",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "rtcp",
        .m_name       = "Remote RTCP port",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
};

static osa_cmd_arg_t vcs_cmds_stopjpegoutput_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "sid",
        .m_name       = "session ID",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_killstream_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "sid",
        .m_name       = "session ID",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_lockmgr_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_BOOLEAN,
        .m_tag        = "enabled",
        .m_name       = "enabled: yes|no",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_keepalive_args[4] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "sid",
        .m_name       = "Session ID",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_IPADDR,
        .m_tag        = "ip",
        .m_name       = "Remote ip address",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "rtp",
        .m_name       = "Remote RTP port",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "rtcp",
        .m_name       = "Remote RTCP port",
        .m_required   = 0,
        .m_default_value = MAKE_DEFAULT_VALUE(0),
    },
};

static osa_cmd_arg_t vcs_cmds_setccdvab_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vab",
        .m_name       = "VAB",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setccdtransvols_args[3] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vh",
        .m_name       = "VH",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vhl",
        .m_name       = "VHL",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "rg",
        .m_name       = "RG",
        .m_required   = 0,
    },
};

static osa_cmd_arg_t vcs_cmds_setsensorparams_args[7] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "sn",
        .m_name       = "Serial Number",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "hw_id",
        .m_name       = "Hardware ID",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vh",
        .m_name       = "VH",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vhl",
        .m_name       = "VHL",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "rg",
        .m_name       = "RG",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "vab",
        .m_name       = "VAB",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_BOOLEAN,
        .m_tag        = "debug",
        .m_name       = "Debug: yes|no",
        .m_required   = 0,
    },
};

static osa_cmd_arg_t vcs_cmds_setshutter_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setrawgain_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value (1-1023, 512 nominal)",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_selectparam_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_getparams_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "idx",
        .m_name       = "Index",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setparams_args[2] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "id",
        .m_name       = "parameter index",
        .m_required   = 0,
    },
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "base64",
        .m_name       = "struct ipnc_camera_param_set, in base64",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_enableautoexp_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_BOOLEAN,
        .m_tag        = "en",
        .m_name       = "Enabled",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_enableautowb_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_BOOLEAN,
        .m_tag        = "en",
        .m_name       = "Enabled",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setwb_args[8] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "gain_r",
        .m_name       = "Gain R",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "gain_gr",
        .m_name       = "Gain GR",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "gain_gb",
        .m_name       = "Gain GB",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "gain_b",
        .m_name       = "Gain B",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "offs_r",
        .m_name       = "Offset R",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "offs_gr",
        .m_name       = "Offset GR",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "offs_gb",
        .m_name       = "Offset GB",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "offs_b",
        .m_name       = "Offset B",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setbrightness_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setcontrast_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setsharpness_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setglobalgain_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setexpcompensation_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setwbcompensation_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setiso_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setgamma_args[3] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "r",
        .m_name       = "Gamma R (Q9)",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "g",
        .m_name       = "Gamma G (Q9)",
        .m_required   = 1,
    },
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "b",
        .m_name       = "Gamma B (Q9)",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setlmmode_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_UNSIGNED,
        .m_tag        = "mode",
        .m_name       = "Mode: 0 - average, 1 - centered",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setdarkcurrent_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_SIGNED,
        .m_tag        = "v",
        .m_name       = "Value",
        .m_required   = 1,
    },
};

static osa_cmd_arg_t vcs_cmds_setcodecsettings_args[1] =
{
    {
        .m_type       = IPNC_CMD_ARG_STRING,
        .m_tag        = "base64",
        .m_name       = "struct ipnc_camera_settings, in base64",
        .m_required   = 1,
    },
};

osa_cmd_t gbl_vcs_cmds[VCS_CMDS_NUMS] =
{
    {
        .id           = IPNC_CMD_LOGON,
        .cmd          = "logon",
        .description  = "Logon the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 2,
        .args         = osa_cmd_logon_args,
    },
    {
        .id           = IPNC_CMD_LOGOFF,
        .cmd          = "logoff",
        .description  = "Logoff the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_QUIT,
        .cmd          = "quit",
        .description  = "Alias of 'logoff'.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_USERADD,
        .cmd          = "useradd",
        .description  = "Add an user to the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 4,
        .args         = osa_cmd_useradd_args,
    },
    {
        .id           = IPNC_CMD_USERMOD,
        .cmd          = "usermod",
        .description  = "Modify an user of the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 4,
        .args         = osa_cmd_usermod_args,
    },
    {
        .id           = IPNC_CMD_USERLIST,
        .cmd          = "userlist",
        .description  = "List users of the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_CHANGEPWD,
        .cmd          = "pwd",
        .description  = "Change user password.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 2,
        .args         = osa_cmd_pwd_args,
    },
    {
        .id           = IPNC_CMD_USERDEL,
        .cmd          = "userdel",
        .description  = "Delete of an user of the IPNC management console.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_userdel_args,
    },
    {
        .id           = IPNC_CMD_REBOOT,
        .cmd          = "reboot",
        .description  = "Reboot the IPNC system.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_SYSINFO,
        .cmd          = "sysinfo",
        .description  = "Get the system info.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_NET_PARAMS,
        .cmd          = "getnetparams",
        .description  = "Get the network parameters.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_SET_NET_PARAMS,
        .cmd          = "setnetparams",
        .description  = "Set the network parameters.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 8,
        .args         = osa_cmd_setnetparams_args,
    },
    {
        .id           = IPNC_CMD_GET_TEMPERATURE,
        .cmd          = "gettemp",
        .description  = "Get the temperature of the IPNC.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_DATETIME,
        .cmd          = "getdatetime",
        .description  = "Get the date/time of the IPNC.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_SET_DATETIME,
        .cmd          = "setdatetime",
        .description  = "Set the date/time of the IPNC.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setdatetime_args,
    },
    {
        .id           = IPNC_CMD_GET_VOLTAGES,
        .cmd          = "getvoltages",
        .description  = "Get the voltages of the IPNC.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_COLORBAR,
        .cmd          = "getcolorbar",
        .description  = "Gets the colorbar mode.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_SET_COLORBAR,
        .cmd          = "setcolorbar",
        .description  = "Sets the colorbar mode.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setcolorbar_args,
    },
    {
        .id           = IPNC_CMD_GET_PEER_IP,
        .cmd          = "getpeerip",
        .description  = "Get the peer (client) IP address.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_JPEGHEADER,
        .cmd          = "getjpegheader",
        .description  = "Gets the header of jpeg compressed images.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_START_JPEG,
        .cmd          = "startjpegoutput",
        .description  = "Start outputting jpeg compressed images.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 3,
        .args         = osa_cmd_startjpegoutput_args,
    },
    {
        .id           = IPNC_CMD_STOP_JPEG,
        .cmd          = "stopjpegoutput",
        .description  = "Stop outputting jpeg compressed images.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_stopjpegoutput_args,
    },
    {
        .id           = IPNC_CMD_GET_DIMS,
        .cmd          = "getdims",
        .description  = "Get video dimension.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_LIST_STREAMS,
        .cmd          = "liststreams",
        .description  = "List video streams.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_KILL_STREAM,
        .cmd          = "killstream",
        .description  = "Kill video stream.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_killstream_args,
    },
    {
        .id           = IPNC_CMD_GET_EMBHEADER,
        .cmd          = "getembheader",
        .description  = "Get the embedded JPEG header status.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_LOCK_MGR,
        .cmd          = "lockmgr",
        .description  = "Lock the device into management mode.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_lockmgr_args,
    },
    {
        .id           = IPNC_CMD_KEEPALIVE,
        .cmd          = "keepalive",
        .description  = "Keep the video stream alive.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 4,
        .args         = osa_cmd_keepalive_args,
    },
    {
        .id           = IPNC_CMD_GETACCESS,
        .cmd          = "getaccess",
        .description  = "Get the access rights of the current user.",
        .access       = IPNC_CMD_ACCESS_ALL,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GETVAB,
        .cmd          = "getccdvab",
        .description  = "Get the CCD VAB voltage.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_SETVAB,
        .cmd          = "setccdvab",
        .description  = "Set the CCD VAB voltage.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 1,
        .args         = osa_cmd_setccdvab_args,
    },
    {
        .id           = IPNC_CMD_G_TRANS_VOLS,
        .cmd          = "getccdtransvols",
        .description  = "Get the CCD Transfer voltages.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_S_TRANS_VOLS,
        .cmd          = "setccdtransvols",
        .description  = "Set the CCD Transfer voltages.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 3,
        .args         = osa_cmd_setccdtransvols_args,
    },
    {
        .id           = IPNC_CMD_G_SENSOR_PARAMS,
        .cmd          = "getsensorparams",
        .description  = "Get the sensor parameters.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_S_SENSOR_PARAMS,
        .cmd          = "setsensorparams",
        .description  = "Set the sensor parameters.",
        .access       = IPNC_CMD_ACCESS_TUNING,
        .nargs        = 7,
        .args         = osa_cmd_setsensorparams_args,
    },
    {
        .id           = IPNC_CMD_SET_SHUTTTER,
        .cmd          = "setshutter",
        .description  = "Set the electronic shutter timing.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_setshutter_args,
    },
    {
        .id           = IPNC_CMD_SET_RAWGAIN,
        .cmd          = "setrawgain",
        .description  = "Set the raw data gain.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_setrawgain_args,
    },
    {
        .id           = IPNC_CMD_SEL_PARAMSET,
        .cmd          = "selectparam",
        .description  = "Select parameter set.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_selectparam_args,
    },
    {
        .id           = IPNC_CMD_GET_PARAMSET,
        .cmd          = "getparams",
        .description  = "Get parameter set (struct ipnc_camera_param_set, in base64).",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 1,
        .args         = osa_cmd_getparams_args,
    },
    {
        .id           = IPNC_CMD_GET_CURRENT_PARAMSET,
        .cmd          = "getcurrentparams",
        .description  = "Get current parameter set (struct ipnc_camera_param_set, in base64).",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_GET_NUM_PARAMSETS,
        .cmd          = "getnumparams",
        .description  = "Get number of parameter sets.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_SET_PARAMSET,
        .cmd          = "setparams",
        .description  = "Set parameter set.",
        .access       = IPNC_CMD_ACCESS_VIDEO,
        .nargs        = 2,
        .args         = osa_cmd_setparams_args,
    },
    {
        .id           = IPNC_CMD_ENABLE_AUTO_EXP,
        .cmd          = "enableautoexp",
        .description  = "Enable auto exposure.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_enableautoexp_args,
    },
    {
        .id           = IPNC_CMD_ENABLE_AUTO_WB,
        .cmd          = "enableautowb",
        .description  = "Enable auto white balance.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_enableautowb_args,
    },
    {
        .id           = IPNC_CMD_SET_WB,
        .cmd          = "setwb",
        .description  = "Set white balance configurations.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 8,
        .args         = osa_cmd_setwb_args,
    },
    {
        .id           = IPNC_CMD_SET_BRIGHTNESS,
        .cmd          = "setbrightness",
        .description  = "Set brightness.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setbrightness_args,
    },
    {
        .id           = IPNC_CMD_SET_CONTRAST,
        .cmd          = "setcontrast",
        .description  = "Set contrast.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setcontrast_args,
    },
    {
        .id           = IPNC_CMD_SET_SHARPNESS,
        .cmd          = "setsharpness",
        .description  = "Set sharpness.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setsharpness_args,
    },
    {
        .id           = IPNC_CMD_SET_GLOBAL_GAIN,
        .cmd          = "setglobalgain",
        .description  = "Set global gain.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setglobalgain_args,
    },
    {
        .id           = IPNC_CMD_SET_EXP_COMPENSATION,
        .cmd          = "setexpcompensation",
        .description  = "Set auto exposure compensation.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setexpcompensation_args,
    },
    {
        .id           = IPNC_CMD_SET_WB_COMPENSATION,
        .cmd          = "setwbcompensation",
        .description  = "Set auto white balance compensation.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setwbcompensation_args,
    },
    {
        .id           = IPNC_CMD_SET_ISO,
        .cmd          = "setiso",
        .description  = "Set ISO (light sensitivity).",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setiso_args,
    },
    {
        .id           = IPNC_CMD_SET_GAMMA,
        .cmd          = "setgamma",
        .description  = "Set gamma..",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 3,
        .args         = osa_cmd_setgamma_args,
    },
    {
        .id           = IPNC_CMD_SET_LM_MODE,
        .cmd          = "setlmmode",
        .description  = "Set light measurment mode.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setlmmode_args,
    },
    {
        .id           = IPNC_CMD_SET_DARK_CURRENT,
        .cmd          = "setdarkcurrent",
        .description  = "Set CCD dark current.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setdarkcurrent_args,
    },
    {
        .id           = IPNC_CMD_SET_CODEC_SETTINGS,
        .cmd          = "setcodecsettings",
        .description  = "Set codec settings.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 1,
        .args         = osa_cmd_setcodecsettings_args,
    },
    {
        .id           = IPNC_CMD_GET_CODEC_SETTINGS,
        .cmd          = "getcodecsettings",
        .description  = "Set codec settings (struct ipnc_camera_settings, in base64).",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_DUMP_WORKING_STATUS,
        .cmd          = "dumpstatus",
        .description  = "Dump working status.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
    {
        .id           = IPNC_CMD_WRITE_SETTINGS,
        .cmd          = "writesettings",
        .description  = "Write settings into EEPROM.",
        .access       = IPNC_CMD_ACCESS_ADMIN,
        .nargs        = 0,
        .             = NULL,
    },
};

/* Generated file vcs_cmds.c ends. */
