
echo > ccg.stat
echo > cat.stat

for f in $HOME/Benchs/standard/*; do
	if test -d $f; then
		for i in 0 1 2 3 4 5 6 7 8 9; do
			bin=`basename $f`
			path="$f/$bin"
			./ilp_times -ccg $path 2> /dev/null >> ccg.stat
			./ilp_times -cat $path 2> /dev/null >> cat.stat
		done
	fi
done
