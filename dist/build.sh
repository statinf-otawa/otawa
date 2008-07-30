#!/bin/bash
tool=otawa-build-new
version=2.0

# Actions
#	dist
#	install
#	make

# Build modes
#	dev		development
#	debug	debugging
#	normal	normal working
#	prod	production mode

# Configuration
action=make
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
		if check_$CHECK; then
			done="$done $NAME"
			true
			return
		fi
	fi
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
	cversion=`$program --version | head -1 | cut -f 4 -d " "`
	major=${version%%.*}
	cmajor=${cversion%%.*}
	minor=${version#*.}
	cminor=${cversion#*.}
	if [ "$major" \> "$cmajor" -o "${minor%%.*}" \> "${cminor%%.*}"  ]; then
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
	for file in $CHECK_FILES; do
		if [ ! -e "$NAME/$file" ]; then
			false
			return
		fi
	done
	for dir in $CHECK_DIRS; do
		if [ ! -d "$NAME/$dir" ]; then
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


########### Scan arguments ###########

function help {
	echo "$tool $version"
	echo "SYNTAX: build.sh [options] modules..."
	echo "  --auto: build an automatic script, including the configuraion."
	echo "	--build: download and make modules."
	echo "  --config=PATH: path to the configuration file to use."
	echo "  --dev: use development mode (identified checkout)."  
	echo "	--dist: download and generate a distribution."
	echo "	-h|--help: display this message."
	echo "	--install: download, make and install modules."
	echo "  --jobs=N: number of jobs to create to parallel compilation (default 10)"
	echo "  --list: list available configurations."
	echo "  --mode=[dev,debug,normal,prod]: select building mode"
	echo "  --package=NAME: name of the package for 'dist' building"
	echo "	--prefix=PATH: target path of the build."
	echo "	--proxy=ADDRESS:PORT: configure a proxy use."
	echo "	--with-systemc: SystemC location."
	echo "	--work=PATH: directory to build in."
}

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
		help
		exit
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
	--with-systemc=*)
		systemc_location=${arg#--with-systemc=}
		;;
	--work=*)
		basedir=${arg#--work=}
		;;
	-*|--*)
		help
		error "unknown option \"$arg\"."
		;;
	*)
		modules="$modules $arg"
		;;
	esac
done


# Configuration
###configuration###

########### Actions ###########

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
	#DONE=
	#DONE_FILE=
	INSTALL=
	LIBTOOLIZE_FLAGS=
	MAKE_FLAGS=
	PATCH=
	REQUIRE=
	SETUP=
	VERSION=
	WGET_ADDRESS=
	WGET_PACKAGE=
	DIST=
	mod_$1
}

# Build the requirements
#	$*	list of requirements
function require {
	for mod in $*; do
		display "requiring $mod in [$done]"  
		if expr "$done" : ".*$mod" > /dev/null; then
			true
		else
			action_$action $mod
		fi
	done
}


# Perform the make action
#	$1	Name of the module to make
function action_make {
	load_module $1
	if do_check; then
		return
	fi
	require $REQUIRE
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
	require $REQUIRE
	load_module $1
	info "*** making dist for $1 ***"
	do_download
	do_patch
	cd $basedir/$NAME
	do_setup
	do_dist
	cd $basedir
}

# Perform a make for a distribution.
#	$1	name of the module to dist make
function action_distmake {
	load_module $1
	if do_check; then
		return
	fi
	require $REQUIRE
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
		else
			error "no default configuration available: select one with --config"
		fi
	else
		if test ! -f $config; then
			error "cannot use the configuration file $config."
		fi
	fi
	display "INFO: configuration = $config"
	. $config
	cd $basedir
fi

# Module selection
if [ -z "$modules" ]; then
	modules="$default_modules"
fi
display "INFO: modules = $modules"
if [ $action = update ]; then
	updates="$modules"
fi

# Prefix management
if [ -z "$prefix" ]; then
	prefix=$basedir
fi
if [ "${prefix:0:1}" != "/" ]; then
	prefix="$PWD/$prefix"
fi

# systemc
if [ ! -z $systemc_location ]; then
	rm -f systemc
	ln -s $systemc_location systemc
fi

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
