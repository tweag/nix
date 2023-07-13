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
 * This object stores error state.
 * Passed as a first parameter to C functions that can fail, will store error
 * information. Optional wherever it is used, passing NULL will throw a C++
 * exception instead. The first field is a nix_err, that can be read directly to
 * check for errors. Note: these can be reused between different function calls,
 * make sure not to use them for multiple calls simultaneously (which can happen
 * in callbacks).
 */
typedef struct nix_c_context nix_c_context;

// Function prototypes

/**
 * Allocate a new nix_c_context.
 * @throws std::bad_alloc
 * @return allocated nix_c_context, owned by the caller. Free using
 * `nix_c_context_free`.
 */
nix_c_context *nix_c_context_create();
/**
 * Free a nix_c_context. Does not fail.
 * @param context The context to free, mandatory.
 */
void nix_c_context_free(nix_c_context *context);

/**
 * Initializes nix_libutil and its dependencies.
 *
 * This function can be called multiple times, but should be called at least
 * once prior to any other nix function.
 *
 * @param context Optional, stores error information
 * @return NIX_OK if the initialization is successful, or an error code
 * otherwise.
 */
nix_err nix_libutil_init(nix_c_context *context);

/**
 * Retrieves a setting from the nix global configuration.
 *
 * This function requires nix_libutil_init() to be called at least once prior to
 * its use.
 *
 * @param context optional, Stores error information
 * @param key The key of the setting to retrieve.
 * @param value A pointer to a buffer where the value of the setting will be
 * stored.
 * @param n The size of the buffer pointed to by value.
 * @return NIX_ERR_KEY if the setting is unknown, NIX_ERR_OVERFLOW if the
 * provided buffer is too short, or NIX_OK if the setting was retrieved
 * successfully.
 */
nix_err nix_setting_get(nix_c_context *context, const char *key, char *value,
                        int n);

/**
 * Sets a setting in the nix global configuration.
 *
 * Use "extra-<setting name>" to append to the setting's value.
 *
 * Settings only apply for new States. Call nix_plugins_init() when you are done
 * with the settings to load any plugins.
 *
 * @param context optional, Stores error information
 * @param key The key of the setting to set.
 * @param value The value to set for the setting.
 * @return NIX_ERR_KEY if the setting is unknown, or NIX_OK if the setting was
 * set successfully.
 */
nix_err nix_setting_set(nix_c_context *context, const char *key,
                        const char *value);

// todo: nix_plugins_init()

/**
 * Retrieves the nix library version. Does not fail.
 *
 * @return A static string representing the version of the nix library.
 */
const char *nix_version_get();

/**
 * Retrieves the most recent error message from a context.
 *
 * This function should only be called after a previous nix function has
 * returned an error.
 *
 * @param context optional, the context to store errors in if this function
 * fails
 * @param ctx the context to retrieve the error message from
 * @param n optional: a pointer to an unsigned int that is set to the length of
 * the error.
 * @return nullptr if no error message was ever set,
 *         a borrowed pointer to the error message otherwise.
 */
const char *nix_err_msg(nix_c_context *context, const nix_c_context *ctx,
                        unsigned int *n);

/**
 * Retrieves the error message from errorInfo in a context. Used to inspect nix
 * Error messages.
 *
 * This function should only be called after a previous nix function has
 * returned a NIX_ERR_NIX_ERROR
 *
 * @param context optional, the context to store errors in if this function
 * fails
 * @param read_context the context to retrieve the error message from
 * @param value The allocated area to write the error string to.
 * @param n Maximum size of the returned string.
 * @return NIX_OK if there were no errors, an error code otherwise.
 */
nix_err nix_err_info_msg(nix_c_context *context,
                         const nix_c_context *read_context, char *value, int n);

/**
 * Retrieves the error name from a context. Used to inspect nix Error messages.
 *
 * This function should only be called after a previous nix function has
 * returned a NIX_ERR_NIX_ERROR
 *
 * @param context optional, the context to store errors in if this function
 * fails
 * @param read_context the context to retrieve the error message from
 * @param value The allocated area to write the error string to.
 * @param n Maximum size of the returned string.
 * @return NIX_OK if there were no errors, an error code otherwise.
 */
nix_err nix_err_name(nix_c_context *context, const nix_c_context *read_context,
                     char *value, int n);

/**
 * Retrieves the most recent error code from a nix_c_context
 * Equivalent to reading the first field of the context.
 *
 * @param context optional, the context to store errors in if this function
 * fails
 * @param ctx the context to retrieve the error message from
 * @return most recent error code stored in the context.
 */
nix_err nix_err_code(nix_c_context *context, const nix_c_context *read_context);

// cffi end
#ifdef __cplusplus
}
#endif

#endif // NIX_API_UTIL_H
