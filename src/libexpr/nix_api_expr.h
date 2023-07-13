#ifndef NIX_API_EXPR_H
#define NIX_API_EXPR_H

#include "nix_api_store.h"
#include "nix_api_util.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

// Type definitions
/**
 * Represents a parsed nix Expression, can be evaluated into a Value
 */
typedef void Expr; // nix::Expr
/**
 * Represents a nix evaluator state. Multiple can be created for multi-threaded
 * operation.
 */
typedef struct State State; // nix::EvalState
/**
 * Represents a nix value.
 */
typedef void Value; // nix::Value
/**
 * Nix uses a garbage collector that may not be able to see into
 * your stack and heap. Keep GCRef objects around for every
 * garbage-collected object that you want to keep alive.
 */
typedef struct GCRef GCRef; // void*

// Function propotypes
/**
 * Initializes the Nix expression evaluator.
 *
 * This function should be called before creating a State.
 * This function can be called multiple times.
 *
 * @param context Optional, stores error information
 * @return NIX_OK if the initialization was successful, an error code otherwise.
 */
nix_err nix_libexpr_init(nix_c_context *context);

/**
 * Parses a Nix expression from a string.
 *
 * The returned expression is owned by the garbage collector.
 * Pass a gcref to keep a reference.
 *
 * @param context Optional, stores error information
 * @param state Evaluator state.
 * @param expr The Nix expression to parse.
 * @param path The file path to associate with the expression.
 * @param ref Optional, will store a reference to the returned value.
 * @return A parsed expression or NULL on failure.
 */
Expr *nix_parse_expr_from_string(nix_c_context *context, State *state,
                                 const char *expr, const char *path,
                                 GCRef *ref);

/**
 * Evaluates a parsed Nix expression.
 *
 * @param context Optional, stores error information
 * @param state The state of the evaluation.
 * @param expr The Nix expression to evaluate.
 * @param value The result of the evaluation.
 * @return NIX_OK if the evaluation was successful, an error code otherwise.
 */
nix_err nix_expr_eval(nix_c_context *context, State *state, Expr *expr,
                      Value *value);

/**
 * Calls a Nix function with an argument.
 *
 * @param context Optional, stores error information
 * @param state The state of the evaluation.
 * @param fn The Nix function to call.
 * @param arg The argument to pass to the function.
 * @param value The result of the function call.
 * @return NIX_OK if the function call was successful, an error code otherwise.
 */
nix_err nix_value_call(nix_c_context *context, State *state, Value *fn,
                       Value *arg, Value *value);

/**
 * Forces the evaluation of a Nix value.
 *
 * @param context Optional, stores error information
 * @param state The state of the evaluation.
 * @param value The Nix value to force.
 * @return NIX_OK if the force operation was successful, an error code
 * otherwise.
 */
nix_err nix_value_force(nix_c_context *context, State *state, Value *value);

/**
 * Forces the deep evaluation of a Nix value.
 *
 * @param context Optional, stores error information
 * @param state The state of the evaluation.
 * @param value The Nix value to force.
 * @return NIX_OK if the deep force operation was successful, an error code
 * otherwise.
 */
nix_err nix_value_force_deep(nix_c_context *context, State *state,
                             Value *value);

/**
 * Creates a new Nix state.
 *
 * @param context Optional, stores error information
 * @param searchPath The NIX_PATH.
 * @param store The Nix store to use.
 * @return A new Nix state or NULL on failure.
 */
State *nix_state_create(nix_c_context *context, const char **searchPath,
                        Store *store);

/**
 * Frees a Nix state.
 *
 * @param state The state to free.
 */
void nix_state_free(State *state);

/**
 * Creates a new garbage collector reference.
 *
 * @param context Optional, stores error information
 * @param obj The object to create a reference for.
 * @return A new garbage collector reference or NULL on failure.
 */
GCRef *nix_gc_ref(nix_c_context *context, void *obj);

/**
 * Frees a garbage collector reference. Does not fail.
 *
 * @param ref The reference to free.
 */
void nix_gc_free(GCRef *ref);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_EXPR_H
