from __future__ import annotations

from typing import TypeAlias, TypeVar, Optional, Callable
from typing import Concatenate, ParamSpec

from ._nix_api_util import lib, ffi

CData: TypeAlias = ffi.CData

R = TypeVar("R")
P = ParamSpec("P")


class Context:
    def __init__(self) -> None:
        self._ctx = ffi.gc(lib.nix_c_context_create(), lib.nix_c_context_free)

    def nix_err_msg(self) -> str:
        msg = lib.nix_err_msg(self._ctx, ffi.NULL)
        return ffi.string(msg).decode("utf-8", errors="replace")

    def nix_err_info_msg(self) -> str:
        value = ffi.new("char[1024]")
        self._err_check(lib.nix_err_info_msg(self._ctx, value, len(value)))
        return ffi.string(value).decode()

    # todo type
    def err_check(
        self,
        fn: Callable[Concatenate[ffi.CData, P], int],
        *rest: P.args,
        **kwrest: P.kwargs,
    ) -> None:
        return self._err_check(fn(self._ctx, *rest, **kwrest))

    def _err_check(self, err_code: int) -> None:
        if err_code == lib.NIX_ERR_NIX_ERROR:
            msg = self.nix_err_info_msg()
            err = NixError(self.nix_err_msg())
            err.msg = msg
            raise err
        if err_code != lib.NIX_OK:
            raise NixAPIError(self.nix_err_msg())

    # todo type
    def null_check(
        self,
        fn: Callable[Concatenate[ffi.CData, P], ffi.CData],
        *rest: P.args,
        **kwrest: P.kwargs,
    ) -> ffi.CData:
        return self._null_check(fn(self._ctx, *rest, **kwrest))

    # todo: implement nix_context_err_code getter
    # todo: how to find out the error type from this?
    def _null_check(self, obj: ffi.CData) -> ffi.CData:
        if not obj:
            raise NixAPIError(self.nix_err_msg())
        return obj


class NixAPIError(Exception):
    pass


class NixError(NixAPIError):
    msg: Optional[str]


class Settings:
    def __init__(self) -> None:
        pass

    def __setitem__(self, key: str, value: str) -> None:
        return ctx().err_check(lib.nix_setting_set, key.encode(), value.encode())

    def __getitem__(self, key: str) -> str:
        value = ffi.new("char[1024]")
        ctx().err_check(lib.nix_setting_get, key.encode(), value, len(value))
        return ffi.string(value).decode()


err_context: Optional[Context] = None


def ctx() -> Context:
    global err_context
    if err_context is None:
        err_context = Context()
    return err_context


settings = Settings()

version = ffi.string(lib.nix_version_get()).decode()


def nix_util_init() -> None:
    ctx().err_check(lib.nix_libutil_init)
