#!/usr/bin/python
#
# OTAWA Extension Installer
#
# This file is part of OTAWA
# Copyright (c) 2016, IRIT UPS.
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

##
# @addtogroup commands
# @section otawa-install otawa-install.py Command
#
# `otawa-install.py` is an easy way to install extension in OTAWA.
# It automatically download a package index from the OTAWA's
# website and (a) it can display it and (b) retrieve
# source file of the wanted extensions and their dependencies,
# compile and install it. Notice that it needs OTAWA to be
# installed first.
#
# In the example below, the extension `otawa-sparc` is required
# that implies the download and the compilation of `gliss2` and
# `sparc`:
#
# @code
# > otawa-install.py otawa-sparc
# Looking for tools:
# 	looking for c++ ... [OK]
#	looking for cmake ... [OK]
#	looking for hg ... [OK]
#	looking for make ... [OK]
#	looking for ocamlc ... [OK]
# downloading gliss2 ... [OK]
# building gliss2 ... [OK]
# downloading sparc ... [OK]
# building sparc ... [OK]
# downloading otawa-sparc ... [OK]
# building otawa-sparc ... [OK]
# Installing otawa-sparc ... [OK]
# @endcode
#	
# In the case there is an error, the full log of installation
# can be found in a file named build.log.
#
# otawa-install.py supports the following options:
# @li a list of extensions to install,
# @li `--base`, `-B` URL -- place to retrieve the package index
# (if it is note the main OTAWA website),
# @li `--list`, `-l` -- display the list of extensions found in the package index,
# @li `--top`, `-t` TOP -- usually, otawa-install.py automatically retrieve the OTAWA
# top directory if the command otawa-config is reachable; if it fails, you can
# used this option to pass it.
# @li `--build-dir`, `-B` PATH -- select the build directory (as a default, the build is
# performed in the current directory),
# @li `--dry`, `-D` -- do not perform the installation, only display operations to perform.
# @li `--verbose`, `-v` -- display more details on the operations.
# @li `--log`, `-L` PATH -- select the file to output the log to (as default, build.log
# of the build directory).


import argparse
import datetime
import os
import os.path
import shutil
import subprocess
import sys
import urllib2
import urlparse


class Monitor:
	
	def __init__(self):
		self.out = sys.stdout
		self.err = sys.stderr
		self.verbose = False
		self.build_dir = os.getcwd()
		self.otawa_dir = ""
		self.env = { }
		self.log_file = self.err

	def get(self, name, default = None):
		try:
			return self.env[name]
		except KeyError:
			return default

	def set(self, name, val):
		self.env[name] = val

	def say(self, msg):
		self.out.write("%s\n" % msg)
	
	def warn(self, msg):
		self.err.write("WARNING: %s\n" % msg)
	
	def error(self, msg):
		self.err.write("ERROR: %s\n" % msg)

	def fatal(self, msg):
		self.err.write("ERROR: %s\n" % msg)
		exit(1)
	
	def check(self, msg):
		self.out.write("%s ... " % msg)
		self.out.flush()
	
	def succeed(self):
		self.out.write("[OK]\n")
	
	def fail(self):
		self.out.write("[FAILED]\n")

	def comment(self, msg):
		if self.verbose:
			self.err.write("%s\n" % msg)
	
	def execute(self, cmd, out = None, err = None, log = False):
		if not out:
			if log:
				out = self.log_file
			else:
				out = self.out
		if not err:
			if log:
				err = self.log_file
			else:
				err = self.err
		return subprocess.call(cmd, stdout=out, stderr=err, shell=True)

	def open_log(self, path):
		try:
			self.log_file = open(path, "w")
			self.log("=== created by otawa-install.py V1.0 (%s) ===\n\n" % datetime.datetime.now())
		except IOError as e:
			self.warn("cannot log to %s: %s. Falling back to stderr." % (path, e))
			self.log_file = self.stderr

	def log(self, msg):
		self.log_file.write(msg)
		self.log_file.flush()


class Test:
	
	def test(self, mon):
		return False


class InOtawaTest(Test):
	
	def __init__(self, arg):
		self.file = arg
	
	def test(self, mon):
		path = os.path.join(mon.get("otawa_dir"), self.file)
		return os.path.exists(path)


def make_test(arg):
	return InOtawaTest(arg)


class Downloader:
	
	def __init__(self, pack):
		self.pack = pack
	
	def download(self, mon):
		return False


class HgDownloader(Downloader):
	
	def __init__(self, pack, arg):
		Downloader.__init__(self, pack)
		self.address = arg
	
	def download(self, mon):
		target = os.path.join(mon.build_dir, self.pack.name)
		if os.path.exists(target):
			shutil.rmtree(target)
		cmd = "hg clone %s %s" % (self.address, target)
		mon.log("\nDowloading %s: %s" % (self.pack.name, cmd))
		res = mon.execute(cmd, mon.log_file, mon.log_file)
		return res == 0


