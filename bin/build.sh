#!/bin/bash
# $Id$

# Compute root path
if [ "$root" == "" ]; then
	root=`dirname $0`
fi

# Initial configuration
tool=OBuild
version=0.4
basedir=otawa
verbose=yes
log=build.log
config=
dev=
action=make
cvs_user=anonymous
dist_flags=
with_so=
done=
checked=
build_script=test.sh
making_script=


# functions
function display {
	if test "$verbose" = yes; then
		echo -e $*
	fi
}

function error {
	echo -e "ERROR:$*"
	exit
}

function say {
	if test "$verbose" = yes; then
		echo -n $* "..."
	fi
	say="$*"
}

function success {
	if test "$verbose" = yes; then
		echo "[DONE]"
	fi
	log "$say... [DONE]"
}

function failed {
	if test "$verbose" = yes; then
		echo "[FAILED]"
		if [ -n "$*" ]; then
			echo "$*"
		fi
	fi
	log "$say... [FAILED]"
	if [ -n "$*" ]; then
		log "$*"
	else
		tail $basedir/$log
	fi
	exit
}

function log {
	echo -e "$*\n" >> $basedir/$log
}

function log_command {
	if [ "$making_script" == yes ]; then
		echo $* '|| exit 1' >> $build_script
	else
		say "$*"
		echo "$*" | bash >> $basedir/$log 2>&1 || failed
		success
	fi
}


############# Downloads #################

function download_home {
	if [ -z "$CVS_MOD" ]; then
		CVS_MOD=$NAME
	fi
	FLAGS=
	if [ -n "$VERSION" ]; then
		FLAGS="$FLAGS -r $VERSION"
	fi
	log_command cvs -d ":pserver:$cvs_user@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA" co $FLAGS $CVS_MOD
}

function download_cvs {
	log_command cvs -d $CVS_ROOT co $CVS_MOD
}

function download_wget {
	log_command wget $WGET_ADDRESS/$WGET_PACKAGE
	package=${WGET_PACKAGE%.tgz}
	if test "$package" != $WGET_PACKAGE; then
		log_command tar xvfz $WGET_PACKAGE
	else
		package=${WGET_PACKAGE%.tar.gz}
		if test "$package" != $WGET_PACKAGE; then
			log_command tar xvfz $WGET_PACKAGE
		else
			error "Unsupported archive"
		fi
	fi
	if test "$mod" != "$package"; then
		log_command ln -s $package $mod
	fi
	rm -rf $WGET_PACKAGE
}


########## update_XXX ############

function update_home {
	log_command cvs -d ":pserver:$cvs_user@cvs.irit.fr:/usr/local/CVS_IRIT/CVS_OTAWA" update
}

function update_cvs {
	log_command cvs -d $CVS_ROOT update
}

function update_wget {
	_x=
}


########## setup_XXX ############

function setup_autotool {
	if [ ! -e configure ]; then
		log_command aclocal
		log_command autoheader
		log_command automake --add-missing
		log_command autoconf
	fi
}

function setup_libtool {
	if [ ! -e configure ]; then
		log_command aclocal
		log_command autoheader
		log_command libtoolize $LIBTOOLIZE_FLAGS
		log_command automake --add-missing
		log_command autoconf
	fi
}

function setup_bootstrap {
	if [ ! -e configure ]; then
		log_command ./bootstrap
	fi
}


########## build_XXX ############

function build_autotool {
	if [ ! -e Makefile ]; then
		args="--prefix=$prefix"
		if [ "$with_so" == yes ]; then
			args="$args --enable-shared"
		fi
		log_command ./configure  $args
	fi
	log_command make all $MAKE_FLAGS
}


function build_make {
	echo "make all $MAKE_FLAGS"
	log_command make all "$MAKE_FLAGS"
}


########### install_XXX ##########

function install_make {
	log_command make install
}

function install_autotool {
	install_make
}


########### distclean_XXX ##########

function distclean_autotool {
	log_command make distclean
}

function distclean_make {
	log_command make distclean
}

function distclean_clean {
	log_command make clean
}


########### Module macros ##########

function mod_elm {
	NAME=elm
	DOWNLOAD=home
	SETUP=bootstrap
	LIBTOOLIZE_FLAGS=--ltdl
	BUILD=autotool
	INSTALL=autotool
	CLEAN=autotool
	CHECK="automake-1.7 autoconf-2.59 libtool-1.5.12"
}

function mod_gel {
	NAME=gel
	DOWNLOAD=home
	SETUP=bootstrap
	LIBTOOLIZE_FLAGS=--ltdl
	BUILD=autotool
	INSTALL=autotool
	DISTCLEAN=autotool
	CHECK="automake-1.7 autoconf-2.59 libtool-1.5.12"
}

function mod_frontc {
	NAME=frontc
	DOWNLOAD=wget
	WGET_ADDRESS=http://www.irit.fr/recherches/ARCHI/MARCH/frontc/
	WGET_PACKAGE=Frontc-3.2.tgz
	BUILD=make
	INSTALL=make
	DISTCLEAN=make
}

function mod_gliss {
	NAME=gliss
	DOWNLOAD=home
	BUILD=make
	DISTCLEAN=make
}

function mod_ppc {
	NAME=ppc
	DOWNLOAD=home
	BUILD=make
	MAKE_FLAGS='OPT=-DISS_DISASM GEP_OPTS="-a user0 int8 -a category int8"'
	REQUIRES="gliss gel"
	DISTCLEAN=make
}

function mod_lp_solve {
	NAME=lp_solve
	DOWNLOAD=wget
	WGET_ADDRESS=ftp://ftp.es.ele.tue.nl/pub/lp_solve/old_versions_which_you_probably_dont_want/
	WGET_PACKAGE=lp_solve_4.0.tar.gz
	BUILD=make
	MAKE_FLAGS=Makefile.linux
	DISTCLEAN=clean
}

