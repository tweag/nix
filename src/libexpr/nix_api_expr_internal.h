#ifndef NIX_API_EXPR_INTERNAL_H
#define NIX_API_EXPR_INTERNAL_H

namespace nix {
class EvalState;
};

struct State {
  nix::EvalState state;
};

struct GCRef {
  void *ptr;
};

#endif // NIX_API_EXPR_INTERNAL_H