DOWNLOADERS = {
	"hg": HgDownloader
}
def make_downloader(pack, arg):
	p = arg.find(":")
	if p < 0:
		return None
	try:
		return DOWNLOADERS[arg[:p]](pack, arg[p + 1:])
	except KeyError:
		return None


class Builder:
	
	def __init__(self, pack):
		self.pack = pack

	def build(self, mon):
		return True
	
	def install(self, mon):
		return True

class MakeBuilder(Builder):
	
	def __init__(self, pack, args):
		Builder.__init__(self, pack)
		self.bflags = ""
		self.iflags = ""
		parts = args.split(":")
		if len(parts) >= 1:
			self.bflags = parts[0]
		if len(parts) >= 2:
			self.iflags = parts[1]
	
	def build(self, mon):
		path = os.path.join(mon.build_dir, self.pack.name)
		cur = os.getcwd()
		os.chdir(path)
		cmd = "make %s" % self.bflags
		mon.log("\nBuilding %s: %s\n" % (self.pack.name, cmd))
		res = mon.execute(cmd, log = True)
		os.chdir(path)
		return res == 0

	def install(self, mon):
		path = os.path.join(mon.build_dir, self.pack.name)
		cur = os.getcwd()
		os.chdir(path)
		cmd = "make install %s" % self.iflags
		mon.log("\nInstalling %s: %s\n" % (self.pack.name, cmd))
		res = mon.execute(cmd, log = True)
		os.chdir(path)
		return res == 0
		

class CMakeBuilder(Builder):
	
	def __init__(self, pack, args):
		Builder.__init__(self, pack)
	
	def build(self, mon):
		path = os.path.join(mon.build_dir, self.pack.name)
		cur = os.getcwd()
		os.chdir(path)
		cmd = "cmake . -DOTAWA_CONFIG=%s/bin/otawa-config" % mon.get("otawa_dir")
		mon.log("\nSetting up %s: %s\n" % (self.pack.name, cmd))
		res = mon.execute(cmd, log = True)
		if res == 0:
			mon.log("\nBuilding %s: make\n" % self.pack.name)
			res = mon.execute("make", log = True)
		os.chdir(path)
		return res == 0		
		
	def install(self, mon):
		path = os.path.join(mon.build_dir, self.pack.name)
		cur = os.getcwd()
		os.chdir(path)
		cmd = "make install"
		mon.log("\nInstalling %s: %s\n" % (self.pack.name, cmd))
		res = mon.execute(cmd, log = True)
		os.chdir(path)
		return res == 0

BUILDERS = {
	"make": MakeBuilder,
	"cmake": CMakeBuilder
}
def make_builder(pack, name, args):
	try:
		return BUILDERS[name](pack, args)
	except KeyError:
		return None


class Package:
	
	def __init__(self, name):
		self.name = name
		self.tools = []
		self.download = None
		self.uses = []
		self.requires = []
		self.build = None
		self.test = Test()
		self.done = False
	
	def do_test(self, mon):
		return self.test.test(mon)
	
	def is_ready(self):
		return all([pack.done for pack in self.uses + self.requires]) 


DEFAULT_DB = "http://www.tracesgroup.net/otawa/index.pkg"
DB = { }
MONITOR = Monitor()


# parse arguments
parser = argparse.ArgumentParser(description="installer for OTAWA plugins")
parser.add_argument('--base', '-b',
	help="select the database to use (default to " + DEFAULT_DB,
	default=DEFAULT_DB)
parser.add_argument('--list', '-l', action="store_true",
	help="display the list of available packages")
parser.add_argument('--top', '-t',
	help="select the top directory of OTAWA")
parser.add_argument('--build-dir', '-B', default=MONITOR.build_dir,
	help="select the directory to perform the build in")
parser.add_argument('--dry', '-D', action="store_true",
	help="dry execution: only display packahes to install")
parser.add_argument('--verbose', '-v', action="store_true",
	help="verbose mode")
parser.add_argument('--log', '-L',
	help="select the log file to use")
parser.add_argument('packages', nargs='*', help="packages to install")
args = parser.parse_args()

BDIR = args.build_dir
MONITOR.verbose = args.verbose
DRY = args.dry
MONITOR.comment("build directory = %s" % BDIR)
if not DRY:
	if args.log:
		MONITOR.open(args.log)
	else:
		MONITOR.open_log(os.path.join(MONITOR.build_dir, "build.log"))


