AC_DEFUN([XERCESC_CHECK_HEADER],[
	SEARCH_FOR=$1
	EXTRA_SEARCH_PATH=$2
	AS_IF([test -z $EXTRA_SEARCH_PATH], [], [
		EXTRA_SEARCH_PATH="$EXTRA_SEARCH_PATH "
	])
	XERCESC_SEARCH_PATH="${EXTRA_SEARCH_PATH}/usr /usr/local /opt/local"
	dnl AS_ECHO(["XERCESC_SEARCH_PATH=${XERCESC_SEARCH_PATH}"])
	AC_MSG_CHECKING(["for $1"])
	#m4_foreach_w([path], [$XERCESC_SEARCH_PATH], [
	for path in $XERCESC_SEARCH_PATH; do	
		dnl AS_ECHO(["Checking $path for $SEARCH_FOR"])
		AS_IF([test -z "$XERCESC_PREFIX"], [
			AS_IF([test -f $path/include/xercesc/$SEARCH_FOR], [
				XERCESC_PREFIX="$path"
				XERCESC_INCLUDE="-I${path}/include "
				XERCESC_CFLAGS="-I${path}/include "
				XERCESC_LIBS="${path}/lib/libxerces-c.a"
				AC_MSG_RESULT(found in $path)
			])
		])
	# ])
	done

	AS_IF([test -z "$XERCESC_PREFIX"], [
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([Xerces-C++ include directory could not be found])
	])

])

AC_DEFUN([CHECK_FOR_XERCESC], [
    dnl echo "1=$1"
    dnl echo "2=$2"
    dnl EXTRA_SEARCH_PATH=$1
    XERCESC_CHECK_HEADER(dom/DOM.hpp, [$1])
])