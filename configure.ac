#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

dnl A bug in some versions of autoconf causes this to get inserted below its
dnl invocation
# as_fn_set_status STATUS
# -----------------------
# Set $? to STATUS, without forking.
as_fn_set_status ()
{
  return $1
} # as_fn_set_status


AC_PREREQ([2.61])

AC_INIT([xydiff], [1.0], [fdintino@gmail.com])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_LANG(C++)


AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CXX
AC_PROG_SED
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
AC_LTDL_DLLIB

XERCESC_TEST_PREFIX=""

AC_ARG_WITH([xercesc],
    AS_HELP_STRING([--with-xercesc=prefix], [The path where Xerces-C++ is installed @<:@default=check@:>@]), [
		AS_IF([test "x$withval" != xyes], [
			AS_IF([test "x$withval" = xno],
				[AC_MSG_ERROR([Xerces-C++ library is required.])])
			AS_IF([test "x$withval" != xno], [
				XERCESC_TEST_PREFIX="$withval"
			])			
		])
		AS_IF([test "x$withval" = xyes], [
        	PKG_CHECK_MODULES([XERCESC], [xercesc],[
        	    XERCES_TEST_PREFIX=$(echo "${XERCESC_CFLAGS}" | $SED 's%^\-I\(.*include\)%\1%')
        	],[
        	    dnl Don't throw an error
        	])
        ])
	],
	[
	    PKG_CHECK_MODULES([XERCESC], [xerces-c], [
    	    XERCES_TEST_PREFIX=$(echo "${XERCESC_CFLAGS}" | $SED 's%^\-I\(.*include\)%\1%')
  	    ], [
  	        dnl Don't throw an error
  	    ])
	])

PKG_CHECK_MODULES([XERCESC], [xercesc])

AC_SUBST(XERCESC_PREFIX)
AC_SUBST(XERCESC_INCLUDE)
AC_SUBST(XERCESC_LIB)

AC_CONFIG_FILES([Makefile src/Makefile src/include/Makefile])

AC_OUTPUT
