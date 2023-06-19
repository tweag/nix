#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

#include "eval.hh"
#include "globals.hh"
#include "config.hh"
#include "util.hh"

#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_expr.h"
#include "nix_api_store_internal.h"
#include "nix_api_util_internal.h"

#ifdef HAVE_BOEHMGC
#define GC_INCLUDE_NEW 1
#include "gc_cpp.h"
#endif

struct GCRef {
    void* ptr;
};
struct State {
    nix::EvalState state;
};

nix_err nix_libexpr_init(nix_c_context* context) {
    {
        auto ret = nix_libutil_init(context);
        if (ret != NIX_OK) return ret;
    }
    {
        auto ret = nix_libstore_init(context);
        if (ret != NIX_OK) return ret;
    }
    try {
        nix::initGC();
    } NIXC_CATCH_ERRS
}

Expr* nix_parse_expr_from_string(nix_c_context* context, State* state, const char* expr, const char* path) {
    try {
        return state->state.parseExprFromString(expr, state->state.rootPath(nix::CanonPath(path)));
    } NIXC_CATCH_ERRS_NULL
}

nix_err nix_expr_eval(nix_c_context* context, State* state, Expr* expr, Value* value) {
    try {
        state->state.eval((nix::Expr*)expr, *(nix::Value*)value);
    } NIXC_CATCH_ERRS
}


nix_err nix_value_call(nix_c_context* context, State* state, Value* fn, Value* arg, Value* value) {
    // todo functors
    try {
        state->state.callFunction(*(nix::Value*)fn, *(nix::Value*) arg, *(nix::Value*)value, nix::noPos);
    } NIXC_CATCH_ERRS
}

nix_err nix_value_force(nix_c_context* context, State* state, Value* value) {
    try {
        state->state.forceValue(*(nix::Value*)value, nix::noPos);
    } NIXC_CATCH_ERRS
}

nix_err nix_value_force_deep(nix_c_context* context, State* state, Value* value) {
    try {
        state->state.forceValueDeep(*(nix::Value*)value);
    } NIXC_CATCH_ERRS
}

State* nix_state_create(nix_c_context* context, const char** searchPath_c, Store* store) {
    try {
        nix::Strings searchPath;
        if (searchPath_c != nullptr)
            for (size_t i = 0; searchPath_c[i] != nullptr; i++)
                searchPath.push_back(searchPath_c[i]);

        return new State{nix::EvalState(searchPath, store->ptr)};
    } NIXC_CATCH_ERRS_NULL
}

void nix_state_free(State* state) {
    delete state;
}

GCRef* nix_gc_ref(nix_c_context* context, void* obj) {
    try {
#if HAVE_BOEHMGC
        return new (NoGC) GCRef{obj};
#else
        return new GCRef{obj};
#endif
    } NIXC_CATCH_ERRS_NULL
}

void nix_gc_free(GCRef* ref) {
#if HAVE_BOEHMGC
    GC_FREE(ref);
#else
    delete ref;
#endif
}
