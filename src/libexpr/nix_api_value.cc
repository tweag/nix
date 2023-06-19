#include "eval.hh"
#include "globals.hh"
#include "config.hh"
#include "value.hh"
#include "attr-set.hh"

#include "nix_api_util.h"
#include "nix_api_expr.h"
#include "nix_api_value.h"
#include "nix_api_util_internal.h"

struct State {
    nix::EvalState state;
};
// Helper function to throw an exception if value is null
static void check_value_not_null(const Value* value) {
    if (!value) {
        throw std::runtime_error("Value is null");
    }
}

Value* nix_alloc_value(State* state) {
    return state->state.allocValue();
}

ValueType nix_get_type(const Value* value) {
    check_value_not_null(value);
    using namespace nix;
    switch (((const nix::Value*)value)->type()) {
    case nThunk: return NIX_TYPE_THUNK;
    case nInt: return NIX_TYPE_INT;
    case nFloat: return NIX_TYPE_FLOAT;
    case nBool: return NIX_TYPE_BOOL;
    case nString: return NIX_TYPE_STRING;
    case nPath: return NIX_TYPE_PATH;
    case nNull: return NIX_TYPE_NULL;
    case nAttrs: return NIX_TYPE_ATTRS;
    case nList: return NIX_TYPE_LIST;
    case nFunction: return NIX_TYPE_FUNCTION;
    case nExternal: return NIX_TYPE_EXTERNAL;
    }
    return NIX_TYPE_NULL;
}

const char* nix_get_typename(const Value* value) {
    check_value_not_null(value);
    auto s = nix::showType(*(const nix::Value*)value);
    return strdup(s.c_str());
}

bool nix_get_bool(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nBool);
    return v.boolean;
}

const char* nix_get_string(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nString);
    return v.string.s;
}

unsigned int nix_get_list_size(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nList);
    return v.listSize();
}

unsigned int nix_get_attrs_size(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nAttrs);
    return v.attrs->size();
}

double nix_get_double(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nFloat);
    return v.fpoint;
}

int64_t nix_get_int(const Value* value) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nInt);
    return v.integer;
}

Value* nix_get_list_byidx(const Value* value, unsigned int ix) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nList);
    return (Value*)v.listElems()[ix];
}

Value* nix_get_attr_byname(const Value* value, State* state, const char* name) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nAttrs);
    nix::Symbol s = state->state.symbols.create(name);
    auto attr = v.attrs->get(s);
    if (attr) return attr->value;
    // todo: set_error_message
    return nullptr;
}

bool nix_has_attr_byname(const Value* value, State* state, const char* name) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nAttrs);
    nix::Symbol s = state->state.symbols.create(name);
    auto attr = v.attrs->get(s);
    if (attr) return true;
    return false;
}

Value* nix_get_attr_byidx(const Value* value, State* state, unsigned int i, const char** name) {
    const nix::Value &v = *(const nix::Value*)value;
    const nix::Attr& a = (*v.attrs)[i];
    *name = ((const std::string&)(state->state.symbols[a.name])).c_str();
    return a.value;
}

void nix_set_bool(Value* value, bool b) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    v.mkBool(b);
}

// todo string context
void nix_set_string(Value* value, const char* str) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    v.mkString(std::string_view(str));
}

void nix_set_double(Value* value, double d) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    v.mkFloat(d);
}

void nix_set_int(Value* value, int64_t i) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    v.mkInt(i);
}

void nix_set_null(Value* value) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    v.mkNull();
}

void nix_make_list(State* s, Value* value, unsigned int size) {
    check_value_not_null(value);
    nix::Value &v = *(nix::Value*)value;
    s->state.mkList(v, size);
}

void nix_set_list_byidx(Value* value, unsigned int ix, Value* elem) {
    check_value_not_null(value);
    check_value_not_null(elem);
    // todo: assert that this is a list
    nix::Value &v = *(nix::Value*)value;
    v.listElems()[ix] = (nix::Value*)elem;
}

typedef std::shared_ptr<nix::BindingsBuilder> BindingsBuilder_Inner;

void nix_make_attrs(Value* value, BindingsBuilder* b) {
    check_value_not_null(value);
    nix::BindingsBuilder& builder = **(BindingsBuilder_Inner*)b;
    nix::Value &v = *(nix::Value*)value;
    v.mkAttrs(builder);
}

BindingsBuilder* nix_make_bindings_builder(State* state, size_t capacity) {
    auto bb = state->state.buildBindings(capacity);
    auto res = new BindingsBuilder_Inner();
    *res = std::allocate_shared<nix::BindingsBuilder>(traceable_allocator<nix::BindingsBuilder>(), bb);
    return res;
}

void nix_bindings_builder_insert(BindingsBuilder* b, const char* name, Value* value) {
    nix::BindingsBuilder& builder = **(BindingsBuilder_Inner*)b;
    nix::Value &v = *(nix::Value*)value;
    nix::Symbol s = builder.state.symbols.create(name);
    builder.insert(s, &v);
}

void nix_bindings_builder_unref(BindingsBuilder* bb) {
    delete (BindingsBuilder_Inner*)bb;
}
