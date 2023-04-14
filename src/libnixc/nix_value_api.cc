#include "eval.hh"
#include "globals.hh"
#include "shared.hh"
#include "config.hh"
#include "nix_value_api.h"
#include "nix_api.h"
#include "value.hh"
#include "attr-set.hh"

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

Value* nix_get_list_byid(const Value* value, unsigned int ix) {
    check_value_not_null(value);
    // TODO: Implement this function based on the internal structure of Value
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nList);
    return (Value*)v.listElems()[ix];
}

Value* nix_get_attr_byname(const Value* value, State* state, const char* name) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nAttrs);
    // TODO: Implement this function based on the internal structure of Value
    nix::Symbol s = state->state.symbols.create(name);
    return (*v.attrs).get(s)->value;
}

Value* nix_get_attr_iterate(const Value* value, const State* state, void (*iter)(const char*, Value*, void*), void* data) {
    check_value_not_null(value);
    const nix::Value &v = *(const nix::Value*)value;
    assert(v.type() == nix::nAttrs);
    for (nix::Attr& a : *v.attrs) {
        const std::string& name = (const std::string&)state->state.symbols[a.name];
        iter(name.c_str(), a.value, data);
    }
    return nullptr;
}

