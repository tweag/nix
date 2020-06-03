#include "path.hh"

namespace nix {

/**
 * A symbolic reference to a derivation output (that might not have been built
 * yet)
*/
struct SymbolicOutput {
  StorePath deriver;
  string outputName;
};

}
