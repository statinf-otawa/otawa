# OTAWA_USE_LIB(var, code)
#	Use the given library
AC_DEFUN([OTAWA_USE_LIB], [
	$2
	AM_CONDITIONAL(HAS_$1, test "$HAS_$1" = yes)
	AC_SUBST($1_CXXFLAGS)
	AC_SUBST($1_LIBS)
])


# OTAWA_REQUIRE_LIB(var, message, code)
#	If the code cannot set the HAS_$var to yes, issue a message failure.
AC_DEFUN([OTAWA_REQUIRE_LIB], [
	$3
	AS_IF(test "$HAS_$1" != yes, AC_MSG_FAILURE([$2]))
])


# OTAWA_CHECK_LTLIB(var, [header], library, [path], [header path], [else])
#	Check for LibTool library.
AC_DEFUN([OTAWA_CHECK_LTLIB], [
	HAS_$1=no
	AC_MSG_CHECKING([for $3 at $4])

	# Compute paths
	AS_IF(test -z "$4" -o "$4" = default,
		[
			ipath=""
			lpath="$3"
			OLD_CXX="$CXX"
			OLD_LIBS="$LIBS"
			CXX="libtool --mode=link $CXX"
			LIBS="$LIBS $lpath"
			AC_LINK_IFELSE(
				AC_LANG_PROGRAM([[ #include <$2> ]], [[ ]]),
					[
						AC_MSG_RESULT([found])
						HAS_$1=yes
					],
					[ AC_MSG_RESULT([not found]) ]
				)
			CXX="$OLD_CXX"
			LIBS="$OLD_LIBS"
		],
		[
			AS_IF(test -z "$5",
				[
					ipath="$4/include"
					lpath="$4/lib/$2"
				],
				[
					ipath="$5"
					lpath="$4"
				])
			AS_IF(test -z "$2" -o -f "$ipath/$2",
				[ AS_IF(test -f "$lpath/$3", [
						HAS_$1=yes
						AC_MSG_RESULT([found])
					],
					[ AC_MSG_RESULT([no lib "$lpath/$3"]) ])
				],
				[ AC_MSG_RESULT([no header "$ipath/$2"]) ])
		])

	# Set variables
	AS_IF(test "$HAS_$1" = yes,
		[
			$1_CXXFLAGS="-I$ipath"
			$1_LIBS="$lpath/$3"
		],
		[
			true
			$6
		])
])


# OTAWA_CHECK_LIB(var, [header], library, [path], [header path], [else])
#	Check for the existence of a library.
AC_DEFUN([OTAWA_CHECK_LIB], [
	HAS_$1=no
	AC_MSG_CHECKING([for $3 at $4])

	# Compute paths
	AS_IF(test -z "$4" -o "$4" = default, [
			ipath=""
			lpath=""
		],
		AS_IF(test -z "$5", [
				ipath="-I$4/include"
				lpath="-L$4"
			], [
				ipath="-I$5"
				lpath="-L$4"
			]))

	# Attempt to compile
	AS_IF(test -n "$2", [ head="#include <$2>" ], [ head="" ])
	OLD_CXXFLAGS="$CXXFLAGS"
	OLD_LIBS="$LIBS"
	CXXFLAGS="$CXXFLAGS $ipath"
	LIBS="$LIBS $lpath -l$3"
	AC_LINK_IFELSE(
		AC_LANG_PROGRAM([[ $head ]], [[ ]]),
		[
			AC_MSG_RESULT([found])
			HAS_$1=yes
		],
		[ AC_MSG_RESULT([not found]) ])
	CXXFLAGS="$OLD_CXXFLAGS"
	LIBS="$OLD_LIBS"

	# Set variables
	AS_IF(test "$HAS_$1" = yes, [
		$1_CXXFLAGS="$ipath"
		$1_LIBS="$lpath -l$3"
	], [
		true
		$6
	])
])

