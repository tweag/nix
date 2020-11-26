#!/usr/bin/env bash

# Ensure that binary substitution works properly with ca derivations

source common.sh

export REMOTE_STORE_PATH=$TEST_ROOT/remote_store
chmod -R u+w "$REMOTE_STORE_PATH" || true
rm -rf "$REMOTE_STORE_PATH"
export REMOTE_STORE=file://$REMOTE_STORE_PATH

nix-store --generate-binary-cache-key cache1.example.org $TEST_ROOT/sk1 $TEST_ROOT/pk1
pk1=$(cat $TEST_ROOT/pk1)
sk1=$TEST_ROOT/sk1

buildDrvs () {
    nix \
        --experimental-features 'nix-command ca-derivations ca-references' \
        build \
        --file ./content-addressed.nix \
        -L --no-link \
        --secret-key-files $sk1 \
        "$@"
}

clearStore
# Populate the remote cache
buildDrvs --post-build-hook $PWD/../push-to-store.sh

# Restart the build on an empty store, ensuring that we don't build
clearStore
buildDrvs --substitute --substituters $REMOTE_STORE --no-require-sigs|& \
    (! grep -q Building)

