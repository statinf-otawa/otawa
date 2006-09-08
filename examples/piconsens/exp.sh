#!/bin/bash
file="$1.res"
opts="$2"

echo -e "OPTIONS: $2\n" > $file

for d in ~casse/Benchs/snu-rt/*; do
	if [ -d "$d" ]; then
		echo "PROCESSING $d" >> $file
		(./piconsens $opts $d/${d##*/}) >> $file 2>> $file
		echo >> $file
	fi
done

rm -f core.*
