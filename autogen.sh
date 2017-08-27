#!/bin/sh

set -xe

test -f configure.ac
rm -rf config.cache autom4te*.cache aclocal.m4 ltmain.sh missing compile configure

aclocal --force
autoconf
automake --foreign --include-deps --add-missing --copy
