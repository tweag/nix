#ifndef NIX_API_UTIL_H
#define NIX_API_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
// cffi start
// Error codes
#define NIX_OK 0
#define NIX_ERR_UNKNOWN -1
#define NIX_ERR_OVERFLOW -2
#define NIX_ERR_KEY -3
#define NIX_ERR_NIX_ERROR -4

// Type definitions
typedef int nix_err;

/**
 * This object stores error state and API config.
 * Optional, passing NULL will throw a C++ exception in case of errors.
 */
typedef struct nix_c_context nix_c_context;

// Function prototypes

nix_c_context* nix_c_context_create();
void nix_c_context_free(nix_c_context* context);
/**
 * Initializes nix_libutil and its dependencies.
 *
 * This function can be called multiple times, but should be called at least once prior to any other nix function.
 * 
 * @return NIX_OK if the initialization is successful, or an error code otherwise.
 */
nix_err nix_libutil_init(nix_c_context*);

/**
 * Retrieves a setting from the nix global configuration.
 *
 * This function requires nix_libutil_init() to be called at least once prior to its use.
 * 
 * @param key The key of the setting to retrieve.
 * @param value A pointer to a buffer where the value of the setting will be stored.
 * @param n The size of the buffer pointed to by value.
 * @return NIX_ERR_KEY if the setting is unknown, NIX_ERR_OVERFLOW if the provided buffer is too short,
 *         or NIX_OK if the setting was retrieved successfully.
 */
nix_err nix_setting_get(nix_c_context*, const char* key, char* value, int n);

/**
 * Sets a setting in the nix global configuration.
 *
 * Use "extra-<setting name>" to append to the setting's value.
 *
 * Settings only apply for new States. Call nix_plugins_init() when you are done with the settings to load any plugins.
 * 
 * @param key The key of the setting to set.
 * @param value The value to set for the setting.
 * @return NIX_ERR_KEY if the setting is unknown, or NIX_OK if the setting was set successfully.
 */
nix_err nix_setting_set(nix_c_context*, const char* key, const char* value);

// todo: nix_plugins_init()
  
/**
 * Retrieves the nix library version.
 * 
 * @return A borrowed static string representing the version of the nix library.
 */
const char* nix_version_get();

/**
 * Retrieves the most recent error message.
 *
 * This function should only be called after a previous nix function has returned an error.
 * 
 * @param n optional: a pointer to an unsigned int that is set to the length of the error.
 * @return nullptr if no error message was ever set,
 *         a borrowed pointer to the error message otherwise.
 */
const char* nix_err_msg(nix_c_context*, unsigned int* n);

nix_err nix_err_info_msg(nix_c_context*, char* value, int n);

nix_err nix_err_name(nix_c_context* context, char* value, int n);
nix_err nix_err_code(nix_c_context* context);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_UTIL_H
