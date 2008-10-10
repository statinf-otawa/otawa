#!/bin/bash

if [ "$1" = "" ]; then
	echo "SYNTAX: compare_all directory"
	echo "	compare all programs from a directory"
fi

for d in `ls $1`; do
	echo "processing $1/$d/$d.elf"
	./compare.sh $1/$d/$d.elf || exit 1
done

