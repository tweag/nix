#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_store_internal.h"

#include "store-api.hh"

#include "globals.hh"

nix_err nix_libstore_init() {
    try {
        nix::initLibStore();
        return NIX_OK;
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return NIX_ERR_UNKNOWN;
    }
}

Store* nix_store_open() {
    try {
        // todo: uri, params
        return new Store{nix::openStore()};
    } catch (const std::exception& e) {
        nix_set_err_msg(e.what());
        return nullptr;
    }
}

void nix_store_unref(Store* store) {
    delete store;
}
