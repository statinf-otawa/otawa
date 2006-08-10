#!/bin/bash

# Defaults
action=check
rebuild=no
cmd=
list=
benchdir=../snu-rt


# Scan arguments
while [ -n "$1" ]; do
	case "$1" in
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
	-*)
		echo "ERROR: unknown option $1"
		exit 1
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
	exit 2
fi


# Prepare the list
if [ -z "$list" ]; then
	for dir in $benchdir/*; do
		if [ -d $dir ]; then
			list="$list ${dir##*/}"
		fi
	done
fi


# Perform the work
for bench in $list; do
	if [ "$rebuild" = yes -o ! -f $bench.out ]; then
		echo "$cmd $benchdir/$bench/$bench > $bench.out"
		./test_ct $benchdir/$bench/$bench > $bench.out || exit 2
	fi
done
echo "SUCCESS: all is done !"
