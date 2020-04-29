#!/bin/sh

ROOT_DIR="$(cd "$(dirname "$0")/../.." >/dev/null 2>&1 && pwd)"

pushd $ROOT_DIR

GIT_COMMON_DIR="$(git rev-parse --git-common-dir)"
if [ -n "$MSYSTEM" ]; then
    export MSYS=winsymlinks:nativestrict
    GIT_COMMON_DIR="$(cygpath -u "$GIT_COMMON_DIR")"
fi
ln -fv "$(realpath Extras/Hooks/pre-commit.sh)" "$GIT_COMMON_DIR/hooks/pre-commit"

popd

