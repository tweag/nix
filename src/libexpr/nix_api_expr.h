#ifndef NIX_API_EXPR_H
#define NIX_API_EXPR_H

#include "nix_api_util.h"
#include "nix_api_store.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

// Type definitions
typedef void Expr;
typedef struct State State;
typedef struct Env Env;
typedef void Value;
typedef struct GCRef GCRef;

// Function propotypes

nix_err nix_libexpr_init();

// returns: GC'd Expr
Expr* nix_parse_expr_from_string(State* state, const char* expr, const char* path);
nix_err nix_expr_eval(State* state, Expr* expr, Value* value);
nix_err nix_value_call(State* state, Value* fn, Value* arg, Value* value);
nix_err nix_value_force(State* state, Value* value);
nix_err nix_value_force_deep(State* state, Value* value);


State* nix_state_create(const char** searchPath, Store* store);
void nix_state_free(State* state);

GCRef* nix_gc_ref(void* obj);
void nix_gc_free(GCRef* ref);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_STORE_H
