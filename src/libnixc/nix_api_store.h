#ifndef NIX_API_STORE_H
#define NIX_API_STORE_H
#include <stdbool.h>
#include "nix_api_util.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

typedef struct Store Store;
typedef struct StorePath StorePath;

nix_err nix_libstore_init();

// returns: ref-counted Store*
// uri: null or store uri
// params: null or {{"endpoint", "https://s3.local/"}, NULL}
Store* nix_store_open(const char* uri, const char*** params);
void nix_store_unref(Store* store);

nix_err nix_store_get_uri(Store* store, char* dest, unsigned int n);

// returns: owned Store*
StorePath* nix_store_parse_path(Store* store, const char* path);
void nix_store_path_free(StorePath*);

bool nix_store_is_valid_path(Store*, StorePath*);
  //void nix_store_ensure(Store*, const char*);
//  void nix_store_build_paths(Store*);

nix_err nix_store_build(Store* store, StorePath* path, void (*iter)(const char*, const char*));
nix_err nix_store_get_version(Store* store, char* dest, unsigned int n);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_STORE_H
