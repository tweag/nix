from __future__ import annotations

import builtins
import collections.abc
import types
import typing
from collections.abc import Callable, Iterator
from typing import Any, TypeAlias, Union

from ._nix_api_util import lib, ffi

CData: TypeAlias = ffi.CData

class NixAPIError(Exception):
    pass

class Settings:
    def __init__(self): pass
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
    msg = ffi.new("char[1024]")
    lib.nix_err_msg(msg, len(msg))
    return ffi.string(msg).decode('utf-8', errors='replace')


def err_check(err_code: int) -> None:
    if err_code != lib.NIX_OK:
        raise NixAPIError(nix_err_msg())

def null_check(obj: ffi.CData) -> ffi.CData:
    if not obj:
        raise NixAPIError(nix_err_msg())
    return obj
