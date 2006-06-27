define(push, [
	old_$1=${$1}
	$1=$2
])


define(pop, [
	$1=${old_$1}
])


dnl check_header($1: config, $2: header, $3: paths, $4: cflags)
define(check_header, [
	if test "$check" != no; then
		check=no
		for path in $3; do
			push(CXXFLAGS, "-c -I$path $4 $CXXFLAGS")
			AC_COMPILE_IFELSE([
				#include <$2>
			], [check=yes])
			pop(CXXFLAGS)
			if test "$check" = yes; then
				if test ! `echo $DEFAULT_HEADER_PATHS | grep $path` \
				-a ! `echo $1 | grep "\\-I$path"`; then
					$1="-I$path $$1"
				fi
				break
			fi
		done	
	fi
])


dnl check_lib($1: config, $2: lib, $3: code, $4: paths, $5: cflags, $6: ldadd)
define(check_lib, [
	if test "$check" != no; then
		check=no
		for path in $4; do
			push(CXXFLAGS, "$5 $CXXFLAGS")
			push(LIBS, "-L$path -l$2 $6 $LIBS")
			AC_COMPILE_IFELSE($3, [check=yes])
			pop(LIBS)
			pop(CXXFLAGS)
			if test "$check" = yes; then
				if test ! `echo $DEFAULT_LIB_PATHS | grep $path` \
				-a ! `echo $1 | grep "\\-L$path"`; then
					$1="-L$path $$1"
				fi
				$1="$$1 -l$2"
				break
			fi
		done
	fi
])

dnl check_libtool($1: config, $2: lib, $3: code, $4: paths, $5: cflags, $6: ldadd)
define(check_libtool, [
	if test "$check" != no; then
		check=no
		for path in $4; do
			push(CXXFLAGS, "$5 $CXXFLAGS")
			push(LIBS, "$path/lib$2.la $6 $LIBS")
			AC_COMPILE_IFELSE($3, [check=yes])
			pop(LIBS)
			pop(CXXFLAGS)
			if test "$check" = yes; then
				$1="$$1 $path/lib$2.la"
				break
			fi
		done
	fi
])