# find the top directory of OTAWA
TOP = args.top
if not TOP:
	try:
		TOP = subprocess.check_output('otawa-config --prefix', shell=True)[:-1]
	except subprocess.CalledProcessError as e:
		MONITOR.fatal("cannot find OTAWA: %s" % e)
MONITOR.comment("OTAWA directory = %s" % args.base)
MONITOR.set("otawa_dir", TOP)
	

# get the database
url = urlparse.urljoin('file://' + os.getcwd(), args.base)
MONITOR.comment("getting database from %s" % url)
pack = None
try:
	stream = urllib2.urlopen(url)
	for line in stream.readlines():
		if line[-1] == "\n":
			line = line[:-1]
		if line.startswith("package:"):
			pack = Package(line[8:])
			MONITOR.comment("\tpackage %s" % pack.name)
			DB[pack.name] = pack
		elif line.startswith('download:'):
			pack.download = make_downloader(pack, line[9:])
			assert pack.download
		elif line.startswith('tools:'):
			pack.tools = line[6:].split(',')
		elif line.startswith('uses:'):
			pack.uses = line[5:].split(',')
		elif line.startswith('requires:'):
			pack.uses = line[9:].split(',')
		elif line.startswith('test:'):
			pack.test = make_test(line[5:])
		elif line.startswith('build:'):
			line = line[6:]
			p = line.find(":")
			if p < 0:
				pack.build = make_builder(pack, line, "")
			else:
				pack.build = make_builder(pack, line[:p], line[p+1:])
			assert pack.build
		elif not line:
			pass
		else:
			assert False
except AssertionError:
	MONITOR.fatal("ERROR: bad DB. Stopping.")
except urllib2.URLError as e:
	MONITOR.fatal(e)


# list action
if args.list:
	for pack in DB.values():
		MONITOR.say(pack.name)
	exit(0)


# prepare the installation
names = args.packages
if not names:
	exit(0)
try:
	packs = [DB[name] for name in names]
except KeyError as e:
	MONITOR.fatal("unknown package: %s" % e)


# closure of packages to download
to_download = list(packs)
todo = list(packs)
try:
	while todo:
		pack = todo.pop()
		pack.uses = [DB[spack] for spack in pack.uses]
		pack.requires = [DB[spack] for spack in pack.requires]
		for spack in pack.uses + pack.requires:
			if spack not in to_download and not spack.test.test(MONITOR):
				to_download.append(spack)
				todo.append(spack)
except KeyError as e:
	MONITOR.comment("cannot find module %s" % e)
	MONITOR.fatal("corrupted DB. Stopping.")


# closure of packages to install
to_install = list(packs)
todo = list(packs)
while todo:
	pack = todo.pop()
	for spack in pack.requires:
		if spack not in to_install:
			to_install(spack)
			todo.append(spack)


# collect required tools
tools = []
for pack in to_download:
	tools = tools + [tool for tool in pack.tools if tool not in tools]

# dry execution
if DRY or MONITOR.verbose:
	MONITOR.say("to download: %s" % ", ".join([pack.name for pack in to_download]))
	MONITOR.say("to install: %s" % ", ".join([pack.name for pack in to_install]))
	MONITOR.say("tools: %s\n" % ", ".join(tools))
if DRY:
	exit(0)


# looking for tools
if tools:
	paths = os.environ["PATH"].split(os.pathsep)
	failed = False
	MONITOR.say("Looking for tools:")
	for tool in tools:

		# display test
		MONITOR.check("\tlooking for %s" % tool)
		
		# perform the test
		success = False
		for path in [os.path.join(path, tool) for path in paths]:
			if os.access(path, os.X_OK):
				success = True
				break
		
		# display test result
		if success:
			MONITOR.succeed()
			MONITOR.comment("\t\tat %s\n" % path)
		else:
			MONITOR.fail()
	if failed:
		exit(1)


# start download and compile
todo = list(to_download)
while todo:
	pack = filter(lambda p: p.is_ready(), todo)[0]
	todo.remove(pack)
	
	# download
	MONITOR.check("downloading %s" % pack.name)
	if pack.download.download(MONITOR):
		MONITOR.succeed()
	else:
		MONITOR.fail()
		MONITOR.fatal("Download failed: see errors in build.log")

	# build
	MONITOR.check("building %s" % pack.name)
	if pack.build.build(MONITOR):
		MONITOR.succeed()
	else:
		MONITOR.fail()
		MONITOR.fatal("Build failed: see errors in build.log")

	# install
	if pack in to_install:
		MONITOR.check("Installing %s" % pack.name)
		if pack.build.install(MONITOR):
			MONITOR.succeed()
		else:
			MONITOR.fail()
			MONITOR.fatal("Installation failed: see errors in build.log")

	# success: done
	pack.done = True
