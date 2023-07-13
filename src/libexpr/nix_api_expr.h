#ifndef NIX_API_EXPR_H
#define NIX_API_EXPR_H

#include "nix_api_store.h"
#include "nix_api_util.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

// Type definitions
typedef void Expr;          // nix::Expr
typedef struct State State; // nix::EvalState
typedef void Value;         // nix::Value
typedef struct GCRef GCRef; // std::shared_ptr<void>

// Function propotypes
nix_err nix_libexpr_init(nix_c_context *);

// returns: GC'd Expr
Expr *nix_parse_expr_from_string(nix_c_context *, State *state,
                                 const char *expr, const char *path,
                                 GCRef *ref);
nix_err nix_expr_eval(nix_c_context *, State *state, Expr *expr, Value *value);
nix_err nix_value_call(nix_c_context *, State *state, Value *fn, Value *arg,
                       Value *value);
nix_err nix_value_force(nix_c_context *, State *state, Value *value);
nix_err nix_value_force_deep(nix_c_context *, State *state, Value *value);

State *nix_state_create(nix_c_context *, const char **searchPath, Store *store);
void nix_state_free(State *state);

GCRef *nix_gc_ref(nix_c_context *, void *obj);
void nix_gc_free(GCRef *ref);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_EXPR_H
