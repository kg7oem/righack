#!/usr/bin/env bash

set -e
set -x

KERNEL="$(uname -s)"

if [ "$KERNEL" = 'Linux' ]; then
    exec "$(dirname $0)/build-linux.sh" "$@"
else
    echo "I don't know how to build for $KERNEL" >&2
    exit 1
fi

echo "Should never get here" >&2
exit 1
