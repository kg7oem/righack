#!/usr/bin/env bash

set -e
set -x

BUILD_DIR="$1"
SRC_DIR="$(pwd)/src"

if [ "$BUILD_DIR" = '' ]; then
    echo "Usage: $(basename $0) <directory to build in>" >&2
    exit 1
fi

if [ -e "$BUILD_DIR" ]; then
    rm -fr "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR"

cmake "$SRC_DIR"
make
