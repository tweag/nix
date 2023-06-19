from __future__ import annotations

import typing
import re

from collections.abc import Callable
from typing import Any

from .util import ctx

if typing.TYPE_CHECKING:
    from ._nix_api_types import Lib


P = typing.ParamSpec("P")


# todo: type this better
def wrap_ffi(f: Callable[..., Any] | int) -> Callable[P, Any] | int:
    """Wrap an ffi.lib object for nix error checking"""
    if isinstance(f, int):
        return f

    if not f.__doc__:
        raise TypeError("couldn't parse to-be-wrapped function")
    sig = f.__doc__.split("\n")[0]
    func, argstr = sig.split("(", 1)
    tp = re.match(r"((struct )?[a-zA-Z0-9_]+[ \*]*)", func)[0].strip()
    args = argstr[:-2].split(", ")
    if tp == "void":
        return f
    if "*" in tp:

        def wrap_null(*args: P.args, **kwargs: P.kwargs) -> Any:
            assert not isinstance(f, int)
            return ctx().null_check(f, *args, **kwargs)

        return wrap_null
    elif tp != "int" and args[1] == "int *":

        def wrap_res(*args: P.args, **kwargs: P.kwargs) -> Any:
            assert not isinstance(f, int)
            return ctx().res_check(f, *args, **kwargs)

        return wrap_res
    else:

        def wrap_err(*args: P.args, **kwargs: P.kwargs) -> None:
            assert not isinstance(f, int)
            ctx().err_check(f, *args, **kwargs)

        return wrap_err


class LibWrap:
    def __init__(self, thing: Lib):
        self._thing = thing

    def __getattr__(self, attr: str) -> Any:
        r: Any = wrap_ffi(getattr(self._thing, attr))
        setattr(self, attr, r)
        return r

    def __dir__(self) -> list[str]:
        return dir(self._thing)
