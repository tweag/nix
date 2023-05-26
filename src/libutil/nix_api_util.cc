#include "nix_api_util.h"
#include "nix_api_util_internal.h"
#include "config.hh"
#include "util.hh"


// Pointer to error buffer. Kept small in case nobody uses the C api
static thread_local std::unique_ptr<std::string> error_buffer;
static thread_local std::unique_ptr<nix::Error> last_error;


void nix_set_err(nix::Error&& err) {
    error_buffer = std::make_unique<std::string>(err.what());
    last_error = std::make_unique<nix::Error>(std::move(err));
}

// Helper function to set error message
void nix_set_err_msg(const char* msg) {
    last_error.reset();
    error_buffer = std::make_unique<std::string>(msg);
}

const char* nix_version_get() {
    return PACKAGE_VERSION;
}

// Implementations
nix_err nix_setting_get(const char* key, char* value, int n) {
    try {
        std::map<std::string, nix::AbstractConfig::SettingInfo> settings;
        nix::globalConfig.getSettings(settings);
        if (settings.contains(key))
            return nix_export_std_string(settings[key].value, value, n);
        else {
            nix_set_err_msg("Setting not found");
            return NIX_ERR_KEY;
        }
    } NIXC_CATCH_ERRS
}

nix_err nix_setting_set(const char* key, const char* value) {
    if (nix::globalConfig.set(key, value))
        return NIX_OK;
    else {
        nix_set_err_msg("unknown setting");
        return NIX_ERR_KEY;
    }
}

nix_err nix_libutil_init() {
    try {
        nix::initLibUtil();
        return NIX_OK;
    } NIXC_CATCH_ERRS
}

const char* nix_err_msg(unsigned int* n) {
    if (!error_buffer) return nullptr;
    if (n != nullptr) *n = error_buffer->size();
    return error_buffer->c_str();
}

nix_err nix_err_info_msg(char* value, int n) {
    if (!last_error) {
        nix_set_err_msg("Last error was not a nix error");
        return NIX_ERR_UNKNOWN;
    }
    std::string msg = last_error->info().msg.str();
    return nix_export_std_string(msg, value, n);
}

nix_err nix_export_std_string(std::string& str, char* dest, unsigned int n) {
    size_t i = str.copy(dest, n-1);
    dest[i] = 0;
    if (i == n - 1) {
        nix_set_err_msg("Provided buffer too short");
        return NIX_ERR_OVERFLOW;
    } else
        return NIX_OK;
}
