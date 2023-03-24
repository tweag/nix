#include "nix_api.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

#include "config.hh"
#include "globals.hh"

struct GCRef {
    std::shared_ptr<void> ptr;
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
    // TODO: Implement this function
    return NIX_OK;
}

Expr* nix_parse_expr_from_string(State* state, const char* expr, const char* path, StaticEnv* env) {
    // TODO: Implement this function
    return nullptr;
}

nix_err nix_expr_eval(State* state, Env* env, Value* value) {
    // TODO: Implement this function
    return NIX_OK;
}

nix_err nix_value_force_deep(State* state, Value* value) {
    // TODO: Implement this function
    return NIX_OK;
}

Store* nix_store_open() {
    // TODO: Implement this function
    return nullptr;
}

void nix_store_unref(Store* store) {
    // TODO: Implement this function
}

State* nix_state_create(const char** searchPath, Store* store) {
    // TODO: Implement this function
    return nullptr;
}

void nix_state_free(State* state) {
    // TODO: Implement this function
}

StaticEnv* nix_state_get_base_env(State* state) {
    // TODO: Implement this function
    return nullptr;
}

void nix_staticenv_unref(StaticEnv* env) {
    // TODO: Implement this function
}

void nix_err_msg(char* msg, int n) {
    strncpy(msg, g_error_buffer, n - 1);
    msg[n - 1] = '\0';
}

GCRef* nix_gc_ref(void* obj) {
    // TODO: Implement this function
    return nullptr;
}

void nix_gc_free(GCRef* ref) {
    delete ref;
}
