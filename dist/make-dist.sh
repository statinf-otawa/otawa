#!/bin/bash

# Select version
dist=default
if [ "$1" != "" ]; then
	dist="$1"
fi
path="$dist.otawa"
if [ ! -f $path ]; then
	echo "ERROR: distribution $path does not exist !"
	exit 1
fi

# Let's go
out="build-$dist.sh"
line=`grep -n "###configuration###" build.sh | cut -f 1 -d ":"`
head -$line build.sh > $out
cat $path >> $out
size=`wc -l < build.sh`
tail=`expr $size - $line - 2`
tail -$tail build.sh >> $out
chmod +x $out
