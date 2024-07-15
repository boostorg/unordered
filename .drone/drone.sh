#!/bin/bash

# Copyright 2022 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

set -ex
export PATH=~/.local/bin:/usr/local/bin:$PATH

: ${TARGET:="libs/$LIBRARY/test"}

DRONE_BUILD_DIR=$(pwd)

BOOST_BRANCH=develop
if [ "$DRONE_BRANCH" = "master" ]; then BOOST_BRANCH=master; fi

if [[ $(uname) == "Linux" && ( "$TSAN" == 1 || "$ASAN" == 1 ) ]]; then
    echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
    sudo sysctl vm.mmap_rnd_bits=28
fi

cd ..
git clone -b $BOOST_BRANCH --depth 1 https://github.com/boostorg/boost.git boost-root
cd boost-root
git submodule update --init tools/boostdep
cp -r $DRONE_BUILD_DIR/* libs/$LIBRARY
python tools/boostdep/depinst/depinst.py $LIBRARY
./bootstrap.sh
./b2 -d0 headers

echo "using $TOOLSET : : $COMPILER ;" > ~/user-config.jam
./b2 -j3 $TARGET toolset=$TOOLSET cxxstd=$CXXSTD variant=debug,release ${ADDRMD:+address-model=$ADDRMD} ${STDLIB:+stdlib=$STDLIB} ${UBSAN:+undefined-sanitizer=norecover debug-symbols=on} ${ASAN:+address-sanitizer=norecover debug-symbols=on} ${TSAN:+thread-sanitizer=norecover debug-symbols=on} ${LINKFLAGS:+linkflags=$LINKFLAGS}
