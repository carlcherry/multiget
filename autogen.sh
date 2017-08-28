#!/bin/sh

set -xe

test -f configure.ac
rm -rf config.cache autom4te*.cache aclocal.m4

autoheader
aclocal --force
automake --foreign --include-deps --add-missing --copy
autoconf