function mod_otawa {
	NAME=otawa
	DOWNLOAD=home
	SETUP=bootstrap
	BUILD=autotool
	INSTALL=make
	REQUIRES="elm"
	DISTCLEAN=autotool
}


########### Useful functions ########

function check_program {
	program=${1%-*}
	version=${1#*-}
	expr "$checked" : "$program" > /dev/null && return

	#echo "$program -> $version"
	say "Checking $program for version $version..."
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
	fi
}


########### Scan arguments ###########

function help {
	echo "$tool $version"
	echo "SYNTAX: build.sh [options] modules..."
	echo "	--with-so: use shared object libraries (enable plugins)."
	echo "	--download: just download modules."
	echo "	--make: download and make modules."
	echo "	--dev: download and make modules for internal development."
	echo "	--install: download, make and install modules."
	echo "	--dist: download and generate a distribution."
	echo "	-h|--help: display this message."
	echo "	--prefix=PATH: target path of the build."
	echo "	--build=PATH: directory to build in."
	echo "	--release=NUMBER: release of the distribution."
	echo "	--proxy=ADDRESS:PORT: configure a proxy use."
	echo "	--update: update the current installation."
	echo "MODULES: elm gliss ppc lp_solve frontc otawa"
}

modules=
for arg in $*; do
	case $arg in
	--prefix=*)
		prefix=${arg#--prefix=}
		;;
	--build=*)
		basedir=${arg#--build=}
		;;
	--make)
		action=make
		;;
	--dev)
		action=dev
		cvs_user=$LOGNAME
		;;
	--download)
		action=download
		;;
	--dist)
		action=dist
		;;
	--release=*)
		action=dist
		release=${arg#--release}
		dist_flags="$dist_flags RELEASE=$release"
		;;
	--install)
		action=install
		;;
	--update)
		action=update
		;;
	--with-so)
		with_so=yes
		;;
	--proxy=*)
		export http_proxy=${arg#--proxy=}
		export ftp_proxy=${arg#--proxy=}
		;;
	-h|--help)
		help
		exit
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
if [ -z "$modules" ]; then
	modules="gliss gel ppc lp_solve frontc elm otawa"
fi
if [ $action = update ]; then
	updates="$modules"
fi
if [ -z "$prefix" ]; then
	prefix=$basedir
fi
if [ "${prefix:0:1}" != "/" ]; then
	prefix="$PWD/$prefix"
fi


# Make the build basedirectory
mkdir -p $basedir
cd $basedir
basedir=`pwd`

# Preparation
echo "Build by $tool $version (" `date` ")\n" > $log
if test -n "$config"; then
	if test -f $config; then
		. $config
	else
		error "cannot use the configuration file $config."
	fi
fi


########### Process a module ###########
function process {
	expr "$done" : "$1" > /dev/null && return
	
	# Perfom requirements first
	REQUIRES=
	mod_$1
	for m in $REQUIRES; do
		if [ $action != update -o `expr match "$updates" ".*$m.*"` != 0 ]; then
			process $m
		else
			action=make
			process $m
			action=update
		fi
	done

	# Configure
	BUILD=
	INSTALL=
	DOWNLOAD=
	PATCH=
	MAKE_FLAGS=
	CVS_MOD=
	WGET_ADDRESS=
	WGET_PACKAGE=
	SETUP=
	LIBTOOLIZE_FLAGS=
	CHECK=
	VERSION=
	DISTCLEAN=
	mod_$1
	echo
	echo "*** Module $1 [$done] $action ***"
	done="$done $1"

	# Perform checking
	for check in $CHECK; do
		check_program $check
	done
	
	# Perform the download
	if [ -n "$DOWNLOAD" ]; then
		if [ ! -d $1 ]; then
			download_$DOWNLOAD
		else
			if [ $action = update ]; then
				cd $1
				if [ -n "$DISTCLEAN" ]; then
					distclean_$DISTCLEAN
				fi
				update_$DOWNLOAD
				cd $basedir
			fi
		fi
	fi
	
	# Requires setup and build ?
	case "$action" in
	update|make|install|dev)
		if [ -n "$SETUP" ]; then
			cd $1
			setup_$SETUP
			cd $basedir
		fi
		if [ -n "$BUILD" ]; then
			cd $1
			build_$BUILD
			cd $basedir
		fi
		;;
	esac
	
	# Perform install
	case "$action" in
	install)
		if [ -n "$INSTALL" ]; then
			cd $1
			install_$INSTALL
			cd $basedir
		fi
		;;
	esac
	
	# Perform distribution
	case "$action" in
	dist)
		old_prefix="$prefix"
		prefix='$prefix'
		making_script=yes
		echo "cd $1" >> $build_script
		if [ -n "$SETUP" ]; then
			setup_$SETUP
		fi
		if [ -n "$BUILD" ]; then
			build_$BUILD
		fi
		if [ -n "$INSTALL" ]; then
			install_$INSTALL
		fi
		echo "cd .." >> $build_script
		macking_script=
		prefix="$old_prefix"
		;;
	esac
}


# Process modules
if [ "$action" == dist ]; then
	echo '#!/bin/bash' > $build_script
	echo 'prefix=$PWD' >> $build_script
	echo 'if [ -n "$1" ]; then' >> $build_script
	echo '	prefix="$1"' >> $build_script
	echo 'fi' >> $build_script
	chmod +x $build_script
fi

done=
for mod in $modules; do
	process $mod
done

if [ "$action" == dist ]; then
	echo "Not implemented yet."
fi

