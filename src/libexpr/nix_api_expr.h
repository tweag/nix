#ifndef NIX_API_EXPR_H
#define NIX_API_EXPR_H
/** @file
 * @brief Main entry for the libexpr C bindings
 */

#include "nix_api_store.h"
#include "nix_api_util.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

// Type definitions
/**
 * @brief Represents a parsed nix Expression, can be evaluated into a Value.
 *
 * Owned by the GC.
 */
typedef void Expr; // nix::Expr
/**
 * @brief Represents a nix evaluator state.
 *
 * Multiple can be created for multi-threaded
 * operation.
 */
typedef struct State State; // nix::EvalState
/**
 * @brief Represents a nix value.
 *
 * Owned by the GC.
 */
typedef void Value; // nix::Value
/**
 * @brief Reference for the GC
 *
 * Nix uses a garbage collector that may not be able to see into
 * your stack and heap. Keep GCRef objects around for every
 * garbage-collected object that you want to keep alive.
 */
typedef struct GCRef GCRef; // void*

// Function propotypes
/**
 * @brief Initializes the Nix expression evaluator.
 *
 * This function should be called before creating a State.
 * This function can be called multiple times.
 *
 * @param[out] context Optional, stores error information
 * @return NIX_OK if the initialization was successful, an error code otherwise.
 */
nix_err nix_libexpr_init(nix_c_context *context);

/**
 * @brief Parses a Nix expression from a string.
 *
 * The returned expression is owned by the garbage collector.
 * Pass a gcref to keep a reference.
 *
 * @param[out] context Optional, stores error information
 * @param[in] state Evaluator state.
 * @param[in] expr The Nix expression to parse.
 * @param[in] path The file path to associate with the expression.
 * @param[out] ref Optional, will store a reference to the returned value.
 * @return A parsed expression or NULL on failure.
 */
Expr *nix_parse_expr_from_string(nix_c_context *context, State *state,
                                 const char *expr, const char *path,
                                 GCRef *ref);

/**
 * @brief Evaluates a parsed Nix expression.
 *
 * @param[out] context Optional, stores error information
 * @param[in] state The state of the evaluation.
 * @param[in] expr The Nix expression to evaluate.
 * @param[in] value The result of the evaluation.
 * @return NIX_OK if the evaluation was successful, an error code otherwise.
 */
nix_err nix_expr_eval(nix_c_context *context, State *state, Expr *expr,
                      Value *value);

/**
 * @brief Calls a Nix function with an argument.
 *
 * @param[out] context Optional, stores error information
 * @param[in] state The state of the evaluation.
 * @param[in] fn The Nix function to call.
 * @param[in] arg The argument to pass to the function.
 * @param[out] value The result of the function call.
 * @return NIX_OK if the function call was successful, an error code otherwise.
 */
nix_err nix_value_call(nix_c_context *context, State *state, Value *fn,
                       Value *arg, Value *value);

/**
 * @brief Forces the evaluation of a Nix value.
 *
 * @param[out] context Optional, stores error information
 * @param[in] state The state of the evaluation.
 * @param[in,out] value The Nix value to force.
 * @return NIX_OK if the force operation was successful, an error code
 * otherwise.
 */
nix_err nix_value_force(nix_c_context *context, State *state, Value *value);

/**
 * @brief Forces the deep evaluation of a Nix value.
 *
 * @param[out] context Optional, stores error information
 * @param[in] state The state of the evaluation.
 * @param[in,out] value The Nix value to force.
 * @return NIX_OK if the deep force operation was successful, an error code
 * otherwise.
 */
nix_err nix_value_force_deep(nix_c_context *context, State *state,
                             Value *value);

/**
 * @brief Creates a new Nix state.
 *
 * @param[out] context Optional, stores error information
 * @param[in] searchPath The NIX_PATH.
 * @param[in] store The Nix store to use.
 * @return A new Nix state or NULL on failure.
 */
State *nix_state_create(nix_c_context *context, const char **searchPath,
                        Store *store);

/**
 * @brief Frees a Nix state.
 *
 * Does not fail.
 *
 * @param[in] state The state to free.
 */
void nix_state_free(State *state);

/**
 * @brief Creates a new garbage collector reference.
 *
 * @param[out] context Optional, stores error information
 * @param[in] obj The object to create a reference for.
 * @return A new garbage collector reference or NULL on failure.
 */
GCRef *nix_gc_ref(nix_c_context *context, void *obj);

/**
 * @brief Frees a garbage collector reference.
 *
 * Does not fail.
 *
 * @param[in] ref The reference to free.
 */
void nix_gc_free(GCRef *ref);

/**
 * @brief Register a callback that gets called when the object is garbage
 * collected.
 * @note objects can only have a single finalizer. This function overwrites
 * silently.
 * @param[in] obj the object to watch
 * @param[in] cd the data to pass to the finalizer
 * @param[in] finalizer the callback function, called with obj and cd
 */
void nix_gc_register_finalizer(void *obj, void *cd,
                               void (*finalizer)(void *obj, void *cd));

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_EXPR_H
