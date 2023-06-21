#ifndef NIX_VALUE_API_H
#define NIX_VALUE_API_H

#include "nix_api_util.h"
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

// Type definitions
typedef enum {
    NIX_TYPE_THUNK,
    NIX_TYPE_INT,
    NIX_TYPE_FLOAT,
    NIX_TYPE_BOOL,
    NIX_TYPE_STRING,
    NIX_TYPE_PATH,
    NIX_TYPE_NULL,
    NIX_TYPE_ATTRS,
    NIX_TYPE_LIST,
    NIX_TYPE_FUNCTION,
    NIX_TYPE_EXTERNAL
} ValueType;

typedef void Value;
typedef void BindingsBuilder;
typedef struct State State;
typedef struct GCRef GCRef;
typedef struct PrimOp PrimOp;

typedef void (* PrimOpFun) (State*, int pos, Value ** args, Value* v);

PrimOp* nix_alloc_primop(nix_c_context*, PrimOpFun fun, int arity, const char* name, const char** args, const char* doc, GCRef* ref);
void nix_gc_register_finalizer(void* obj, void* cd, void (*finalizer)(void* obj, void* cd));

// Function prototypes

Value* nix_alloc_value(nix_c_context*, State* state, GCRef* ref);
ValueType nix_get_type(nix_c_context*, nix_err*, const Value* value);
const char* nix_get_typename(nix_c_context*, const Value* value);

bool nix_get_bool(nix_c_context*, nix_err*, const Value* value);
const char* nix_get_string(nix_c_context*, const Value* value);
const char* nix_get_path_string(nix_c_context*, const Value* value);
unsigned int nix_get_list_size(nix_c_context*, nix_err*, const Value* value);
unsigned int nix_get_attrs_size(nix_c_context*, nix_err*, const Value* value);
double nix_get_double(nix_c_context*, nix_err*, const Value* value);
int64_t nix_get_int(nix_c_context*, nix_err*, const Value* value);

Value* nix_get_list_byidx(nix_c_context*, const Value* value, unsigned int ix, GCRef* ref);
Value* nix_get_attr_byname(nix_c_context*, const Value* value, State* state, const char* name, GCRef* ref);

bool nix_has_attr_byname(nix_c_context*, nix_err*, const Value* value, State* state, const char* name);

Value* nix_get_attr_byidx(nix_c_context*, const Value* value, State* state, unsigned int i, const char** name, GCRef* ref);

nix_err nix_set_bool(nix_c_context*, Value* value, bool b);
nix_err nix_set_string(nix_c_context*, Value* value, const char* str);
nix_err nix_set_path_string(nix_c_context*, Value* value, const char* str);
nix_err nix_set_double(nix_c_context*, Value* value, double d);
nix_err nix_set_int(nix_c_context*, Value* value, int64_t i);
nix_err nix_set_null(nix_c_context*, Value* value);
nix_err nix_make_list(nix_c_context*, State* s, Value* value, unsigned int size);
nix_err nix_set_list_byidx(nix_c_context*, Value* value, unsigned int ix, Value* elem);
nix_err nix_make_attrs(nix_c_context*, Value* value, BindingsBuilder* b);
nix_err nix_set_primop(nix_c_context*, Value* value, PrimOp* op);
nix_err nix_copy_value(nix_c_context*, Value* value, Value* source);

// owned ref
BindingsBuilder* nix_make_bindings_builder(nix_c_context*, State* state, size_t capacity);
nix_err nix_bindings_builder_insert(nix_c_context*, BindingsBuilder* b, const char* name, Value* value);
void nix_bindings_builder_unref(BindingsBuilder*);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_VALUE_API_H
