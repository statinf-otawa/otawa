#!/bin/bash
root=`dirname $0`

# Select version
dist=default.otawa
if [ "$1" != "" ]; then
	dist="$1"
fi
path="$dist"
if [ ! -f $path ]; then
	echo "ERROR: distribution $path does not exist !"
	exit 1
fi

# Let's go
line=`grep -n "###configuration###" build.sh | cut -f 1 -d ":"`
head -$line build.sh
cat $path
echo "config=no"
size=`wc -l < $root/build.sh`
tail=`expr $size - $line - 2`
tail -$tail build.sh
