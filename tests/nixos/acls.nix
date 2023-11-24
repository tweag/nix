{ lib, config, nixpkgs, ... }:

let
  pkgs = config.nodes.machine.nixpkgs.pkgs;

  example-package = builtins.toFile "example.nix" ''
    with import <nixpkgs> {};
    stdenvNoCC.mkDerivation {
      name = "example";
      # Check that importing a source works
      exampleSource = builtins.path {
        path = /tmp/bar;
        permissions = {
          protected = true;
          # TODO remove the "test" user once the example-package-diff-permissions tests succeeds without it.
          users = ["root" "test"];
        };
      };
      buildCommand = "echo Example > $out; cat $exampleSource >> $out";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["root" "test"]; };
        drv = { protected = true; users = ["root" "test"]; groups = ["root"]; };
        log.protected = false;
      };
    }
  '';
  example-package-diff-permissions = builtins.toFile "example-diff-permissions.nix" ''
    with import <nixpkgs> {};
    stdenvNoCC.mkDerivation {
      name = "example";
      # Check that importing a source works
      exampleSource = builtins.path {
        path = /tmp/bar;
        permissions = {
          protected = true;
          users = ["root" "test"];
        };
      };
      buildCommand = "echo Example > $out; cat $exampleSource >> $out";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["root" "test"]; };
        drv = { protected = true; users = [ "root" "test" ]; groups = ["root"]; };
        log.protected = false;
      };
    }
  '';

  example-dependencies = builtins.toFile "example-dependencies.nix" ''
    with import <nixpkgs> {};
    let
      # Check that depending on an already existing but protected package works
      example-package =
        stdenvNoCC.mkDerivation {
          name = "example";
          # Check that importing a source works
          exampleSource = builtins.path {
            path = /tmp/bar;
            permissions = {
              protected = true;
              users = ["root" "test"];
            };
          };
          buildCommand = "echo Example > $out; cat $exampleSource >> $out";
          allowSubstitutes = false;
          __permissions = {
            outputs.out = { protected = true; users = ["root" "test"]; };

            # At the moment, non trusted user must set permissions which are a superset of existing ones.
            # If some other user adds some permission, this one will become incorrect.
            # Could we declare permissions to add instead of declaring them all ?

            drv = { protected = true; users = ["test" "root"]; groups = ["root"]; };
            log.protected = false;
          };
        };
      example2-package =
        stdenvNoCC.mkDerivation {
          name = "example2";
          buildCommand = "echo Example2 > $out";
          allowSubstitutes = false;
          __permissions = {
            outputs.out = { protected = true; users = ["root" "test"]; };
            drv = { protected = true; users = [ "test" ]; groups = [ "root" ]; };
            log.protected = false;
          };
        }
      ;
      # Check that depending on a new protected package works
      package =
        stdenvNoCC.mkDerivation {
          name = "example3";
          examplePackage = example-package.out;
          exampleSource = example-package.exampleSource;
          examplePackageOther = example2-package;
          buildCommand = "cat $examplePackage $examplePackageOther $exampleSource > $out";
        }
      ;
    in package
  '';

  testInit = ''
    # fmt: off
    import json
    start_all()

    def info(path):
      return json.loads(
        machine.succeed(f"""
          nix store access info --json {path}
        """.strip())
      )

    def assert_info(path, expected, when):
      got = info(path)
      assert(got == expected),f"Path info {got} is not as expected {expected} for path {path} {when}"
  '';

 testCli =''
    # fmt: off
    path = machine.succeed(r"""
      nix-build -E '(with import <nixpkgs> {}; runCommand "foo" {} "
        touch $out
      ")'
    """.strip())

    machine.succeed("touch /tmp/bar; chmod 777 /tmp/bar")

    assert_info(path, {"exists": True, "protected": False, "users": [], "groups": []}, "for an empty path")

    machine.succeed(f"""
      nix store access protect {path}
    """)

    assert_info(path, {"exists": True, "protected": True, "users": [], "groups": []}, "after nix store access protect")

    machine.succeed(f"""
      nix store access grant --user root {path}
    """)

    assert_info(path, {"exists": True, "protected": True, "users": ["root"], "groups": []}, "after nix store access grant")

    machine.succeed(f"""
     nix store access grant --group wheel {path}
    """)

    assert_info(path, {"exists": True, "protected": True, "users": ["root"], "groups": ["wheel"]}, "after nix store access grant")

    machine.succeed(f"""
     nix store access revoke --user root --group wheel {path}
    """)

    assert_info(path, {"exists": True, "protected": True, "users": [], "groups": []}, "after nix store access revoke")

    machine.succeed(f"""
      nix store access unprotect {path}
    """)

    assert_info(path, {"exists": True, "protected": False, "users": [], "groups": []}, "after nix store access unprotect")
  '';
  testFoo = ''
    # fmt: off
    machine.succeed("touch foo")

    fooPath = machine.succeed("""
     nix store add-file --protect ./foo
    """).strip()

    assert_info(fooPath, {"exists": True, "protected": True, "users": ["root"], "groups": []}, "after nix store add-file --protect")
