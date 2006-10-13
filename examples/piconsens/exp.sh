#!/bin/bash

file="$1.res"
echo -e "	aucune	-D 4	-E" > $file
for d in ~casse/Benchs/snu-rt/*; do
	if [ -d "$d" ]; then
		echo -n "${d##*/}	" >> $file
		(./piconsens -p deg$1.xml $d/${d##*/}) >> $file 2>> $file
		echo -n "	" >> $file
		(./piconsens -p deg$1.xml -D 4 $d/${d##*/}) >> $file 2>> $file
		echo -n "	" >> $file
		(./piconsens -p deg$1.xml -E $d/${d##*/}) >> $file 2>> $file
		echo >> $file
	fi
done

rm -f core.*
