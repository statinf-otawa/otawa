#!/bin/bash
# $Id$
# Copyright (c) 2006-07 IRIT - UPS 

# Defaults
action=check
rebuild=no
cmd=
list=
benchdir=../snu-rt
prefix=
suffix=
verbose=


# help
function help {
	echo "SYNTAX: build-check [options] command benchs..."
	echo "        build-check [options] benchs... -- command_line"
	echo "Options:"
	echo "	--rebuild: rebuild all"
	echo "	--benchdir[=]PATH: path to the bench directory"
	echo "	--prefix=PREFIX: prefix to prepend to bench out files"
	echo "	--suffix=SUFFIX: suffix to append to bench out files"
	echo "	-h|--help: this display"
	echo "	-v|--verbose: activate the verbose mode"
	exit 1
}


# Scan arguments
while [ -n "$1" ]; do
	case "$1" in
	-h|--help)
		help
		;;
	--rebuild)
		rebuild=yes
		;;
	--benchdir)
		shift
		benchdir="$1"
		;;
	--benchdir=*)
		benchdir=${1#--benchdir=}
		;;
	--prefix=*)
		benchdir=${1#--prefix=}
		;;
	--suffix=*)
		suffix=${1#--suffix=}
		;;
	-v|--verbose)
		verbose=ok
		;;
	--)
		shift
		if [ -n "$cmd" ]; then
			list="$cmd $list"
		fi
		cmd=
		while [ -n "$1" ]; do
			cmd="$cmd $1"
			shift
		done
		;;
	-*)
		echo "ERROR: unknown option $1"
		help
		;;
	*)
		if [ -z "$cmd" ]; then
			cmd=$1
		else
			list="$list $1"
		fi
		;;
	esac
	shift
done
if [ -z "$cmd" ]; then
	echo "ERROR: give at least the test program to call !"
	help
fi


# Prepare the list
if [ -z "$list" ]; then
	for dir in $benchdir/*; do
		if [ -d $dir ]; then
			list="$list ${dir##*/}"
		fi
	done
fi
if [ x$verbose = xok ]; then
	echo "benchs = '$list'"
fi


# Perform the work
for bench in $list; do
	file="$prefix$bench$suffix.out"
	if [ "$rebuild" = yes -o ! -f "$file" ]; then
		md5sum $benchdir/$bench/$bench > $file || exit 3
		echo "$cmd $benchdir/$bench/$bench >> $file"
		$cmd $benchdir/$bench/$bench >> $file || exit 2
	fi
done
echo "SUCCESS: all is done !"