'';
  testExamples = ''
    # fmt: off
    examplePackageDrvPath = machine.succeed("""
      nix eval -f ${example-package} --apply "x: x.drvPath" --raw
    """).strip()

    # TODO: uncomment when the test user is removed from the permissions of the example-package derivation.
    # assert_info(examplePackageDrvPath, {"exists": True, "protected": True, "users": [], "groups": ["root"]}, "after nix eval with __permissions")

    examplePackagePath = machine.succeed("""
      nix-build ${example-package}
    """).strip()

    # TODO: uncomment when the test user is removed from the permissions of the example-package derivation.
    # assert_info(examplePackagePath, {"exists": True, "protected": True, "users": ["root"], "groups": []}, "after nix-build with __permissions")

    examplePackagePathDiffPermissions = machine.succeed("""
      sudo -u test nix-build ${example-package-diff-permissions} --no-out-link
    """).strip()

    assert_info(examplePackagePathDiffPermissions, {"exists": True, "protected": True, "users": ["root", "test"], "groups": []}, "after nix-build as a different user")

    assert(examplePackagePath == examplePackagePathDiffPermissions), "Derivation outputs differ when __permissions change"

    # TODO: a bug currently prevents the permissions to be added back after revoking them: uncomment when this is fixed.
    # machine.succeed(f"""
    #   nix store access revoke --user test {examplePackagePath}
    # """)

    # assert_info(examplePackagePath, {"exists": True, "protected": True, "users": ["root"], "groups": []}, "after nix store access revoke")

    exampleDependenciesPackagePath = machine.succeed("""
      sudo -u test nix-build ${example-dependencies} --no-out-link --show-trace
    """).strip()

    assert_info(exampleDependenciesPackagePath, {"exists": True, "protected": False, "users": [], "groups": []}, "after nix-build with dependencies")
    assert_info(examplePackagePath, {"exists": True, "protected": True, "users": ["root", "test"], "groups": []}, "after nix-build with dependencies")

 '';

  runtime_dep_no_perm = builtins.toFile "runtime_dep_no_perm.nix" ''
    with import <nixpkgs> {};
    stdenvNoCC.mkDerivation {
      name = "example";
      # Check that importing a source works
      exampleSource = builtins.path {
        path = /tmp/dummy;
        permissions = {
          protected = true;
          users = [];
        };
      };
      buildCommand = "echo Example > $out; cat $exampleSource >> $out";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["test"]; };
        drv = { protected = true; users = ["test"]; };
        log.protected = false;
      };
    }
  '';

 testRuntimeDepNoPermScript = ''
    # fmt: off
    machine.succeed("sudo -u test touch /tmp/dummy")
    output_file = machine.fail("""
      sudo -u test nix-build ${runtime_dep_no_perm} --no-out-link
    """)
 '';

  # A private package only root can access
  private-package = builtins.toFile "private.nix" ''
    with import <nixpkgs> {};
    stdenvNoCC.mkDerivation {
      name = "private";
      privateSource = builtins.path {
        path = /tmp/secret;
        permissions = {
          protected = true;
          users = ["root"];
        };
      };
      buildCommand = "cat $privateSource > $out";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["root"]; };
        drv = { protected = true; users = ["root"]; groups = ["root"]; };
        log.protected = true;
        log.users = ["root"];
      };
    }
  '';

  # Test depending on a private output, which should fail.
  depend-on-private = builtins.toFile "depend_on_private.nix" ''
    with import <nixpkgs> {};
    let private = import ${private-package}; in
    stdenvNoCC.mkDerivation {
      name = "public";
      buildCommand = "cat ''${private} > $out ";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["test"]; };
        drv = { protected = true; users = ["test"]; };
        log.protected = true;
      };
    }
  '';

  # Test adding a private runtime dependency, which should fail.
  runtime-depend-on-private = builtins.toFile "depend_on_private.nix" ''
    with import <nixpkgs> {};
    let private = import ${private-package}; in
    stdenvNoCC.mkDerivation {
      name = "public";
      buildCommand = "echo ''${private} > $out ";
      allowSubstitutes = false;
      __permissions = {
        outputs.out = { protected = true; users = ["test"]; };
        drv = { protected = true; users = ["test"]; };
        log.protected = true;
      };
    }
  '';


  # Only root can access /tmp/secret and the output of the private-package.
  # The `test` user cannot read it nor depend on it in a derivation
  testDependOnPrivate = ''
    # fmt: off
    machine.succeed("""echo "secret_string" > /tmp/secret""");

    private_output = machine.succeed("""
      sudo nix-build ${private-package} --no-out-link
    """)

    machine.succeed(f"""cat {private_output}""")

    machine.fail(f"""sudo -u test cat {private_output}""")

    machine.fail("""
      sudo -u test nix-build ${depend-on-private} --no-out-link
    """)

    machine.fail("""
      sudo -u test nix-build ${runtime-depend-on-private} --no-out-link
    """)

  '';
in
{
  name = "acls";

  nodes.machine =
    { config, lib, pkgs, ... }:
    { virtualisation.writableStore = true;
      nix.settings.substituters = lib.mkForce [ ];
      nix.settings.experimental-features = lib.mkForce [ "nix-command" "acls" ];
      nix.nixPath = [ "nixpkgs=${lib.cleanSource pkgs.path}" ];
      virtualisation.additionalPaths = [ pkgs.stdenvNoCC pkgs.pkgsi686Linux.stdenvNoCC ];
      users.users.test = {
        isNormalUser = true;
      };
    };

  testScript = { nodes }: testInit + lib.strings.concatStrings
    [
      testCli
      testFoo
      testExamples
      testDependOnPrivate
      # [TODO] uncomment once access to the runtime closure is unforced
      # testRuntimeDepNoPermScript
    ];
}
