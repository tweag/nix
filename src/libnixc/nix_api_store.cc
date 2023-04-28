#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_store_internal.h"

#include "store-api.hh"

#include "globals.hh"

static nix_err export_std_string(std::string& str, char* dest, unsigned int n) {
    size_t i = str.copy(dest, n-1);
    dest[i] = 0;
    if (i == n - 1) {
        nix_set_err_msg("Provided buffer too short");
        return NIX_ERR_UNKNOWN;
    } else
        return NIX_OK;
}

nix_err nix_libstore_init() {
    try {
        nix::initLibStore();
        return NIX_OK;
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

Store* nix_store_open(const char* uri, const char*** params) {
    try {
        if (!uri) {
            return new Store{nix::openStore()};
        } else {
            std::string uri_str = uri;
            if (!params)
                return new Store{nix::openStore(uri_str)};

            nix::Store::Params params_map;
            for (size_t i = 0; params[i] != nullptr; i++) {
                params_map[params[i][0]] = params[i][1];
            }
            return new Store{nix::openStore(uri_str, params_map)};
        }
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return nullptr;
    }
}

void nix_store_unref(Store* store) {
    delete store;
}

nix_err nix_store_get_uri(Store* store, char* dest, unsigned int n) {
    auto res = store->ptr->getUri();
    return export_std_string(res, dest, n);
}

nix_err nix_store_get_version(Store* store, char* dest, unsigned int n) {
    auto res = store->ptr->getVersion();
    if (res) {
        return export_std_string(*res, dest, n);
    } else {
        nix_set_err_msg("store does not have a version");
        return NIX_ERR_UNKNOWN;
    }
}
