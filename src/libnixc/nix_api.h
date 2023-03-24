#ifndef NIX_API_H
#define NIX_API_H

#ifdef __cplusplus
extern "C" {
#endif
// cffi start
// Error codes
#define NIX_OK 0
#define NIX_ERR_UNKNOWN -1

// Type definitions
typedef int nix_err;
typedef struct Expr Expr;
typedef struct State State;
typedef struct StaticEnv StaticEnv;
typedef struct Env Env;
typedef struct Value Value;
typedef struct Store Store;
typedef struct GCRef GCRef;

// Function prototypes
nix_err nix_setting_get(const char* key, char* value, int n);
nix_err nix_setting_set(const char* key, const char* value);
const char* nix_version_get(); // borrowed static
nix_err nix_init();

Expr* nix_parse_expr_from_string(State* state, const char* expr, const char* path, StaticEnv* env);
nix_err nix_expr_eval(State* state, Env* env, Value* value);
nix_err nix_value_force_deep(State* state, Value* value);

Store* nix_store_open();
void nix_store_unref(Store* store);

State* nix_state_create(const char** searchPath, Store* store);
void nix_state_free(State* state);

StaticEnv* nix_state_get_base_env(State* state);
void nix_staticenv_unref(StaticEnv* env);

void nix_err_msg(char* msg, int n);

GCRef* nix_gc_ref(void* obj);
void nix_gc_free(GCRef* ref);
// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_H
