from __future__ import annotations

from typing import TypeAlias, TypeVar, Optional, Callable, Any
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

    def nix_err_name(self) -> str:
        value = ffi.new("char[128]")
        lib.nix_err_name(self._ctx, value, len(value))
        return ffi.string(value).decode("utf-8", errors="replace")

    def nix_err_info_msg(self) -> str:
        value = ffi.new("char[1024]")
        self._err_check(lib.nix_err_info_msg(self._ctx, value, len(value)))
        return ffi.string(value).decode()

    def res_check(
        self,
        fn: Callable[Concatenate[ffi.CData, ffi.CData, P], R],
        *rest: P.args,
        **kwrest: P.kwargs,
    ) -> R:
        return_code = ffi.new("nix_err*")
        res = fn(self._ctx, return_code, *rest, **kwrest)
        self._err_check(return_code[0])
        return res

    def err_check(
        self,
        fn: Callable[Concatenate[ffi.CData, P], int],
        *rest: P.args,
        **kwrest: P.kwargs,
    ) -> None:
        return self._err_check(fn(self._ctx, *rest, **kwrest))

    def _err_check(self, err_code: int) -> None:
        match err_code:
            case lib.NIX_OK:
                return
            case lib.NIX_ERR_NIX_ERROR:
                name = self.nix_err_name()
                if name in ERR_MAP:
                    err = ERR_MAP[name](self.nix_err_msg())
                else:
                    err = NixError(self.nix_err_msg())
                    err.name = name
                    err.msg = self.nix_err_info_msg()
                raise err
            case lib.NIX_ERR_KEY:
                raise KeyError(self.nix_err_msg())
            case _:
                raise NixAPIError(self.nix_err_msg())

    def null_check(
        self,
        fn: Callable[Concatenate[ffi.CData, P], ffi.CData],
        *rest: P.args,
        **kwrest: P.kwargs,
    ) -> ffi.CData:
        return self._null_check(fn(self._ctx, *rest, **kwrest))

    def err_code(self) -> int:
        res: int = lib.nix_err_code(self._ctx)
        return res

    def _null_check(self, obj: ffi.CData) -> ffi.CData:
        if not obj:
            self._err_check(self.err_code())
        return obj


class NixAPIError(Exception):
    pass


class NixError(NixAPIError):
    msg: Optional[str]
    name: Optional[str]


class ThrownError(NixError):
    def __repr__(self) -> str:
        if self.msg:
            return 'ThrownError("' + self.msg + '")'
        else:
            return super().__repr__()


class AssertionError(NixError):
    pass


ERR_MAP = {"nix::ThrownError": ThrownError, "nix::AssertionError": AssertionError}


class Settings:
    def __init__(self) -> None:
        pass

    def __setitem__(self, key: str, value: str) -> None:
        with Ctx() as ctx:
            return ctx.err_check(lib.nix_setting_set, key.encode(), value.encode())

    def __getitem__(self, key: str) -> str:
        value = ffi.new("char[1024]")
        with Ctx() as ctx:
            ctx.err_check(lib.nix_setting_get, key.encode(), value, len(value))
        return ffi.string(value).decode()


class Ctx:
    err_contexts: list[Context] = []
    ctx_level: int = 0

    def __enter__(self) -> Context:
        self.ctx_level += 1
        if len(self.err_contexts) < self.ctx_level:
            self.err_contexts.append(Context())
        return self.err_contexts[self.ctx_level - 1]

    def __exit__(self, type: Any, value: Any, traceback: Any) -> None:
        self.ctx_level -= 1


settings = Settings()

version = ffi.string(lib.nix_version_get()).decode()


def nix_util_init() -> None:
    with Ctx() as ctx:
        ctx.err_check(lib.nix_libutil_init)
