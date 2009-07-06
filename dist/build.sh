#!/bin/bash
tool=otawa-build-new
version=2.1

# Actions
#	help
#	dist
#	install
#	make

# Build modes
#	dev		development
#	debug	debugging
#	normal	normal working
#	prod	production mode

# Global variables
#	action: type of performed action
#	added_modules: modules added to the current selection
#	config: configuration file
#	curdir: invocation directory
#	cvs_user: CVS user
#	done: list of done modules
#	jobs: number of jobs for make
#	log: log file
#	mod: current module
#	modules: list of modules to build
#	options: module providing an option
#		(must define functions option_XXX, help_XXX)
#	package: package name
#	prefix: path to install to
#	verbose: verbose mode
#	with_so: shared library activated

# Module description
#	NAME: name of the module
#	REQUIRE: required modules
#	USE: possibly used modules
#	DESCRIPTION: description of the module
#	VARIABLES: variables used by the module

#	CLEAN: command to clean the sources
#	CHECK: command to call to check for existence
#	DOWNLOAD: command to download the module
#	PATCH: command to patch the downloaded module
#	SETUP: command to setup the downloaded module
#	BUILD: command to build the module
#	INSTALL: command to install the module
#	DIST: command to distribute the source of the module

# Command
#
#	clean_XXX (CLEAN)
#		clean_make
#			do "make clean"
#		clean_non
#			no cleanup
#
#	check_XXX (CHECK)
#		check_tool
#			check if a tool exist
#			NAME: tool name
#			VERSION: tool version
#		check_exist
#			check if some files exist
#			NAME: name of the directory
#			CHECK_FILES: file to check in directory $NAME
#
#	download_XXX (DOWNLOAD)
#		download_home
#			download from cvs.irit.fr
#			NAME: module name
#			CVS_MOD=NAME: CVS module name
#			VERSION?: version to download
#		download_cvs
#			download from CVS
#			NAME: module name
#			CVS_MOD=NAME: CVS module name
#			VERSION?: version to download
#			CVS_ROOT: CVSROOT to use
#		download_wget
#			download using wget
#			WGET_ADDRESS: address of the site
#			WGET_PACKAGE: namee of the package
#			WGET_DIR?: directory name to rename unpacked package
#		download_link
#			LINKPATH: path to the ressource to link
#		download_bzr
#			download from a Bazaar repository
#			VERSION: module version
#			BZR_REPO: location of the repository
#
#	patch_XXX (PATCH)
#		patch_fun
#			call patch_$NAME
#			NAME: name of the module
#
#	setup_XXX (SETUP)
#		setup_autotool
#			aclocal, autoheader, automake, autoconf
#		setup_libtool
#			aclocal, autoheader, libtoolize, automake, autoconf
#			LIBTOOLIZE_FLAGS: flags for libtoolize
#		setup_bootstrap
#			call "bootstrap"
#
#	build_XXX (BUILD)
#		build_autotool
#			perform configure and make all
#			prefix: installation directory
#			with_so: if "yes", use of shared libraries
#			AUTOCONF_FLAGS: flags for configure (???)
#			MAKE_FLAGS: flags for make
#		build_make
#			perform a simple make
#			MAKE_FLAGS: flags for make
#		build_cmd
#			use a special command
#			BUILD_CMD: command to use
#
#	install_XXX (INSTALL)
#		install_make
#			peform "make install"
#		install_autotool
#			same as install_make
#
#	dist_XXX (DIST)
#		distdir: directory to install the distribution in
#		dist_autotool
#			use autotool to make distribution
#			NAME: name of the module
#			DIST_CONFIGURE_FLAGS: flags for configure in distribution
#		dist_copy
#			build distribution by simple copy
#			NAME: name of the module



# Configuration
action=make
added_modules=
basedir=otawa
config=
curdir=`pwd`
cvs_user=anonymous
debug=no
dev=no
done=
jobs=5
log=build.log
mode=
modules=
package=otawa-dist
prefix=
systemc_location=
verbose=no
with_so=no


###### Util functions ######

# Get the basolute name of a path
#	$1	file to get the absolute path of.
#	return the absolute path
function absname {
	if expr "$1" : "/.*" > /dev/null; then
		echo "$1"
	else
		echo "`pwd`/$1"
	fi
}


# Output message to the log
function log {
	echo -e "$*\n" >> $basedir/$log
}

