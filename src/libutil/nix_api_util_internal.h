#ifndef NIX_API_UTIL_INTERNAL_H
#define NIX_API_UTIL_INTERNAL_H

#include <string>

namespace nix {
  class Error;
};

/**
 * Internal use only.
 *
 * Sets the most recent error message.
 * 
 * TODO: Move to nix_api_util_internal.h
 * 
 * @param msg The error message to set.
 */
void nix_set_err_msg(const char* msg);
void nix_set_err(nix::Error&& err);

nix_err nix_export_std_string(std::string& str, char* dest, unsigned int n);

// todo: nix::ThrownError
// todo: currentExceptionTypeName
#define NIXC_CATCH_ERRS_NULL catch (nix::Error& e) { \
    nix_set_err(std::move(e)); \
    return nullptr; \
  } catch (const std::exception& e) { \
    nix_set_err_msg(e.what()); \
    return nullptr; \
  }
#define NIXC_CATCH_ERRS catch (nix::Error& e) { \
    nix_set_err(std::move(e)); \
    return NIX_ERR_NIX_ERROR; \
  } catch (const std::exception& e) { \
    nix_set_err_msg(e.what()); \
    return NIX_ERR_UNKNOWN; \
  } \
  return NIX_OK;

#endif // NIX_API_UTIL_INTERNAL_H
