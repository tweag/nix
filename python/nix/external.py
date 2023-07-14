from __future__ import annotations

import typing
from collections.abc import Callable
from typing import Any, Optional

from .expr_util import ffi, lib, lib_unwrapped, CData, ReferenceGC, GCpin


def external_alloc_string(string: str) -> CData:
    return typing.cast(CData, lib_unwrapped.nix_external_alloc_string(string.encode()))


@ffi.def_extern()
def py_nix_external_print(slf: CData, printer: CData) -> None:
    self = ExternalValueImpl.from_handle(slf)

    def external_print(string: str) -> None:
        lib.nix_external_print(printer, string.encode())

    self.print(external_print)


@ffi.def_extern()
def py_nix_external_showType(slf: CData) -> CData:
    self = ExternalValueImpl.from_handle(slf)
    return external_alloc_string(self.showType())


@ffi.def_extern()
def py_nix_external_typeOf(slf: CData) -> CData:
    self = ExternalValueImpl.from_handle(slf)
    return external_alloc_string(self.typeOf())


@ffi.def_extern()
def py_nix_external_coerceToString(
    slf: CData, c: CData, copyMore: int, copyToStore: int
) -> CData:
    self = ExternalValueImpl.from_handle(slf)
    def add_context(x: str) -> None:
        lib.nix_external_add_string_context(c, x.encode())
    return external_alloc_string(
        self.coerceToString(add_context, bool(copyMore), bool(copyToStore))
    )


@ffi.def_extern()
def py_nix_external_equal(slf: CData, othr: CData) -> int:
    self = ExternalValueImpl.from_handle(slf)
    other = ExternalValueImpl.from_handle(othr)
    return int(self.equal(other))


standard_def_: Any = ffi.new("struct NixCExternalValueDesc*")
standard_def_.print = lib_unwrapped.py_nix_external_print
standard_def_.showType = lib_unwrapped.py_nix_external_showType
standard_def_.typeOf = lib_unwrapped.py_nix_external_typeOf
standard_def_.coerceToString = lib_unwrapped.py_nix_external_coerceToString
standard_def_.equal = lib_unwrapped.py_nix_external_equal
standard_def_.printValueAsJSON = ffi.NULL
standard_def_.printValueAsXML = ffi.NULL
standard_def: CData = standard_def_


class ExternalValueImpl(ReferenceGC):
    def __init__(self, value: Any, pin: GCpin) -> None:
        self._handle = ffi.new_handle(self)
        self._ref = lib.nix_create_external_value(standard_def, self._handle, pin.ref)
        self.value = value
        super().__init__(self._ref)

    @classmethod
    def from_handle(cls, handle: CData) -> "ExternalValueImpl":
        ev = ffi.from_handle(handle)
        assert isinstance(ev, ExternalValueImpl)
        return ev

    def print(self, printer: Callable[[str], None]) -> None:
        printer("<py: " + repr(self.value) + ">")

    def showType(self) -> str:
        return "Nix External Value"

    def typeOf(self) -> str:
        return "nix-external"

    def coerceToString(
        self, add_context: Callable[[str], None], copyMore: bool, copyToStore: bool
    ) -> str:
        return repr(self.value)

    def equal(self, other: ExternalValueImpl) -> bool:
        return self.value is other.value

    def __repr__(self) -> str:
        return "<ExternalValue: " + repr(self.value) + ">"


class ExternalValue:
    def __init__(
        self,
        value: Any,
        from_ev: Optional[ExternalValueImpl] = None,
        constructor: type = ExternalValueImpl,
    ) -> None:
        if from_ev is not None:
            self._pin = GCpin(from_ev._ref)
            self._x = from_ev
        else:
            self._pin = GCpin()
            self._x = constructor(value, self._pin)

    @classmethod
    def from_handle(cls, handle: CData) -> "ExternalValue":
        return cls(None, ExternalValueImpl.from_handle(handle))

    def __getattr__(self, name: str) -> Any:
        return getattr(self._x, name)

    def __repr__(self) -> str:
        return repr(self._x)
