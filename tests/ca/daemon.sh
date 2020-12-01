source common.sh

# For the post-build hook
export REMOTE_STORE_PATH=$TEST_ROOT/remote_store
export REMOTE_STORE=file://$REMOTE_STORE_PATH

nix-store --generate-binary-cache-key cache1.example.org $TEST_ROOT/sk1 $TEST_ROOT/pk1
pk1=$(cat $TEST_ROOT/pk1)

cat > "$NIX_CONF_DIR"/nix.conf.extra <<EOF
trusted-users = $(whoami)
secret-key-files = $TEST_ROOT/sk1
trusted-public-keys = $pk1
EOF

startDaemon

export NIX_REMOTE_=$NIX_REMOTE

source build.sh
source nix-copy.sh
source substitute.sh
