#!/bin/bash
CURRDIR=$(dirname $0)
rm -rf $CURRDIR/ar-lib \
$CURRDIR/depcomp \
$CURRDIR/configure \
$CURRDIR/config.* \
$CURRDIR/Makefile.in \
$CURRDIR/src/Makefile.in \
$CURRDIR/m4/l* \
$CURRDIR/ltmain.sh \
$CURRDIR/missing \
$CURRDIR/autom4te.cache \
$CURRDIR/install-sh \
$CURRDIR/aclocal.m4

autoreconf -i -v --force
/usr/bin/env perl -pi -e 's/(\*_cv_\*)(?!\))/$1\)/g' $CURRDIR/configure
