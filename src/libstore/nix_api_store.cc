#include "nix_api_util.h"
#include "nix_api_store.h"
#include "nix_api_store_internal.h"
#include "nix_api_util_internal.h"

#include "store-api.hh"

#include "globals.hh"

struct StorePath {
    nix::StorePath path;
};

nix_err nix_libstore_init() {
    try {
        nix::initLibStore();
    } NIXC_CATCH_ERRS
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
    } NIXC_CATCH_ERRS_NULL
}

void nix_store_unref(Store* store) {
    delete store;
}

nix_err nix_store_get_uri(Store* store, char* dest, unsigned int n) {
    auto res = store->ptr->getUri();
    return nix_export_std_string(res, dest, n);
}

nix_err nix_store_get_version(Store* store, char* dest, unsigned int n) {
    auto res = store->ptr->getVersion();
    if (res) {
        return nix_export_std_string(*res, dest, n);
    } else {
        nix_set_err_msg("store does not have a version");
        return NIX_ERR_UNKNOWN;
    }
}

bool nix_store_is_valid_path(Store* store, StorePath* path) {
    return store->ptr->isValidPath(path->path);
}

StorePath* nix_store_parse_path(Store* store, const char* path) {
    try {
        nix::StorePath s = store->ptr->parseStorePath(path);
        return new StorePath{std::move(s)};
    } NIXC_CATCH_ERRS_NULL
}


nix_err nix_store_build(Store* store, StorePath* path, void (*iter)(const char*, const char*)) {
    try {
      store->ptr->buildPaths({
          nix::DerivedPath::Built{
              .drvPath = path->path,
              .outputs = nix::OutputsSpec::All{},
          },
      });
      if (iter) {
          for (auto & [outputName, outputPath] : store->ptr->queryDerivationOutputMap(path->path)) {
              auto op = store->ptr->printStorePath(outputPath);
              iter(outputName.c_str(), op.c_str());
          }
      }
    } NIXC_CATCH_ERRS
}

void nix_store_path_free(StorePath* sp) {
    delete sp;
}
