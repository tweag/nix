from __future__ import annotations

import typing
import re

from collections.abc import Callable
from typing import Any, Concatenate

from .util import ctx, CData

if typing.TYPE_CHECKING:
    from ._nix_api_types import Lib


P = typing.ParamSpec("P")


def wrap_ffi(
    f: Callable[Concatenate[CData, P], Any]
    | Callable[Concatenate[CData, CData, P], Any]
    | Callable[P, Any]
    | int
) -> Callable[P, Any] | int:
    """Wrap an ffi.lib member for nix error checking"""
    if isinstance(f, int):
        return f

    if not f.__doc__:
        raise TypeError("couldn't parse to-be-wrapped function")
    sig = f.__doc__.split("\n")[0]
    func, argstr = sig.split("(", 1)
    mtch = re.match(r"((struct )?[a-zA-Z0-9_]+[ \*]*)", func)
    if not mtch:
        raise RuntimeError("invalid function sig " + sig)
    tp = mtch[0].strip()
    args = argstr[:-2].split(", ")
    if tp == "void":
        return typing.cast(Callable[P, Any], f)
    if "*" in tp:
        # f is void* something(nix_context*, ...)
        g = typing.cast(Callable[Concatenate[CData, P], Any], f)

        def wrap_null(*args: P.args, **kwargs: P.kwargs) -> Any:
            return ctx().null_check(g, *args, **kwargs)

        return wrap_null
    elif tp != "int" and args[1] == "int *":
        # f is foo something(nix_context*, nix_err *res, ...)
        h = typing.cast(Callable[Concatenate[CData, CData, P], Any], f)

        def wrap_res(*args: P.args, **kwargs: P.kwargs) -> Any:
            return ctx().res_check(h, *args, **kwargs)

        return wrap_res
    else:
        # f is nix_err something(nix_context*, ...)
        i = typing.cast(Callable[Concatenate[CData, P], Any], f)

        def wrap_err(*args: P.args, **kwargs: P.kwargs) -> None:
            ctx().err_check(i, *args, **kwargs)

        return wrap_err


class LibWrap:
    """Wrap an ffi.lib for nix error checking"""

    def __init__(self, thing: Lib):
        self._thing = thing

    def __getattr__(self, attr: str) -> Any:
        r: Any = wrap_ffi(getattr(self._thing, attr))
        setattr(self, attr, r)
        return r

    def __dir__(self) -> list[str]:
        return dir(self._thing)
