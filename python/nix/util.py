from __future__ import annotations

from typing import TypeAlias

from ._nix_api_util import lib, ffi

CData: TypeAlias = ffi.CData


class NixAPIError(Exception):
    pass

class NixError(NixAPIError):
    pass

class Settings:
    def __init__(self) -> None:
        pass

    def __setitem__(self, key: str, value: str) -> None:
        return err_check(lib.nix_setting_set(key.encode(), value.encode()))

    def __getitem__(self, key: str) -> str:
        value = ffi.new("char[1024]")
        err_check(lib.nix_setting_get(key.encode(), value, len(value)))
        return ffi.string(value).decode()


settings = Settings()

version = ffi.string(lib.nix_version_get()).decode()


def nix_util_init() -> None:
    err_check(lib.nix_libutil_init())


def nix_err_msg() -> str:
    msg = lib.nix_err_msg(ffi.NULL)
    return ffi.string(msg).decode("utf-8", errors="replace")

def nix_err_info_msg() -> str:
    value = ffi.new("char[1024]")
    err_check(lib.nix_err_info_msg(value, len(value)))
    return ffi.string(value).decode()

def err_check(err_code: int) -> None:
    if err_code == lib.NIX_ERR_NIX_ERROR:
        msg = nix_err_info_msg()
        err = NixError(nix_err_msg())
        err.msg = msg
        raise err
    if err_code != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())


# todo: how to find out the error type from this?
def null_check(obj: ffi.CData) -> ffi.CData:
    if not obj:
        raise NixAPIError(nix_err_msg())
    return obj
