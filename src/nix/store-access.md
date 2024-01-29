R"(
# Description

`nix store access` provides subcommands that query and manipulate access control lists (ACLs) of store paths.
ACLs allow for granular access to the nix store: paths can be protected from all users (`nix store access protect`), and then necessary users can be granted permission to those paths (`nix store access grant`).

Under the hood, `nix store access` uses POSIX ACLs.

## Specify permissions on nix expressions
Alternatively, permissions may be specified directly in nix expressions.
This can be done either through the `permissions` attribute of `builtins.path` (to protect inputs from the file system), or through the
`__permissions` attribute of `mkDerivation` (to protect outputs, logs and the `drv` file of the derivation).

The syntax for this is the following:

```

    stdenvNoCC.mkDerivation {
      name = "example";
      exampleSource = builtins.path {
        path = /tmp/bar;
        permissions = {
          protected = true;
          users = ["root" "test"];
          groups = ["root"];
        };
      };
      buildCommand = "echo Example > $out; cat $exampleSource >> $out";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["root" "test"]; };
        drv = { protected = true; users = [ "root" "test" ]; groups = ["root"]; };
        log.protected = false;
      };

```
)"
