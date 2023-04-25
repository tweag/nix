#ifndef NIX_API_UTIL_H
#define NIX_API_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
// cffi start
// Error codes
#define NIX_OK 0
#define NIX_ERR_UNKNOWN -1

// Type definitions
typedef int nix_err;

// Function prototypes
nix_err nix_libutil_init();

nix_err nix_setting_get(const char* key, char* value, int n);
nix_err nix_setting_set(const char* key, const char* value);

// returns: borrowed string
const char* nix_version_get();

void nix_err_msg(char* msg, int n);
void nix_set_err_msg(const char* msg);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_UTIL_H
