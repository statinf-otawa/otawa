#!/usr/bin/python
#
# OTAWA Plugin Generator
#
# This file is part of OTAWA
# Copyright (c) 2005-10, IRIT UPS.
# 
# OTAWA is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# OTAWA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OTAWA; if not, write to the Free Software 
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import argparse
import os.path

VERSION="1.0.0"

# parse arguments
parser = argparse.ArgumentParser(description = "OTAWA Project Generator")
parser.add_argument('free', type=str, nargs='*', metavar="args", help="NAME FILE...")
parser.add_argument('-v', '--version', action="store_true", default=False, help="display version")
parser.add_argument('-I', '--ilp', action="store_true", default=False, help="generate an ILP plugin")
parser.add_argument('-L', '--loader', action="store_true", default=False, help="generate a loader plugin")
parser.add_argument('-S', '--script', action="store_true", default=False, help="generate for a script support")
parser.add_argument('-e', '--elf', metavar='elf', type=int, help='code for ELF files')
parser.add_argument('-d', '--description', type=str, help="provide description of the module")
parser.add_argument('-l', '--license', type=str, help="provide the license of the module")
parser.add_argument('-a', '--author', type=str, help="provide author")
parser.add_argument('-s', '--site', type=str, help="provide website")
parser.add_argument('-r', '--require', type=str, help="require the given plugin", nargs='*')

args = parser.parse_args()
do_version = args.version
do_loader = args.loader
do_ilp = args.ilp
do_script = args.script
elf = args.elf
free = args.free
description = args.description
license = args.license
author = args.author
site = args.site
reqs = args.require

# process version
if do_version:
    print "otawa-gen %s" % version
    exit(0)

# scan free arguments
if not free:
    print "ERROR: at least, module name must be given."
    parser.print_help()
    exit(1)
name = free[0]
files = free[1:]
eld_path = "%s.elf" % name

# extract namespace
try:
    p = name.rindex('/')
    namespace = name[:p]
    name = name[p+1:]
except ValueError, e:
    namespace = ""


# check for existing files
for f in ["CMakeLists.txt", eld_path]:
    if os.path.exists(f):
        print "ERROR: a %s already exists." % f

# generate CMakeLists
out = open("CMakeLists.txt", "w")
out.write("""
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# configuration
set(PLUGIN       "%s")       # plugin name
set(NAMESPACE    "%s")        # namespace
set(SOURCES      "%s.cpp")    # sources of the plugin

# script
project(${PLUGIN})

# look for OTAWA
if(NOT OTAWA_CONFIG)
    find_program(OTAWA_CONFIG otawa-config DOC "path to otawa-config")
    if(NOT OTAWA_CONFIG)
        message(FATAL_ERROR "ERROR: otawa-config is required !")
    endif()
endif()
message(STATUS "otawa-config found at ${OTAWA_CONFIG}")
execute_process(COMMAND "${OTAWA_CONFIG}" --cflags ${MODULES} OUTPUT_VARIABLE OTAWA_CFLAGS  OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --libs ${MODULES}   OUTPUT_VARIABLE OTAWA_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND "${OTAWA_CONFIG}" --prefix            OUTPUT_VARIABLE OTAWA_PREFIX  OUTPUT_STRIP_TRAILING_WHITESPACE)

# plugin definition
set(ORIGIN $ORIGIN)
set(CMAKE_INSTALL_RPATH "${ORIGIN}/../../../")
include_directories("${CMAKE_SOURCE_DIR}" ".")
add_library(${PLUGIN} SHARED ${SOURCES})
set_property(TARGET ${PLUGIN} PROPERTY PREFIX "")
set_property(TARGET ${PLUGIN} PROPERTY COMPILE_FLAGS "${OTAWA_CFLAGS}")
target_link_libraries(${PLUGIN} "${OTAWA_LDFLAGS}")

# installation
set(PLUGIN_PATH "${OTAWA_PREFIX}/lib/otawa/proc/${NAMESPACE}")
install(TARGETS ${PLUGIN} LIBRARY DESTINATION ${PLUGIN_PATH})
""" % (name, namespace, name))

if do_script:
    out.write("install(FILES %s.osx DESTINATION ${OTAWA_PREFIX}/share/Otawa/scripts)\n" % name)

for f in files:
    p = f.index(':')
    if p < 0:
        out.write("install(FILES %s DESTINATION ${OTAWA_PREFIX}/share/Otawa/)\n" % f)
    else:
        out.write("install(FILES %s DESTINATION ${OTAWA_PREFIX}/%s)\n" % (f[:p], f[p+1:]))

# generate ELD file
eld = open("%s.eld" % name, "w")
eld.write("[elm-plugin]\n")
if description:
    eld.write("description=%s\n" % description)
if author:
    eld.write("author=%s\n" % author)
if site:
    eld.write("site=%s\n" % site)
if reqs:
    eld.write("deps=%s\n" % ";".join(reqs))


# for loader, generate ELF ELD file
if do_loader:
    eld = open("%d.eld" % elf)
    eld.write("[elm-plugin]\n")
    eld.write("path=%s\n" % name)


# for script, generates initial OSX
if do_script:
    eld = open("%s.osx" % name, "w")
    eld.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<otawa-script
    xmlns:xi="http://www.w3.org/2001/XInclude"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<name>%s</name>

<info>
This script provides support for %s.
</info>

<id>
    <arch>%s</arch>
    <abi></abi>
    <mach></mach>
</id>


<configuration>

</configuration>

<platform>

</platform>

<script>

</script>

</otawa-script>
""" % (name, name, name))


# generate initial source
if not namespace:
    names = [name]
else:
    names = namespace.split("/") + [name]
ns_begin = "".join([("namespace %s {" % name) for name in names])
ns_end = "} " * len(names)
ns_ref = "::".join(names)
ns_id = "_".join(names)
src = open("%s.cpp" % name, "w")

if do_loader:
    src.write(
"""/*
 *
 */

#include <otawa/prog/Loader.h>

""")

elif do_ilp:
    src.write(
"""/*
 *
 */

#include <otawa/ilp/ILPPlugin.h>
""")

else:
    src.write(
"""/*
 *
 */

#include <otawa/proc/ProcessorPlugin.h>

%s

using namespace elm;
using namespace otawa;

class Plugin: public ProcessorPlugin {
public:
    typedef elm::genstruct::Table<AbstractRegistration * > procs_t;

    Plugin(void): ProcessorPlugin("%s", Version(1, 0, 0), OTAWA_PROC_VERSION) { }
    virtual procs_t& processors (void) const { return procs_t::EMPTY; };
};

%s

%s::Plugin %s_plugin;
ELM_PLUGIN(%s_plugin, OTAWA_PROC_HOOK);

""" % (ns_begin, ns_ref, ns_end, ns_ref, ns_id, ns_id));


print """Generaton done. Now type:
cmake .
make install
"""