# Output a message only in verbose mode
function say_verbose {
	if [ "$verbose" == yes ] ; then
		echo $*
		log $*
	fi
}

# Display if verbose mode is enabled.
function display {
	if test "$verbose" = yes; then
		echo -e $*
		log $*
	fi
}

# Display an error
function error {
	echo -e "ERROR:$*"
	log "ERROR: $*"
	exit 2
}

# Give some information (ever displayed and output to the log)
function info {
	echo -e "$*"
	log "$*"
}


# Start a say ... success/failed message
function say {
	say="$* ..."
	echo -n "$say"
}


# Sucess after a say
function success {
	echo " [DONE]"
	log "$say [DONE]"
}


# Failed after a say
function failed {
	echo " [FAILED]"
	if [ -n "$*" ]; then
		echo "$*"
	fi
	log "$say [FAILED]"
	if [ -n "$*" ]; then
		log "$*"
	else
		echo "====== ERROR ======"
		tail $basedir/$log
		echo "==================="
	fi
	exit 1
}


# Perform a command and log its output
function log_command {
	say "$*"
	echo "$*" | bash >> $basedir/$log 2>&1 || failed
	success
}


# contains item item_list
#	Test if the item is contained in the item list.
function contains {
	key="$1"
	shift
	for item in $*; do
		if [ "$key" = "$item" ] ; then
			return 0
		fi
	done
	return 1
}


########### clean_XXX ###########

#	NAME : module name
#	CLEAN : clean mode (make[default], none)
function do_clean {
	if [ -n "CLEAN" ]; then
		clean_$CLEAN
	fi
}

# Clean invoking the make.
function clean_make {
	log_command make clean
}

# No clean action available.
function clean_none {
	true
}


########### check_XXX ###########

#	NAME : module name
#	CHECK : check mode (tool, exist)
function do_check {
	if [ -n "$CHECK" ]; then
		say_verbose "CHECK: checking $NAME for $CHECK"
		if check_$CHECK; then
			say_verbose "CHECK: $NAME already done !"
			done="$done $NAME"
			true
			return
		fi
	fi
	say_verbose "CHECK: $NAME must be done..."
	false
}

