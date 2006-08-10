#!/bin/bash

# Defaults
action=check
list=
benchdir=../snu-rt


# Scan arguments
while [ -n "$1" ]; do
	case "$1" in
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


# Perfom check
for bench in $list; do
	if [ ! -f $bench.out ]; then
		echo "WARNING: $bench.out not built !"
	else
		echo -n "checking $bench ... "
		
		# Prepare command line
		args=
		if [ -f $bench.arg ]; then
			args="$args `cat $bench.arg`"
		fi
		if [ -f $bench.in ]; then
			args="$args < $bench.in"
		fi
		
		# Perform the check
		if $cmd $benchdir/$bench/$bench $args > tmp.out; then
			if diff tmp.out $bench.out; then
				echo "checked"
			else
				echo "bad result"
				exit 4
			fi
		else
			echo "failure"
			exit 3
		fi

	fi
done
rm -f tmp.out
echo "SUCCESS: all has been checked successfully !"
