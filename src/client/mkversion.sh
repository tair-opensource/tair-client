#!/bin/sh

set -ex

rm -f version.hpp || True
VERSION=`grep Version ../../rpm/rdb-client.spec | head -n1 | awk '{print $2}'`
DATETIME=`date "+%Y-%m-%d %H:%M:%S"`

echo "#pragma once" >> version.hpp
echo "#define TAIR_CLIENT_VERSION \"${VERSION}\"" >> version.hpp
echo "#define TAIR_CLIENT_BUILDTIME \"${DATETIME}\"" >> version.hpp
