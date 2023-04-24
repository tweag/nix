#ifndef NIX_VALUE_API_H
#define NIX_VALUE_API_H
#include "stdbool.h"
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

// Function prototypes

ValueType nix_get_type(const Value* value);
const char* nix_get_typename(const Value* value);
Value* nix_alloc_value(State* state);

bool nix_get_bool(const Value* value);
const char* nix_get_string(const Value* value);
unsigned int nix_get_list_size(const Value* value);
unsigned int nix_get_attrs_size(const Value* value);
double nix_get_double(const Value* value);
int64_t nix_get_int(const Value* value);

Value* nix_get_list_byid(const Value* value, unsigned int ix);
Value* nix_get_attr_byname(const Value* value, State* state, const char* name);
bool nix_has_attr_byname(const Value* value, State* state, const char* name);
Value* nix_get_attr_iterate(const Value* value, const State* state, void (*iter)(const char*, Value*, void*), void* data);

void nix_set_bool(Value* value, bool b);
void nix_set_string(Value* value, const char* str);
void nix_set_double(Value* value, double d);
void nix_set_int(Value* value, int64_t i);
void nix_set_null(Value* value);
void nix_make_list(State* s, Value* value, unsigned int size);
void nix_set_list_byid(Value* value, unsigned int ix, Value* elem);
void nix_make_attrs(Value* value, BindingsBuilder* b);

// owned ref
BindingsBuilder* nix_make_bindings_builder(State* state, size_t capacity);
void nix_bindings_builder_insert(BindingsBuilder* b, const char* name, Value* value);
void nix_bindings_builder_unref(BindingsBuilder*);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_VALUE_API_H
