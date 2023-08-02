source common.sh

clearStore

# https://github.com/NixOS/nix/issues/6572
issue_6572_independent_outputs() {
    nix build -f multiple-outputs.nix --json independent --no-link > $TEST_ROOT/independent.json

    # Make sure that 'nix build' can build a derivation that depends on both outputs of another derivation.
    p=$(nix build -f multiple-outputs.nix use-independent --no-link --print-out-paths)
    nix-store --delete "$p" # Clean up for next test

    # Make sure that 'nix build' tracks input-outputs correctly when a single output is already present.
    nix-store --delete "$(jq -r <$TEST_ROOT/independent.json .[0].outputs.first)"
    p=$(nix build -f multiple-outputs.nix use-independent --no-link --print-out-paths)
    cmp $p <<EOF
first
second
EOF
    nix-store --delete "$p" # Clean up for next test

    # Make sure that 'nix build' tracks input-outputs correctly when a single output is already present.
    nix-store --delete "$(jq -r <$TEST_ROOT/independent.json .[0].outputs.second)"
    p=$(nix build -f multiple-outputs.nix use-independent --no-link --print-out-paths)
    cmp $p <<EOF
first
second
EOF
    nix-store --delete "$p" # Clean up for next test
}
issue_6572_independent_outputs


# https://github.com/NixOS/nix/issues/6572
issue_6572_dependent_outputs() {

    nix build -f multiple-outputs.nix --json a --no-link > $TEST_ROOT/a.json

    # # Make sure that 'nix build' can build a derivation that depends on both outputs of another derivation.
    p=$(nix build -f multiple-outputs.nix use-a --no-link --print-out-paths)
    nix-store --delete "$p" # Clean up for next test

    # Make sure that 'nix build' tracks input-outputs correctly when a single output is already present.
    nix-store --delete "$(jq -r <$TEST_ROOT/a.json .[0].outputs.second)"
    p=$(nix build -f multiple-outputs.nix use-a --no-link --print-out-paths)
    cmp $p <<EOF
first
second
EOF
    nix-store --delete "$p" # Clean up for next test
}
if isDaemonNewer "2.12pre0"; then
    issue_6572_dependent_outputs
fi

nix_store_delete_best_effort() {
    clearStore
    nix build -f multiple-outputs.nix a --out-link $TEST_ROOT/a
    a_second=$(realpath $TEST_ROOT/a-second)
    a_first=$(realpath $TEST_ROOT/a-first)
    rm $TEST_ROOT/a-second
    p=$(nix build -f multiple-outputs.nix use-a --no-link --print-out-paths)

    # Check that nix store delete --recursive is best-effort (doesn't fail when some paths in the closure are alive)
    nix store delete --recursive "$p"
    [[ -e "$a_first" ]] || fail "a.first is a gc root, shouldn't have been deleted"
    expect 1 [[ -e "$a_second" ]] || fail "a.second is not a gc root and is part of use-a's closure, it should have been deleted"
    expect 1 [[ -e "$p" ]] || fail "use-a should have been deleted"
}

nix_store_delete_best_effort
