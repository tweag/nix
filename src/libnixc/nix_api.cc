#include "nix_api.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

#include "eval.hh"
#include "globals.hh"
#include "shared.hh"
#include "config.hh"
#include <store-api.hh>

struct GCRef {
    std::shared_ptr<void> ptr;
};
struct Store {
    nix::ref<nix::Store> ptr;
};
struct State {
    nix::EvalState state;
};

// Error buffer
static thread_local char g_error_buffer[1024];

// Helper function to set error message
static void set_error_message(const char* msg) {
    strncpy(g_error_buffer, msg, sizeof(g_error_buffer) - 1);
    g_error_buffer[sizeof(g_error_buffer) - 1] = '\0';
}

// Implementations
nix_err nix_setting_get(const char* key, char* value, int n) {
    try {
        // std::map<std::string, SettingInfo>
        // nix::settings.getSettings(settings);
        // TODO: Implement this function
        return NIX_OK;
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

nix_err nix_setting_set(const char* key, const char* value) {
    // TODO: Implement this function
    return NIX_OK;
}

const char* nix_version_get() {
    // borrowed static
    return nix::nixVersion.c_str();
}

nix_err nix_init() {
    try {
        nix::initNix();
        nix::initGC();
        return NIX_OK;
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

Expr* nix_parse_expr_from_string(State* state, const char* expr, const char* path) {
    try {
        return state->state.parseExprFromString(expr, path);
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return nullptr;
    }
}

nix_err nix_expr_eval(State* state, Expr* expr, Value* value) {
    try {
        state->state.eval((nix::Expr*)expr, *(nix::Value*)value);
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}


nix_err nix_value_call(State* state, Value* fn, Value* arg, Value* value) {
    try {
        state->state.callFunction(*(nix::Value*)fn, *(nix::Value*) arg, *(nix::Value*)value, nix::noPos);
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

nix_err nix_value_force(State* state, Value* value) {
    try {
        state->state.forceValue(*(nix::Value*)value, nix::noPos);
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

nix_err nix_value_force_deep(State* state, Value* value) {
    try {
        state->state.forceValueDeep(*(nix::Value*)value);
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return NIX_ERR_UNKNOWN;
    }
    return NIX_OK;
}

Store* nix_store_open() {
    try {
        // todo: uri, params
        return new Store{nix::openStore()};
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return nullptr;
    }
}

void nix_store_unref(Store* store) {
    delete store;
}

State* nix_state_create(const char** searchPath, Store* store) {
    // todo: searchPath
    try {
        nix::Strings searchPath;
        return new State{nix::EvalState(searchPath, store->ptr)};
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return nullptr;
    }
}

void nix_state_free(State* state) {
    delete state;
}

void nix_err_msg(char* msg, int n) {
    strncpy(msg, g_error_buffer, n - 1);
    msg[n - 1] = '\0';
}

GCRef* nix_gc_ref(void* obj) {
    try {
#if HAVE_BOEHMGC
        return new GCRef{std::allocate_shared<void*>(traceable_allocator<void*>(), obj)};
#else
        return new GCRef{std::make_shared<void*>(obj)};
#endif
    } catch (const std::exception& e) {
        set_error_message(e.what());
        return nullptr;
    }
}

void nix_gc_free(GCRef* ref) {
    delete ref;
}
