#!/bin/bash

SRC_ROOT=$(dirname "$(dirname "$(readlink -f "$0")")")

sed -n -e '/Test suite options/,/End test suite options/p' "${SRC_ROOT}/src/lib/unittest/test_driver.c" | \
grep '#define' | \
awk '$3>x{x=$3};END{ print $3 }'