# Check for an external tool
#	NAME	name of the tool
#	VERSION	version of the tool
function check_tool {
	program=$NAME
	version=$VERSION
	#expr "$checked" : "$program" > /dev/null && return

	#echo "$program -> $version"
	say "Checking $program for version $version"
	which $program > /dev/null || failed "not found";
 	cversion=`$program --version | head -1 |  sed -e 's/[^0-9]\+ \([0-9]\+\.[0-9]\+\(\.[0-9]\+\)\?\).*/\1/'`
	major=${version%%.*}
	cmajor=${cversion%%.*}
	minor=${version#*.}
	cminor=${cversion#*.}
	if [ "$major" -gt "$cmajor" -o "${minor%%.*}" -gt "${cminor%%.*}"  ]; then
		failed "at least version $version required !"
	else
		checked="$checked $program"
		success
		true
	fi
}


# Check for built files
#	NAME		name of the built module
#	CHECK_FILES	files to check
#	CHECK_DIRS	directories to check
function check_exist {
	say_verbose "CHECK: exists $CHECK_FILES, $CHECK_DIRS ?"
	for file in $CHECK_FILES; do
		if [ ! -e "$basedir/$NAME/$file" ]; then
			say_verbose "CHECK: $basedir/$NAME/$file does not exist !"
			false
			return
		fi
	done
	for dir in $CHECK_DIRS; do
		if [ ! -d "$basedir/$NAME/$dir" ]; then
			say_verbose "CHECK: $basedir/$NAME/$dir does not exist !"
			false
			return
		fi
	done
	true
}


############# download_XXX #################

# Do the download of the current module
# 	DOWNLOAD	type of download to perform (home, cvs, wget).
function do_download {
	if [ -n "$DOWNLOAD" ]; then
		download_$DOWNLOAD
	fi
}

# Download from OTAWA home repository.
#	NAME		name of the module
#	VERSION		version to download
#	BZR_REPO	bazaar repository
function download_bzr {
	FLAGS=
	if [ -n "$VERSION" ]; then
		FLAGS="$FLAGS -r $VERSION"
	fi
	log_command bzr export $FLAGS $NAME $BZR_REPO/$NAME
}


# Download from OTAWA home repository.
#	NAME	name of the module
#	CVS_MOD	if different of the name
#	VERSION	version to download
function download_home {
	if [ -z "$CVS_MOD" ]; then
		CVS_MOD=$NAME
	fi
	FLAGS=
	#get_tag $NAME $VERSION
	if [ -n "$VERSION" ]; then
		FLAGS="$FLAGS -r $VERSION"
	fi
	log_command cvs -d ":pserver:$cvs_user@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA" co $FLAGS $CVS_MOD
}

# Download from CVS server
#	NAME		name of the module
#	CVS_MOD		if different of the name
#	CVS_ROOT	CVS root to use
function download_cvs {
	if [ -z "$CVS_MOD" ]; then
		CVS_MOD=$NAME
	fi
	FLAGS=
	#get_tag $NAME $VERSION
	if [ -n "$VERSION" ]; then
		FLAGS="$FLAGS -r $VERSION"
	fi
	log_command cvs -d $CVS_ROOT co $FLAGS $CVS_MOD
}

# Download using wget
#	WGET_ADDRESS: address to download from,
#	WGET_PACKAGE: package to download,
#	WGET_DIR: directory name after unpacking.
#
function download_wget {
	log_command wget $WGET_ADDRESS/$WGET_PACKAGE
	wpackage=${WGET_PACKAGE%.tgz}
	if test "$wpackage" != $WGET_PACKAGE; then
		log_command tar xvfz $WGET_PACKAGE
	else
		wpackage=${WGET_PACKAGE%.tar.gz}
		if test "$wpackage" != $WGET_PACKAGE; then
			log_command tar xvfz $WGET_PACKAGE
		else
			error "Unsupported archive"
		fi
	fi
	if [ "$WGET_DIR" = "" ]; then
		WGET_DIR="$wpackage"
	fi
	if test "$mod" != "$WGET_DIR"; then
		log_command ln -s $WGET_DIR $mod
	fi
	rm -rf $WGET_PACKAGE
}


###### patch_XXX ######

# Do the patch of the current module
# 	PATCH	type of patch to perform (fun).
function do_patch {
	if [ -n "$PATCH" ]; then
		patch_$PATCH
	fi
}

# Patch by calling the function patch_$NAME
# NAME	 module name
function patch_fun {
	say "patching "
	patch_$NAME >> $basedir/$log 2>&1 || failed
	success
}


# Use a simple link
function download_link {
	if [ -z "$LINKPATH" ]; then
		error "no path given for $NAME link"
	fi
	log_command ln -s "$LINKPATH" $NAME
}



########## setup_XXX ############

# Do the setup of the current module
# 	SETUP	type of setup to perform (autotool, libtool, bootstrap).
function do_setup {
	if [ -n "$SETUP" ]; then
		setup_$SETUP
	fi
}

# Use autotool for setup.
function setup_autotool {
	if [ ! -e configure ]; then
		log_command aclocal
		log_command autoheader
		log_command automake --add-missing
		log_command autoconf
	fi
}

# Use libtool for setup.
#	LIBTOOLIZE_FLAGS	flags to pass to libtoolize.
function setup_libtool {
	if [ ! -e configure ]; then
		log_command aclocal
		log_command autoheader
		log_command libtoolize $LIBTOOLIZE_FLAGS
		log_command automake --add-missing
		log_command autoconf
	fi
}

# Use the bootstrap file found in the module.
function setup_bootstrap {
	if [ ! -e configure ]; then
		log_command ./bootstrap
	fi
}


########## build_XXX ############

# Do the build of the current module
# 	BUILD	type of build to perform (autotool, make, cmd).
function do_build {
	if [ -n "$BUILD" ]; then
		build_$BUILD
	fi
}

# Build using autotool
#	AUTOCONF_DEBUG	debug flags to pass to autoconf
#	AUTOCONF_FLAGS	flags to pass to autoconf
#	MAKE_FLAGS		flags to pass to make
function build_autotool {
	if [ ! -e Makefile ]; then
		args="--prefix=$prefix"
		if [ "$with_so" == yes ]; then
			args="$args --enable-shared"
		fi
		args="$args $AUTOCONF_FLAGS"
		log_command ./configure  $args
	fi
	log_command make all $MAKE_FLAGS
}

# Build using a simple make
#	MAKE_FLAGS	flags to pass to make
function build_make {
	#echo "make all $MAKE_FLAGS"
	echo "#!/bin/bash" > build.sh
	echo "make all $MAKE_FLAGS" >> build.sh
	chmod +x build.sh
	log_command make all "$MAKE_FLAGS"
}

# Build using a dedicated command
#	BUILD_CMD	command to launch
function build_cmd {
	log_command $BUILD_CMD
}


########### install_XXX ##########

# Do the installation of the current module
# 	INSTALL	type of build to perform (autotool, make).
function do_install {
	if [ -n "$INSTALL" ]; then
		install_$INSTALL
	fi
}

# Install using make
function install_make {
	log_command make install
}

# Install using autotool
function install_autotool {
	install_make
}


########### dist_XXX ##########

# Do the distribution of the current module
# 	DIST	type of distribution to perform (autotool).
function do_dist {
	if [ -n "$DIST" ]; then
		dist_$DIST
	fi
}

# Build distribution by autotool
#	NAME			name of the module
function dist_autotool {
	if [ ! -e Makefile ]; then
		log_command ./configure $DIST_CONFIGURE_FLAGS
	fi
	do_clean
	log_command make distdir distdir=$distdir/$NAME
}

# Build distribution by copy
#	NAME		name of the module
#	MAKE_FLAGS	flags used to perform make
function dist_copy {
	save_pwd=$PWD
	cd ..
	log_command cp -RL $NAME $distdir
	cd $save_pwd
}


# Configuration
###configuration###


########### Scan arguments ###########

for arg in $*; do
	case $arg in
	--auto)
		action=auto
		;;
	--build)
		action=build
		;;
	--config=*)
		config=${arg#--config=}
		;;
	--dev)
		dev=yes
		cvs_user=$USER
		;;
	--dist)
		action=dist
		;;
	-h|--help)
		action=help
		;;
	--install)
		action=install
		;;
	--jobs=*)
		jobs=${arg#--jobs=}
		;;
	--list)
		echo "Available configurations:"
		for f in *.otawa $root/*.otawa; do
			if [ -f "$f" ]; then
				echo "	$f"
			fi
		done
		exit 0
		;;
	--mode=*)
		mode=${arg#--mode=}
		;;
	--package=*)
		package=${arg#--package=}
		;;
	--prefix=*)
		prefix=${arg#--prefix=}
		;;
	--proxy=*)
		export http_proxy=${arg#--proxy=}
		export ftp_proxy=${arg#--proxy=}
		;;
	--verbose)
		verbose=yes
		;;
	--with-*=*)
		;;
	--work=*)
		basedir=${arg#--work=}
		;;
	-*|--*)
		action=help
		error "unknown option \"$arg\"."
		;;
	*)
		modules="$modules $arg"
		;;
	esac
done


########### Actions ###########

function make_help {

	# arguments
	echo "$tool $version"
	echo "SYNTAX: build.sh [options] modules..."
	echo "  --auto: build an automatic script, including the configuraion."
	echo "  --build: download and make modules."
	echo "  --config=PATH: path to the configuration file to use."
	echo "  --dev: use development mode (identified checkout)."  
	echo "  --dist: download and generate a distribution."
	echo "  -h|--help: display this message."
	echo "  --install: download, make and install modules."
	echo "  --jobs=N: number of jobs to create to parallel compilation (default 10)"
	echo "  --list: list available configurations."
	echo "  --mode=[dev,debug,normal,prod]: select building mode"
	echo "  --package=NAME: name of the package for 'dist' building"
	echo "  --prefix=PATH: target path of the build."
	echo "  --proxy=ADDRESS:PORT: configure a proxy use."
	echo "  --with-VARIABLE=VALUE: set a variable value."
	echo "  --work=PATH: directory to build in."

	# modules
	echo "MODULES:"
	if [ "$config" = "" ] ; then
		echo "	no configuration provided."
	else
		for mod in $options; do
			echo "  $mod"
			help_$mod
		done
	fi
}

# Make a distribution build script
#	$1 configuration to use
#	$2 default action
function make_build {
	cd $curdir
	line=`grep -n "^###configuration###" "$rootscript" | cut -f 1 -d ":"`
	head -$line "$rootscript" | sed -e "s/^action=make$/action=$2/" -e "s/^config=.*/config=no/"
	if [ -n "$1" ]; then
		cat $1 | sed -e "s/^default_modules=.*/default_modules=\"$modules\"/"
	fi
	size=`wc -l < $rootscript`
	tail=`expr $size - $line - 2`
	tail -$tail $rootscript
	cd $basedir
}


