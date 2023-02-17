# Python Bindings

This directory contains experimental Python bindings to a small subset of Nix's functionality. These bindings are very fast since they link to the necessary dynamic libraries directly, without having to call the Nix CLI for every operation.

Originally these bindings were created by [@Mic92](https://github.com/Mic92) and called [Pythonix](https://github.com/Mic92/pythonix). However, after Nix changed the internal library interface multiple times, therefore requiring Pythonix to be updated, it wasn't worth the third-party time investment anymore and the Pythonix repository was archived. At that point it only worked for Nix versions below 2.4. By having these bindings upstream, they can be updated right along the C++ code that defines the library interface.

## Documentation

See [index.md](./doc/index.md), which is also rendered in the HTML manual.

To hack on these bindings, see [hacking.md](./doc/hacking.md), also rendered in the HTML manual.
