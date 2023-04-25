#include "nix_api_util.h"
#include "config.hh"
#include "util.hh"


// Error buffer
static thread_local char g_error_buffer[1024];

// Helper function to set error message
void nix_set_err_msg(const char* msg) {
    strncpy(g_error_buffer, msg, sizeof(g_error_buffer) - 1);
    g_error_buffer[sizeof(g_error_buffer) - 1] = '\0';
}


const char* nix_version_get() {
    // borrowed static
    return PACKAGE_VERSION;
}

// Implementations
nix_err nix_setting_get(const char* key, char* value, int n) {
    try {
        std::map<std::string, nix::AbstractConfig::SettingInfo> settings;
        nix::globalConfig.getSettings(settings);
        if (settings.contains(key)) {
            size_t i = settings[key].value.copy(value, n-1);
            value[i] = 0;
            if (i == n - 1) {
                nix_set_err_msg("Provided buffer too short");
                return NIX_ERR_UNKNOWN;
            } else
                return NIX_OK;
        } else {
            nix_set_err_msg("Setting not found");
            return NIX_ERR_UNKNOWN;
        }
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

nix_err nix_setting_set(const char* key, const char* value) {
    if (nix::globalConfig.set(key, value))
        return NIX_OK;
    else {
        nix_set_err_msg("unknown setting");
        return NIX_ERR_UNKNOWN;
    }
}

nix_err nix_libutil_init() {
    try {
        nix::initLibUtil();
        return NIX_OK;
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

void nix_err_msg(char* msg, int n) {
    strncpy(msg, g_error_buffer, n - 1);
    msg[n - 1] = '\0';
}