# Load the module information
#	$1	Name of the module
function load_module {
	BUILD=
	CHECK=
	CLEAN=make
	CVS_MOD=
	DIST=
	DIST_CONFIGURE_FLAGS=
	DOWNLOAD=
	INSTALL=
	LIBTOOLIZE_FLAGS=
	MAKE_FLAGS=
	PATCH=
	REQUIRE=
	USE=
	SETUP=
	VERSION=
	WGET_ADDRESS=
	WGET_PACKAGE=
	WGET_DIR=
	DIST=
	mod_$1
}

# Build the requirements
#	$1	list of requirements
#	$2	list of uses
function require {
	modules="$modules $1"

	for mod in $1; do
		display "requiring $mod in [$done]"  
		if contains "$mod" $done; then
			true
		else
			action_$action $mod
		fi
	done
	
	for mod in $2; do
		if contains "$mod" $modules; then
			display "using $mod in [$modules]"  
			if contains "$mod" $done; then
				true
			else
				action_$action $mod
			fi
		fi
	done	
}


# Perform the make action
#	$1	Name of the module to make
function action_make {
	if expr "$done" : ".*$mod" > /dev/null; then
		return
	fi
	load_module $1
	if do_check; then
		return
	fi
	require "$REQUIRE" "$USE"
	load_module $1
	info "*** making $1 ***"
	do_download
	do_patch
	cd $basedir/$NAME
	do_setup
	do_build
	cd $basedir
	done="$done $1"
}

