#!/usr/bin/python
#
# -*- coding: utf-8 -*-
#
################################################################################
#
#
#  @File name:	module_cmds_parser.py
#
#  @Author: ztao   Version: v1.0   Date: 2012-03-20
#
#  @Description:   The python script to generate module command codes.
#
#
#  @History:	   Review history
#
#	<author>	    <time>	     <version>	    <desc>
#  z-tao           2012-03-20     v1.0	        Write this script.
#
#  xiong-kaifang   2015-07-28     v2.0          Rewrite this scrip.
#
################################################################################

import os
import sys
import time
import traceback

from xml.dom.minidom import parse
from string import Template


try:

    xmlfile  = sys.argv[1]

    dom      = parse(xmlfile)
    nod_name = dom.documentElement.nodeName

    source   = '{0}.c'.format(nod_name)
    header   = '{0}.h'.format(nod_name)

    source_template = Template("""
/*
 * ${module_name}.c
 *
 * This file is automatically generated on ${generation_time}
 * DO NOT modify this file!
 *
 */

#include "osa_cmd.h"
#include "${module_name}.h"
 
#define MAKE_DEFAULT_VALUE(x)   ((void *)(x))

"""
    )

    header_template = Template("""
/*
 * ${module_name}.h
 *
 * This file is automatically generated on ${generation_time}
 * DO NOT modify this file!
 *
 */

#if !defined (__${module_name_upper}_H)
#define __${module_name_upper}_H

"""
    )

    source_tailer = "/* Generated file {0}.c ends. */\n"
    header_tailer = "#endif  /* if !defined (__{0}_H) */\n"


    src_fp        = open(source, "w")
    hdr_fp        = open(header, "w")

    try:

        hdr_fp.write(header_template.substitute(module_name     = nod_name,
                                                generation_time = time.ctime(time.time()),
                                                module_name_upper = nod_name.upper()))

        src_fp.write(source_template.substitute(module_name     = nod_name,
                                                generation_time = time.ctime(time.time())))

        cmds_list = dom.documentElement.getElementsByTagName("cmd")
	
		#Write file contents here
        cmd_nums = 0
		
        hdr_fp.write("/* Module command codes */\n\n")

        for cmd in cmds_list:
            id       = cmd.getAttribute("id")
            id_txt   = cmd.getAttribute("idtxt")
            hdr_fp.write("#define {0:32s}{1:16d}\n".format(id_txt, int(id)))
            cmd_nums = cmd_nums + 1

        hdr_fp.write("\n#define {0:32s}{1:16d}\n\n".format(nod_name.upper() +'_NUMS', cmd_nums))
        hdr_fp.write("extern osa_cmd_t gbl_{0}[{1}_NUMS];\n\n".format(nod_name,
                    nod_name.upper()))
		
        for cmd in cmds_list:
            id       = cmd.getAttribute("idtxt")
            cmd_txt   = cmd.getAttribute("cmd")
            args_list = cmd.getElementsByTagName("arg")

            if(args_list.length > 0):
                arg_idx  = 0
                arg_name = "{0}_{1}_args".format(nod_name, cmd_txt)
                src_fp.write("static osa_cmd_arg_t {0}[{1}] =\n".format(arg_name, args_list.length))
                src_fp.write("{\n")

                for cmd_arg in args_list :
                    arg_type 	 = cmd_arg.getAttribute("type")
                    arg_tag 	 = cmd_arg.getAttribute("tag")
                    arg_name	 = cmd_arg.getAttribute("name")
                    arg_required = cmd_arg.getAttribute("required")
                    arg_defv	 = cmd_arg.getAttribute("default_value")

                    src_fp.write("    {\n")
                    src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_type", arg_type))
                    src_fp.write("{0:8s}.{1:12s} = \"{2}\",\n".format(' ', "m_tag", arg_tag))
                    src_fp.write("{0:8s}.{1:12s} = \"{2}\",\n".format(' ', "m_name", arg_name))
                    src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_required", arg_required))
                    
                    if arg_defv != "":
                        src_fp.write("{0:8s}.{1:12s} = MAKE_DEFAULT_VALUE({2}),\n".format(' ', "m_default_value", arg_defv))

                    src_fp.write("    },\n")
					
                    hdr_fp.write("#define {0:32s}{1:16d}\n".format(id + "_ARG_" + arg_tag, arg_idx))

                    arg_idx		 = arg_idx + 1

                src_fp.write("};\n\n")

        hdr_fp.write("\n")
        src_fp.write("osa_cmd_t gbl_{0}[{1}] =\n".format(nod_name, nod_name.upper() + '_NUMS'))
        src_fp.write("{\n")

        for cmd in cmds_list:
            id        = cmd.getAttribute("idtxt")
            cmd_txt   = cmd.getAttribute("cmd")
            desc      = cmd.getAttribute("desc")
            access 	  = cmd.getAttribute("access")
            args_list = cmd.getElementsByTagName("arg")
            nargs     = args_list.length

            src_fp.write("    {\n")
            src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_id", id))
            src_fp.write("{0:8s}.{1:12s} = \"{2}\",\n".format(' ', "m_cmd", cmd_txt))
            src_fp.write("{0:8s}.{1:12s} = \"{2}\",\n".format(' ', "m_description", desc))
            src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_access", access))
            src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_nargs", nargs))

            if(nargs == 0):
                src_fp.write("{0:8s}.{0:12s} = NULL,\n".format(' ', "m_args"))
            else:
                arg_name = "{0}_{1}_args".format(nod_name, cmd_txt)
                src_fp.write("{0:8s}.{1:12s} = {2},\n".format(' ', "m_args", arg_name))

            src_fp.write("    },\n")

        src_fp.write("};\n\n")


        #End writting file contents
        hdr_fp.write(header_tailer.format(nod_name.upper()))
        src_fp.write(source_tailer.format(nod_name))

    finally:
        src_fp.close();
        hdr_fp.close();


    print '{0}: Successed to generate {0} files.'.format(nod_name)

except:

    traceback.print_exc()
    sys.exit( '{0}: Failed to generate {0} files.'.format(nod_name))
