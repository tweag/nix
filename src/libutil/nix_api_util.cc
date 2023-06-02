#include "nix_api_util.h"
#include "error.hh"
#include "nix_api_util_internal.h"
#include "config.hh"
#include "util.hh"


struct nix_c_context {
    std::variant<std::nullopt_t, std::string, nix::Error> last_err = std::nullopt;
    nix_err last_err_code = NIX_OK;
};

nix_c_context* nix_c_context_create() {
    return new nix_c_context();
}

void nix_c_context_free(nix_c_context* context) {
    delete context;
}

nix_err nix_context_error(nix_c_context* context) {
    if (context == nullptr) {
        throw;
    }
    try {
        throw;
    } catch (nix::Error& e) {
        context->last_err = e;
        context->last_err_code = NIX_ERR_NIX_ERROR;
        return context->last_err_code;
    } catch (const std::exception& e) {
        context->last_err = e.what();
        context->last_err_code = NIX_ERR_UNKNOWN;
        return context->last_err_code;
    }
    asm("unreachable\n");
}

nix_err nix_set_err_msg(nix_c_context* context, nix_err err, const char* msg) {
    if (context == nullptr) {
        // todo last_err_code
        throw new nix::Error("Nix C api error", msg);
    }
    context->last_err_code = err;
    context->last_err = msg;
    return err;
}

const char* nix_version_get() {
    return PACKAGE_VERSION;
}

// Implementations
nix_err nix_setting_get(nix_c_context* context, const char* key, char* value, int n) {
    try {
        std::map<std::string, nix::AbstractConfig::SettingInfo> settings;
        nix::globalConfig.getSettings(settings);
        if (settings.contains(key))
            return nix_export_std_string(context, settings[key].value, value, n);
        else {
            return nix_set_err_msg(context, NIX_ERR_KEY, "Setting not found");
        }
    } NIXC_CATCH_ERRS
}

nix_err nix_setting_set(nix_c_context* context, const char* key, const char* value) {
    if (nix::globalConfig.set(key, value))
        return NIX_OK;
    else {
        return nix_set_err_msg(context, NIX_ERR_KEY, "Setting not found");
    }
}

nix_err nix_libutil_init(nix_c_context* context) {
    try {
        nix::initLibUtil();
        return NIX_OK;
    } NIXC_CATCH_ERRS
}

const char* nix_err_msg(nix_c_context* context, unsigned int* n) {
    std::string* error_buffer = std::get_if<std::string>(&context->last_err);
    if (!error_buffer) {
        nix::Error* err = std::get_if<nix::Error>(&context->last_err);
        if (err) {
            // todo
            error_buffer = new std::string(err->what());
        } else return nullptr;
    }
    if (n != nullptr) *n = error_buffer->size();
    return error_buffer->c_str();
}

nix_err nix_err_info_msg(nix_c_context* context, char* value, int n) {
    if (context->last_err_code != NIX_ERR_NIX_ERROR) {
        return nix_set_err_msg(context, NIX_ERR_UNKNOWN, "Last error was not a nix error");
    }
    nix::Error& e = std::get<nix::Error>(context->last_err);
    std::string msg = e.info().msg.str();
    return nix_export_std_string(context, msg, value, n);
}


nix_err nix_export_std_string(nix_c_context* context, std::string& str, char* dest, unsigned int n) {
    size_t i = str.copy(dest, n-1);
    dest[i] = 0;
    if (i == n - 1) {
        return nix_set_err_msg(context, NIX_ERR_OVERFLOW, "Provided buffer too short");
    } else
        return NIX_OK;
}