# Perform the install action
#	$1	name of the module to install
function action_install {
	action_make $1
	if [ -n "$INSTALL" ]; then
		cd $basedir/$NAME
		do_install
		cd $basedir
	fi
}

# Perform distribution building
#	$1	name of the module to install
function action_dist {
	load_module $1
	if do_check; then
		return
	fi
	require "$REQUIRE" "$USE"
	load_module $1
	info "*** making dist for $1 ***"
	do_download
	do_patch
	cd $basedir/$NAME
	do_setup
	do_dist
	cd $basedir
	done="$done $1"
}

# Perform a make for a distribution.
#	$1	name of the module to dist make
function action_distmake {
	load_module $1
	if do_check; then
		return
	fi
	require "$REQUIRE" "$USE"
	load_module $1
	info "*** making $1 ***"
	cd $basedir/$NAME
	do_build
	do_install
	cd $basedir
	done="$done $1"
}


################# Entry #####################

# Compute root path
rootscript=`absname $0`
rootdir=`dirname $0`

# Make the build base directory
if [ "$action" != distmake ]; then
	mkdir -p $basedir
	cd $basedir
fi
basedir=`pwd`
log "Build by $tool $version (" `date` ")\n" > $basedir/$log

# Select configuration
if [ "$config" != "no" ]; then
	cd $curdir
	if [ "$config" == "" ]; then
		if test -f "default.otawa"; then
			config="./default.otawa"
		elif test -f "$root/dists/default.otawa"; then
			config="$root/dists/default.otawa"
		elif [ "$action" != "help" ] ; then
			error "no default configuration available: select one with --config"
		fi
	else
		if test ! -f $config; then
			error "cannot use the configuration file $config."
		fi
	fi
	if [ "$config" != "" ] ; then
		display "INFO: configuration  = $config"
		. $config
		cd $basedir
	fi
fi

# Prefix management
if [ -z "$prefix" ]; then
	prefix=$basedir
fi
if [ "${prefix:0:1}" != "/" ]; then
	prefix="$PWD/$prefix"
fi


# options for modules
for arg in $*; do
	case $arg in
	--with-*=*)
		tmp=${arg#--with-}
		for mod in $options; do
			option_$mod "${tmp%%=*}" "${tmp#*=}"
		done
		;;
	*)
		;;
	esac
done	


# Module selection
if [ -z "$modules" ]; then
	modules="$default_modules"
fi
modules="$modules $added_modules"
display "INFO: modules = $modules"


# pre-action
case $action in
	dist)
		distdir=$curdir/$package
		log_command mkdir "$distdir"
		distscript="$distdir/build.sh"
		make_build $config distmake > $distscript
		chmod +x "$distscript"
		;;
	auto)
		make_build $config make
		exit 0
		;;
	help)
		make_help
		exit 1
esac

# build entry
for mod in $modules; do
	action_$action $mod
	true
done

# post-action
case $action in
	dist)
		cd $curdir
		log_command tar cvfz $distdir.tgz $package/
		#log_command rm -rf $distdir
		;;
esac
