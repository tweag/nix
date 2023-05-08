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

struct GCRef {
    std::shared_ptr<void> ptr;
};
struct State {
    nix::EvalState state;
};

nix_err nix_libexpr_init() {
    {
        auto ret = nix_libutil_init();
        if (ret != NIX_OK) return ret;
    }
    {
        auto ret = nix_libstore_init();
        if (ret != NIX_OK) return ret;
    }
    try {
        nix::initGC();
        return NIX_OK;
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

Expr* nix_parse_expr_from_string(State* state, const char* expr, const char* path) {
    try {
        return state->state.parseExprFromString(expr, state->state.rootPath(nix::CanonPath(path)));
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return nullptr;
    }
}

nix_err nix_expr_eval(State* state, Expr* expr, Value* value) {
    try {
        state->state.eval((nix::Expr*)expr, *(nix::Value*)value);
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}


nix_err nix_value_call(State* state, Value* fn, Value* arg, Value* value) {
    try {
        state->state.callFunction(*(nix::Value*)fn, *(nix::Value*) arg, *(nix::Value*)value, nix::noPos);
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

nix_err nix_value_force(State* state, Value* value) {
    try {
        state->state.forceValue(*(nix::Value*)value, nix::noPos);
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

nix_err nix_value_force_deep(State* state, Value* value) {
    try {
        state->state.forceValueDeep(*(nix::Value*)value);
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

State* nix_state_create(const char** searchPath_c, Store* store) {
    try {
        nix::Strings searchPath;
        if (searchPath_c != nullptr)
            for (size_t i = 0; searchPath_c[i] != nullptr; i++)
                searchPath.push_back(searchPath_c[i]);

        return new State{nix::EvalState(searchPath, store->ptr)};
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return nullptr;
    }
}

void nix_state_free(State* state) {
    delete state;
}

GCRef* nix_gc_ref(void* obj) {
    try {
#if HAVE_BOEHMGC
        return new GCRef{std::allocate_shared<void*>(traceable_allocator<void*>(), obj)};
#else
        return new GCRef{std::make_shared<void*>(obj)};
#endif
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return nullptr;
    }
}

void nix_gc_free(GCRef* ref) {
    delete ref;
}
