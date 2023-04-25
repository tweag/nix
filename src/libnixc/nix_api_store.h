#ifndef NIX_API_STORE_H
#define NIX_API_STORE_H

#include "nix_api_util.h"

#ifdef __cplusplus
extern "C" {
#endif
// cffi start

typedef struct Store Store;

nix_err nix_libstore_init();

// returns: owned Store*
Store* nix_store_open();
void nix_store_unref(Store* store);


// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_STORE_H
